#include "Validator.h"
#include "CommandLine.h"
#include <QFile>
#include <future>
#include <queue>
#include <unordered_map>

namespace {

struct PathResult { bool ok = false; float length = 0.f; Vec3 stuckAt; float stuckDist = 0.f; bool startBlocked = false, endBlocked = false; size_t expanded = 0; bool limit = false; };

struct Grid {
    const World& w; Vec3 mn; float c = 0.5f; float clearance = 0.4f;
    std::unordered_map<int64_t, uint8_t> freeCache;
    Grid(const World& wo) : w(wo) { mn = wo.sdf.bounds().mn; }
    static int64_t key(int x, int y, int z) { return ((int64_t)(x + 100000) << 42) | ((int64_t)(y + 100000) << 21) | (int64_t)(z + 100000); }
    static void unkey(int64_t k, int& x, int& y, int& z) { x = (int)(k >> 42) - 100000; y = (int)((k >> 21) & 0x1FFFFF) - 100000; z = (int)(k & 0x1FFFFF) - 100000; }
    Vec3 center(int x, int y, int z) const { return mn + Vec3((x + 0.5f) * c, (y + 0.5f) * c, (z + 0.5f) * c); }
    void cellOf(const Vec3& p, int& x, int& y, int& z) const { x = (int)std::floor((p.x() - mn.x()) / c); y = (int)std::floor((p.y() - mn.y()) / c); z = (int)std::floor((p.z() - mn.z()) / c); }
    bool isFree(int x, int y, int z) {
        int64_t k = key(x, y, z);
        auto it = freeCache.find(k);
        if (it != freeCache.end()) return it->second;
        bool f = w.solidDistance(center(x, y, z), false) > clearance;
        freeCache[k] = f;
        return f;
    }
    bool snap(const Vec3& p, int& x, int& y, int& z) {
        cellOf(p, x, y, z);
        if (isFree(x, y, z)) return true;
        for (int r = 1; r <= 3; ++r)
            for (int dz = -r; dz <= r; ++dz) for (int dy = -r; dy <= r; ++dy) for (int dx = -r; dx <= r; ++dx)
                if (isFree(x + dx, y + dy, z + dz)) { x += dx; y += dy; z += dz; return true; }
        return false;
    }
};

PathResult findPath(const World& w, const Vec3& a, const Vec3& b)
{
    PathResult r;
    Grid g(w);
    int ax, ay, az, bx, by, bz;
    if (!g.snap(a, ax, ay, az)) { r.startBlocked = true; r.stuckAt = a; return r; }
    if (!g.snap(b, bx, by, bz)) { r.endBlocked = true; r.stuckAt = b; return r; }
    Aabb region; region.expand(a); region.expand(b); region.inflate(45.f);
    int rx0, ry0, rz0, rx1, ry1, rz1;
    g.cellOf(region.mn, rx0, ry0, rz0); g.cellOf(region.mx, rx1, ry1, rz1);
    struct Node { float g; int64_t parent; };
    std::unordered_map<int64_t, Node> nodes;
    using QE = std::pair<float, int64_t>;
    std::priority_queue<QE, std::vector<QE>, std::greater<QE>> open;
    Vec3 goal = g.center(bx, by, bz);
    auto h = [&](int x, int y, int z) { return (g.center(x, y, z) - goal).length() * 1.25f; };
    int64_t sk = Grid::key(ax, ay, az);
    nodes[sk] = {0.f, -1};
    open.push({h(ax, ay, az), sk});
    float bestD = 1e30f; Vec3 bestP = a;
    const size_t kLimit = 6000000;
    while (!open.empty()) {
        auto [f, k] = open.top(); open.pop();
        int x, y, z; Grid::unkey(k, x, y, z);
        Node cur = nodes[k];
        ++r.expanded;
        Vec3 cp = g.center(x, y, z);
        float dg = (cp - goal).length();
        if (dg < bestD) { bestD = dg; bestP = cp; }
        if (dg < 0.9f) { r.ok = true; r.length = cur.g; return r; }
        if (r.expanded > kLimit) { r.limit = true; break; }
        for (int dz = -1; dz <= 1; ++dz) for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
            if (!dx && !dy && !dz) continue;
            int nx = x + dx, ny = y + dy, nz = z + dz;
            if (nx < rx0 || nx > rx1 || ny < ry0 || ny > ry1 || nz < rz0 || nz > rz1) continue;
            if (!g.isFree(nx, ny, nz)) continue;
            // evita cortar quinas: diagonais exigem os vizinhos axiais livres
            if (dx && dy && dz == 0 && !(g.isFree(x + dx, y, z) && g.isFree(x, y + dy, z))) continue;
            float step = std::sqrt((float)(dx * dx + dy * dy + dz * dz)) * g.c;
            int64_t nk = Grid::key(nx, ny, nz);
            float ng = cur.g + step;
            auto it = nodes.find(nk);
            if (it == nodes.end() || ng < it->second.g) { nodes[nk] = {ng, k}; open.push({ng + h(nx, ny, nz), nk}); }
        }
    }
    r.stuckAt = bestP; r.stuckDist = bestD;
    return r;
}

QString fmt(const Vec3& p) { return QStringLiteral("(%1, %2, %3)").arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1).arg(p.z(), 0, 'f', 1); }

} // namespace

void probeLine(World& w);
int runValidation(World& w, const QString& reportPath)
{
    probeLine(w);
    QStringList lines;
    int fails = 0;
    lines << QStringLiteral("=== Validação da masmorra ===");
    lines << QStringLiteral("Primitivas SDF: %1 | pontos de vista: %2 | POIs: %3 | pontos de rota: %4")
                 .arg(w.sdf.primCount()).arg(w.viewpoints().size()).arg(w.pois().size()).arg(w.route().size());
    const auto& rt = w.route();
    std::vector<std::future<PathResult>> futs(rt.size());
    for (size_t i = 0; i + 1 < rt.size(); ++i)
        if (!rt[i + 1].chainStart)
            futs[i] = std::async(std::launch::async, [&w, a = rt[i].pos, b = rt[i + 1].pos]() { return findPath(w, a, b); });
    for (size_t i = 0; i + 1 < rt.size(); ++i) {
        if (rt[i + 1].chainStart) { lines << QStringLiteral("----- trecho novo: %1 -----").arg(rt[i + 1].name); continue; }
        PathResult r = futs[i].get();
        QString head = QStringLiteral("rota %1 → %2  %3 → %4").arg(rt[i].name).arg(rt[i + 1].name).arg(fmt(rt[i].pos)).arg(fmt(rt[i + 1].pos));
        if (r.ok) lines << QStringLiteral("OK    %1  (caminho ≈ %2 m)").arg(head).arg(r.length, 0, 'f', 1);
        else {
            ++fails;
            if (r.startBlocked) lines << QStringLiteral("FALHA %1  — ponto inicial dentro da rocha").arg(head);
            else if (r.endBlocked) lines << QStringLiteral("FALHA %1  — ponto final dentro da rocha").arg(head);
            else lines << QStringLiteral("FALHA %1  — busca travou perto de %2, a %3 m do destino%4").arg(head).arg(fmt(r.stuckAt)).arg(r.stuckDist, 0, 'f', 1).arg(r.limit ? " (limite de busca)" : "");
        }
    }
    int inRock = 0;
    for (const Viewpoint& v : w.viewpoints()) {
        float d = w.sdf.distance(v.pos);
        if (d < 0.f) { ++inRock; lines << QStringLiteral("FALHA ponto de vista '%1' dentro da rocha %2 (d=%3)").arg(v.name).arg(fmt(v.pos)).arg(d, 0, 'f', 2); }
    }
    for (const auto& p : w.pois()) {
        float d = w.sdf.distance(p->pos);
        if (d < 0.f) { ++inRock; lines << QStringLiteral("FALHA POI '%1' dentro da rocha %2 (d=%3)").arg(p->title).arg(fmt(p->pos)).arg(d, 0, 'f', 2); }
    }
    fails += inRock;
    if (rt.size() < 2) lines << QStringLiteral("AVISO: menos de 2 pontos de rota — nada a conectar.");
    lines << (fails ? QStringLiteral("RESULTADO: %1 falha(s).").arg(fails) : QStringLiteral("RESULTADO: tudo OK."));
    for (const QString& l : lines) outLine(l);
    QFile f(reportPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) { f.write(lines.join('\n').toUtf8()); f.write("\n"); }
    return fails ? 1 : 0;
}

// Depuracao: MASMORRA_PROBE="x0,y0,z0,x1,y1,z1" imprime a distancia solida ao longo do segmento.
void probeLine(World& w)
{
    QByteArray e = qgetenv("MASMORRA_PROBE");
    if (e.isEmpty()) return;
    QList<QByteArray> p = e.split(',');
    if (p.size() != 6) return;
    Vec3 a(p[0].toFloat(), p[1].toFloat(), p[2].toFloat()), b(p[3].toFloat(), p[4].toFloat(), p[5].toFloat());
    for (int i = 0; i <= 40; ++i) {
        Vec3 q = a + (b - a) * (i / 40.f);
        outLine(QStringLiteral("probe %1 -> d=%2").arg(fmt(q)).arg(w.solidDistance(q, false), 0, 'f', 2));
    }
}
