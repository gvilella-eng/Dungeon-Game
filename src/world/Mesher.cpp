#include "Mesher.h"
#include "Noise.h"
#include <QDir>
#include <QFile>
#include <QElapsedTimer>
#include <chrono>
#include <cstring>

namespace {
constexpr uint32_t kMagic = 0x434D534Du;     // "MSMC"
constexpr uint32_t kEndMagic = 0x21444E45u;  // "END!"

struct CacheHeader {
    uint32_t magic, version;
    uint64_t hash;
    uint32_t count;
    float voxel;
    uint32_t cells;
};
struct ChunkRecord {
    int32_t cx, cy, cz;
    uint32_t flags, nv, ni;
    float bb[6];
};

const int kEdges[12][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3}, {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};

double nowSec()
{
    using namespace std::chrono;
    return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
}
double g_startTime = 0.0;
} // namespace

Mesher::Mesher(const SdfWorld& world, const Config& cfg) : m_w(world), m_cfg(cfg) {}

Mesher::~Mesher()
{
    waitAll();
    for (auto& t : m_threads) if (t.joinable()) t.join();
    delete static_cast<QFile*>(m_cacheFile);
}

void Mesher::waitAll()
{
    for (auto& t : m_threads) if (t.joinable()) t.join();
    m_threads.clear();
}

Vec3 Mesher::chunkOrigin(int cx, int cy, int cz) const
{
    float s = chunkSize();
    return m_w.bounds().mn + Vec3(cx * s, cy * s, cz * s);
}

QString Mesher::cachePath() const
{
    return QDir(m_cfg.cacheDir).filePath(QString("mesh_%1.bin").arg((qulonglong)m_hash, 16, 16, QChar('0')));
}

bool Mesher::meshChunk(int cx, int cy, int cz, ChunkMesh& out) const
{
    const int N = m_cfg.cells, D = N + 2, C = N + 1;
    const float vox = m_cfg.voxel;
    const Vec3 mn = m_w.bounds().mn;
    const int gx0 = cx * N, gy0 = cy * N, gz0 = cz * N;
    auto pos = [&](int gx, int gy, int gz) { return mn + Vec3(gx * vox, gy * vox, gz * vox); };

    // limite de distancia no centro: prova ausencia de superficie
    float half = chunkSize() * 0.8660254f;
    float dc = m_w.distance(pos(gx0 + N / 2, gy0 + N / 2, gz0 + N / 2));
    if (std::fabs(dc) > half * 1.4f) return false;

    std::vector<float> d((size_t)D * D * D);
    int neg = 0, posc = 0;
    for (int k = 0; k < D; ++k)
        for (int j = 0; j < D; ++j)
            for (int i = 0; i < D; ++i) {
                float v = m_w.distance(pos(gx0 + i - 1, gy0 + j - 1, gz0 + k - 1));
                d[((size_t)k * D + j) * D + i] = v;
                if (v < 0) ++neg; else ++posc;
            }
    if (neg == 0 || posc == 0) return false;

    MeshData& md = out.mesh;
    md.clear();
    std::vector<int32_t> vid((size_t)C * C * C, -1);
    auto S = [&](int i, int j, int k) { return d[((size_t)k * D + j) * D + i]; };

    for (int ck = 0; ck < C; ++ck)
        for (int cj = 0; cj < C; ++cj)
            for (int ci = 0; ci < C; ++ci) {
                float c[8]; int mask = 0;
                for (int n = 0; n < 8; ++n) {
                    c[n] = S(ci + (n & 1), cj + ((n >> 1) & 1), ck + ((n >> 2) & 1));
                    if (c[n] < 0) mask |= 1 << n;
                }
                if (mask == 0 || mask == 255) continue;
                Vec3 acc(0, 0, 0); int cnt = 0;
                for (int e = 0; e < 12; ++e) {
                    int a = kEdges[e][0], b = kEdges[e][1];
                    if ((c[a] < 0) == (c[b] < 0)) continue;
                    float t = c[a] / (c[a] - c[b]);
                    Vec3 pa((float)(a & 1), (float)((a >> 1) & 1), (float)((a >> 2) & 1));
                    Vec3 pb((float)(b & 1), (float)((b >> 1) & 1), (float)((b >> 2) & 1));
                    acc += pa + (pb - pa) * t; ++cnt;
                }
                Vec3 f = acc / (float)cnt;
                Vec3 p = pos(gx0 + ci - 1, gy0 + cj - 1, gz0 + ck - 1) + f * vox;

                // refinamento: um passo de Newton ate a superficie + normal do gradiente
                Vec3 n = m_w.gradient(p, vox * 0.4f);
                SdfWorld::Sample sm = m_w.eval(p);
                float step = clampf(sm.d, -vox * 0.5f, vox * 0.5f);
                p -= n * step;
                sm = m_w.eval(p);
                n = m_w.gradient(p, vox * 0.4f);

                // oclusao ambiente por SDF ao longo da normal
                float occ = 0.f, w = 1.f, wsum = 0.f;
                const float hs[3] = {0.25f, 0.6f, 1.3f};
                for (float h : hs) {
                    float dd = m_w.distance(p + n * h);
                    occ += clampf((h - dd) / h, 0.f, 1.f) * w; wsum += w; w *= 0.65f;
                }
                float ao = clampf(1.f - 1.15f * occ / wsum, 0.08f, 1.f);

                Vertex v;
                v.px = p.x(); v.py = p.y(); v.pz = p.z();
                v.nx = n.x(); v.ny = n.y(); v.nz = n.z();
                v.r = (uint8_t)clampf(128.f + 110.f * Noise::gnoise(p.x() * 0.17f, p.y() * 0.17f, p.z() * 0.17f), 0.f, 255.f);
                v.g = (uint8_t)clampf(128.f + 110.f * Noise::gnoise(p.x() * 0.55f + 7.f, p.y() * 0.55f, p.z() * 0.55f - 3.f), 0.f, 255.f);
                v.b = 0; v.a = 255;
                v.mat = (uint8_t)sm.mat;
                v.ao = (uint8_t)(ao * 255.f);
                v.fx = v.fy = 0;
                vid[((size_t)ck * C + cj) * C + ci] = (int32_t)md.v.size();
                md.v.push_back(v);
                md.bounds.expand(p);
            }

    auto cid = [&](int i, int j, int k) { return vid[((size_t)k * C + j) * C + i]; };
    auto emitQuad = [&](int c0, int c1, int c2, int c3, const Vec3& expect) {
        if (c0 < 0 || c1 < 0 || c2 < 0 || c3 < 0) return;
        auto P = [&](int id) { const Vertex& v = md.v[id]; return Vec3(v.px, v.py, v.pz); };
        int q[4] = {c0, c1, c2, c3};
        // escolhe a diagonal mais curta
        int t[2][3];
        if ((P(c0) - P(c2)).lengthSquared() <= (P(c1) - P(c3)).lengthSquared()) {
            t[0][0] = q[0]; t[0][1] = q[1]; t[0][2] = q[2]; t[1][0] = q[0]; t[1][1] = q[2]; t[1][2] = q[3];
        } else {
            t[0][0] = q[0]; t[0][1] = q[1]; t[0][2] = q[3]; t[1][0] = q[1]; t[1][1] = q[2]; t[1][2] = q[3];
        }
        for (int k = 0; k < 2; ++k) {
            Vec3 g = Vec3::crossProduct(P(t[k][1]) - P(t[k][0]), P(t[k][2]) - P(t[k][0]));
            if (Vec3::dotProduct(g, expect) < 0) std::swap(t[k][1], t[k][2]);
            md.i.push_back(t[k][0]); md.i.push_back(t[k][1]); md.i.push_back(t[k][2]);
        }
    };

    for (int k = 1; k <= N; ++k)
        for (int j = 1; j <= N; ++j)
            for (int i = 1; i <= N; ++i) {
                float s0 = S(i, j, k);
                // eixo X
                float s1 = S(i + 1, j, k);
                if ((s0 < 0) != (s1 < 0))
                    emitQuad(cid(i, j - 1, k - 1), cid(i, j, k - 1), cid(i, j, k), cid(i, j - 1, k), Vec3(s0 < 0 ? 1.f : -1.f, 0, 0));
                s1 = S(i, j + 1, k);
                if ((s0 < 0) != (s1 < 0))
                    emitQuad(cid(i - 1, j, k - 1), cid(i, j, k - 1), cid(i, j, k), cid(i - 1, j, k), Vec3(0, s0 < 0 ? 1.f : -1.f, 0));
                s1 = S(i, j, k + 1);
                if ((s0 < 0) != (s1 < 0))
                    emitQuad(cid(i - 1, j - 1, k), cid(i, j - 1, k), cid(i, j, k), cid(i - 1, j, k), Vec3(0, 0, s0 < 0 ? 1.f : -1.f));
            }

    if (md.i.empty()) { md.clear(); return false; }

    // remove vertices nao usados (celulas da borda -1 sem quads proprios)
    std::vector<int32_t> remap(md.v.size(), -1);
    std::vector<Vertex> nv; nv.reserve(md.v.size());
    for (auto& ix : md.i) {
        if (remap[ix] < 0) { remap[ix] = (int32_t)nv.size(); nv.push_back(md.v[ix]); }
        ix = (uint32_t)remap[ix];
    }
    md.v.swap(nv);
    md.bounds = Aabb();
    for (const Vertex& v : md.v) md.bounds.expand(Vec3(v.px, v.py, v.pz));

    out.cx = cx; out.cy = cy; out.cz = cz;
    Aabb cb(chunkOrigin(cx, cy, cz), chunkOrigin(cx, cy, cz) + Vec3(chunkSize(), chunkSize(), chunkSize()));
    out.exterior = m_w.overlapsExterior(cb);
    return true;
}

void Mesher::start()
{
    g_startTime = nowSec();
    m_hash = m_w.hash();
    { Hasher h; h.h = m_hash; h.add(kVersion); h.add(m_cfg.voxel); h.add(m_cfg.cells); m_hash = h.h; }

    // lista de tarefas
    float s = chunkSize();
    Aabb wb = m_w.bounds();
    int nx = (int)std::ceil(wb.size().x() / s), ny = (int)std::ceil(wb.size().y() / s), nz = (int)std::ceil(wb.size().z() / s);
    for (int z = 0; z < nz; ++z)
        for (int y = 0; y < ny; ++y)
            for (int x = 0; x < nx; ++x) {
                Aabb cb(chunkOrigin(x, y, z), chunkOrigin(x, y, z) + Vec3(s, s, s));
                if (m_w.maybeSurface(cb.inflated(m_cfg.voxel * 2.f))) m_tasks.push_back({x, y, z});
            }
    m_total = m_tasks.size();

    QString path;
    if (m_cfg.useCache && !m_cfg.cacheDir.isEmpty()) {
        QDir().mkpath(m_cfg.cacheDir);
        path = cachePath();
        // apaga caches antigos
        for (const QFileInfo& fi : QDir(m_cfg.cacheDir).entryInfoList({"mesh_*.bin*"}, QDir::Files))
            if (fi.absoluteFilePath() != QFileInfo(path).absoluteFilePath()) QFile::remove(fi.absoluteFilePath());
        QFile f(path);
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            CacheHeader h{};
            bool ok = f.read((char*)&h, sizeof h) == (qint64)sizeof h && h.magic == kMagic && h.version == (uint32_t)kVersion && h.hash == m_hash;
            if (ok && f.size() > (qint64)sizeof h + 4) {
                f.seek(f.size() - 4);
                uint32_t e = 0; f.read((char*)&e, 4);
                ok = (e == kEndMagic);
            } else ok = false;
            f.close();
            if (ok) {
                m_fromCache = true; m_total = h.count;
                m_alive = 1;
                m_threads.emplace_back([this, path] { loaderLoop(path); });
                return;
            }
            QFile::remove(path);
        }
        auto* wf = new QFile(path + ".tmp");
        if (wf->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            CacheHeader h{kMagic, (uint32_t)kVersion, m_hash, 0, m_cfg.voxel, (uint32_t)m_cfg.cells};
            wf->write((const char*)&h, sizeof h);
            m_cacheFile = wf;
        } else delete wf;
    }
    int nt = m_cfg.threads > 0 ? m_cfg.threads : std::max(1, (int)std::thread::hardware_concurrency() - 1);
    m_alive = nt;
    for (int i = 0; i < nt; ++i) m_threads.emplace_back([this] { workerLoop(); });
}

void Mesher::workerLoop()
{
    for (;;) {
        size_t i = m_next.fetch_add(1);
        if (i >= m_tasks.size()) break;
        ChunkMesh cm;
        if (meshChunk(m_tasks[i].cx, m_tasks[i].cy, m_tasks[i].cz, cm)) {
            m_verts += cm.mesh.v.size();
            if (m_cacheFile) {
                std::lock_guard<std::mutex> lk(m_fileMx);
                auto* f = static_cast<QFile*>(m_cacheFile);
                ChunkRecord r{cm.cx, cm.cy, cm.cz, cm.exterior ? 1u : 0u, (uint32_t)cm.mesh.v.size(), (uint32_t)cm.mesh.i.size(),
                              {cm.mesh.bounds.mn.x(), cm.mesh.bounds.mn.y(), cm.mesh.bounds.mn.z(), cm.mesh.bounds.mx.x(), cm.mesh.bounds.mx.y(), cm.mesh.bounds.mx.z()}};
                f->write((const char*)&r, sizeof r);
                f->write((const char*)cm.mesh.v.data(), (qint64)(cm.mesh.v.size() * sizeof(Vertex)));
                f->write((const char*)cm.mesh.i.data(), (qint64)(cm.mesh.i.size() * 4));
                m_written++;
            }
            std::lock_guard<std::mutex> lk(m_mx);
            m_results.push_back(std::move(cm));
        }
        m_done++;
    }
    if (--m_alive == 0) {
        m_elapsed = nowSec() - g_startTime;
        if (m_cacheFile) {
            auto* f = static_cast<QFile*>(m_cacheFile);
            uint32_t e = kEndMagic;
            f->write((const char*)&e, 4);
            f->seek(offsetof(CacheHeader, count));
            uint32_t cnt = (uint32_t)m_written;
            f->write((const char*)&cnt, 4);
            f->close();
            QString fn = f->fileName();
            QString dst = fn.left(fn.size() - 4);
            QFile::remove(dst);
            QFile::rename(fn, dst);
        }
    }
}

void Mesher::loaderLoop(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) { m_failed = true; m_done = m_total; return; }
    CacheHeader h{};
    f.read((char*)&h, sizeof h);
    for (uint32_t n = 0; n < h.count; ++n) {
        ChunkRecord r{};
        if (f.read((char*)&r, sizeof r) != (qint64)sizeof r) break;
        ChunkMesh cm;
        cm.cx = r.cx; cm.cy = r.cy; cm.cz = r.cz; cm.exterior = r.flags & 1;
        cm.mesh.v.resize(r.nv); cm.mesh.i.resize(r.ni);
        f.read((char*)cm.mesh.v.data(), (qint64)r.nv * sizeof(Vertex));
        f.read((char*)cm.mesh.i.data(), (qint64)r.ni * 4);
        cm.mesh.bounds = Aabb(Vec3(r.bb[0], r.bb[1], r.bb[2]), Vec3(r.bb[3], r.bb[4], r.bb[5]));
        m_verts += r.nv;
        {
            // limita a fila para nao estourar memoria se a GPU atrasar
            for (;;) { { std::lock_guard<std::mutex> lk(m_mx); if (m_results.size() < 64) break; } std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
            std::lock_guard<std::mutex> lk(m_mx);
            m_results.push_back(std::move(cm));
        }
        m_done++;
    }
    m_done = m_total;
    m_elapsed = nowSec() - g_startTime;
    --m_alive;
}

int Mesher::poll(std::vector<ChunkMesh>& out)
{
    std::lock_guard<std::mutex> lk(m_mx);
    int n = (int)m_results.size();
    for (auto& c : m_results) out.push_back(std::move(c));
    m_results.clear();
    m_polled += n;
    return n;
}

float Mesher::progress() const
{
    if (m_total == 0) return 1.f;
    return std::min(1.f, (float)m_done.load() / (float)m_total);
}

bool Mesher::finished() const
{
    if (m_done.load() < m_total || m_alive.load() > 0) return false;
    std::lock_guard<std::mutex> lk(const_cast<std::mutex&>(m_mx));
    return m_results.empty();
}
