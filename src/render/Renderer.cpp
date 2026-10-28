#include "Renderer.h"
#include "world/Noise.h"
#include <algorithm>
#include <QDebug>

bool Renderer::init()
{
    bool ok = m_world.load("world.vert", "world.frag");
    ok &= m_particle.load("particle.vert", "particle.frag");
    ok &= m_beam.load("beam.vert", "beam.frag");
    ok &= m_post.init();
    ok &= m_dbg.init();
    makeNoise();
    GL->glGenVertexArrays(1, &m_pVao); GL->glGenBuffers(1, &m_pVbo);
    GL->glBindVertexArray(m_pVao); GL->glBindBuffer(GL_ARRAY_BUFFER, m_pVbo);
    GL->glEnableVertexAttribArray(0); GL->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
    GL->glEnableVertexAttribArray(1); GL->glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 32, (void*)12);
    GL->glEnableVertexAttribArray(2); GL->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 32, (void*)16);
    GL->glGenVertexArrays(1, &m_bVao); GL->glGenBuffers(1, &m_bVbo);
    GL->glBindVertexArray(m_bVao); GL->glBindBuffer(GL_ARRAY_BUFFER, m_bVbo);
    GL->glEnableVertexAttribArray(0); GL->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 36, (void*)0);
    GL->glEnableVertexAttribArray(1); GL->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 36, (void*)12);
    GL->glEnableVertexAttribArray(2); GL->glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 36, (void*)20);
    GL->glBindVertexArray(0);
    m_ready = ok;
    return ok;
}

// Textura 3D periodica: A = fbm de valor; RGB = gradiente (usado como normal-map procedural sem esticamento).
void Renderer::makeNoise()
{
    const int N = 64;
    const int sizes[4] = {4, 8, 16, 32};
    const float amps[4] = {0.50f, 0.28f, 0.14f, 0.08f};
    std::vector<float> lat[4];
    for (int o = 0; o < 4; ++o) {
        int s = sizes[o]; lat[o].resize((size_t)s * s * s);
        for (int z = 0; z < s; ++z) for (int y = 0; y < s; ++y) for (int x = 0; x < s; ++x)
            lat[o][((size_t)z * s + y) * s + x] = (Noise::ihash(x + o * 131, y + o * 71, z + o * 17) & 0xFFFF) / 65535.f;
    }
    std::vector<float> val((size_t)N * N * N);
    auto fade = [](float t) { return t * t * t * (t * (t * 6 - 15) + 10); };
    float mn = 1e9f, mx = -1e9f;
    for (int z = 0; z < N; ++z) for (int y = 0; y < N; ++y) for (int x = 0; x < N; ++x) {
        float v = 0;
        for (int o = 0; o < 4; ++o) {
            int s = sizes[o];
            float fx = (float)x / N * s, fy = (float)y / N * s, fz = (float)z / N * s;
            int ix = (int)fx, iy = (int)fy, iz = (int)fz;
            float u = fade(fx - ix), vv = fade(fy - iy), w = fade(fz - iz);
            auto L = [&](int dx, int dy, int dz) { return lat[o][(((size_t)((iz + dz) % s)) * s + ((iy + dy) % s)) * s + ((ix + dx) % s)]; };
            float a = lerpf(lerpf(L(0, 0, 0), L(1, 0, 0), u), lerpf(L(0, 1, 0), L(1, 1, 0), u), vv);
            float b = lerpf(lerpf(L(0, 0, 1), L(1, 0, 1), u), lerpf(L(0, 1, 1), L(1, 1, 1), u), vv);
            v += amps[o] * lerpf(a, b, w);
        }
        val[((size_t)z * N + y) * N + x] = v; mn = std::min(mn, v); mx = std::max(mx, v);
    }
    for (auto& v : val) v = clampf((v - mn) / (mx - mn), 0.f, 1.f);
    std::vector<uint8_t> px((size_t)N * N * N * 4);
    auto V = [&](int x, int y, int z) { return val[(((size_t)((z + N) % N)) * N + ((y + N) % N)) * N + ((x + N) % N)]; };
    for (int z = 0; z < N; ++z) for (int y = 0; y < N; ++y) for (int x = 0; x < N; ++x) {
        size_t i = (((size_t)z * N + y) * N + x) * 4;
        float gx = (V(x + 1, y, z) - V(x - 1, y, z)) * 0.5f * N, gy = (V(x, y + 1, z) - V(x, y - 1, z)) * 0.5f * N, gz = (V(x, y, z + 1) - V(x, y, z - 1)) * 0.5f * N;
        auto enc = [](float g) { return (uint8_t)(clampf(g / 16.f, -1.f, 1.f) * 127.5f + 127.5f); };
        px[i] = enc(gx); px[i + 1] = enc(gy); px[i + 2] = enc(gz);
        px[i + 3] = (uint8_t)(V(x, y, z) * 255.f);
    }
    GL->glGenTextures(1, &m_noiseTex);
    GL->glBindTexture(GL_TEXTURE_3D, m_noiseTex);
    GL->glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, N, N, N, 0, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    GL->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    GL->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    GL->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GL->glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    GL->glGenerateMipmap(GL_TEXTURE_3D);
}

void Renderer::addChunks(std::vector<ChunkMesh>& cs)
{
    for (auto& c : cs) {
        auto ch = std::make_unique<Chunk>();
        ch->mesh.upload(c.mesh); ch->exterior = c.exterior;
        c.mesh.clear(); c.mesh.v.shrink_to_fit(); c.mesh.i.shrink_to_fit();
        m_chunks.push_back(std::move(ch));
    }
    cs.clear();
}

void Renderer::clearChunks() { m_chunks.clear(); }

void Renderer::drawMesh(const GpuMesh& m, const QMatrix4x4& model)
{
    m_world.set("uModel", model);
    m_world.set("uNormalMat", model.normalMatrix());
    m.draw();
    ++drawCalls; triangles += m.indexCount() / 3;
}

void Renderer::setupWorldShader(const FrameParams& fp, World& w)
{
    Shader& s = m_world;
    s.bind();
    s.set("uVP", fp.proj * fp.view);
    s.set("uCamPos", fp.camPos); s.set("uTime", fp.time);
    s.set("uFullbright", fp.fullbright ? 1 : 0); s.set("uBackTint", fp.backTint ? 1 : 0);
    s.set("uNoFog", fp.noFog ? 1 : 0); s.set("uClipOn", fp.clipOn ? 1 : 0); s.set("uClip", fp.clip);
    s.set("uFogColor", fp.zone.fogColor); s.set("uFogDensity", fp.zone.fogDensity);
    s.set("uAmbSky", fp.zone.ambientColor); s.set("uAmbGround", fp.zone.ambientGround);
    s.set("uAmbUp", fp.ambUp); s.set("uAmbStr", fp.zone.ambientStrength);
    s.set("uHallu", fp.zone.hallucination); s.set("uDark", fp.darkvision);
    s.set("uXray", 0); s.set("uPass", 0);
    static QVector4D params[48], bases[48];
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 48; ++i) {
            const MaterialInfo& mi = materialInfo(i);
            params[i] = QVector4D(mi.roughness, mi.metal, mi.emissive, mi.wet);
            bases[i] = QVector4D(mi.base.x(), mi.base.y(), mi.base.z(), mi.transparent ? 1.f : 0.f);
        }
        init = true;
    }
    s.setArray("uMatParams", params, 48); s.setArray("uMatBase", bases, 48);
    s.set("uNoise", 0);
    GL->glActiveTexture(GL_TEXTURE0); GL->glBindTexture(GL_TEXTURE_3D, m_noiseTex);

    // selecao de luzes
    struct L { QVector4D pos, col; float score; };
    std::vector<L> ls;
    float t = fp.time;
    if (fp.torchOn) ls.push_back({QVector4D(fp.torchPos, fp.torchRange), QVector4D(fp.torchColor * fp.torchIntensity, 0), 1e9f});
    for (auto& l : w.lights()) {
        if (!l->enabled) continue;
        float d = (l->pos - fp.camPos).length();
        if (d > l->range + 40.f) continue;
        float inten = l->intensityAt(t);
        if (inten <= 0.001f) continue;
        float score = inten / (d * d + 4.f);
        if (!fp.torchOn && false) score += 0;
        ls.push_back({QVector4D(l->pos, l->range), QVector4D(l->color * inten, 0), score});
    }
    if (ls.size() > 32) { std::partial_sort(ls.begin(), ls.begin() + 32, ls.end(), [](const L& a, const L& b) { return a.score > b.score; }); ls.resize(32); }
    QVector4D lp[32], lc[32];
    for (size_t i = 0; i < ls.size(); ++i) { lp[i] = ls[i].pos; lc[i] = ls[i].col; }
    s.set("uNumLights", (int)ls.size());
    s.setArray("uLightPos", lp, 32); s.setArray("uLightCol", lc, 32);
}

void Renderer::render(World& w, const FrameParams& fp)
{
    drawCalls = 0; triangles = 0;
    m_post.resize(fp.w, fp.h);
    m_post.bindScene();
    m_frustum.set(fp.proj * fp.view);
    Vec3 clear = fp.fullbright ? Vec3(0.05f, 0.06f, 0.09f) : fp.zone.fogColor;
    GL->glClearColor(clear.x(), clear.y(), clear.z(), 1.f);
    GL->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    GL->glEnable(GL_DEPTH_TEST); GL->glDepthFunc(GL_LEQUAL); GL->glDepthMask(GL_TRUE);
    GL->glDisable(GL_CULL_FACE); GL->glDisable(GL_BLEND);
    GL->glEnable(GL_MULTISAMPLE);
    GL->glDisable(GL_SCISSOR_TEST); GL->glDisable(GL_STENCIL_TEST); GL->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    if (fp.wireframe) GL->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    setupWorldShader(fp, w);
    QMatrix4x4 ident;
    std::vector<TItem> trans;

    // ---- chunks de rocha ----
    for (auto& c : m_chunks) {
        if (fp.hideTerrain && c->exterior) continue;
        if (!c->mesh.valid() || !m_frustum.intersects(c->mesh.bounds)) continue;
        if (fp.xray) { trans.push_back({&c->mesh, ident, (c->mesh.bounds.center() - fp.camPos).length(), true}); continue; }
        drawMesh(c->mesh, ident);
    }
    // ---- props estaticos ----
    for (auto& kv : w.staticBatches()) {
        StaticBatch& b = kv.second;
        if (b.dirty) {
            if (!b.opaque.empty()) { if (!b.gpuO) b.gpuO = std::make_unique<GpuMesh>(); b.gpuO->upload(b.opaque); }
            if (!b.transparent.empty()) { if (!b.gpuT) b.gpuT = std::make_unique<GpuMesh>(); b.gpuT->upload(b.transparent); }
            b.dirty = false;
        }
        if (!m_frustum.intersects(b.bounds)) continue;
        if (b.gpuO && b.gpuO->valid()) drawMesh(*b.gpuO, ident);
        if (b.gpuT && b.gpuT->valid()) trans.push_back({b.gpuT.get(), ident, (b.gpuT->bounds.center() - fp.camPos).length(), false});
    }
    // ---- dinamicos ----
    for (auto& d : w.dynamics()) {
        if (d->dirty) {
            if (!d->opaque.empty()) { if (!d->gpuO) d->gpuO = std::make_unique<GpuMesh>(); d->gpuO->upload(d->opaque); }
            if (!d->transparent.empty()) { if (!d->gpuT) d->gpuT = std::make_unique<GpuMesh>(); d->gpuT->upload(d->transparent); }
            d->dirty = false;
        }
        if (!d->visible) continue;
        QMatrix4x4 m = d->matrix();
        if (!m_frustum.intersects(transformAabb(m, d->localBounds))) continue;
        if (d->gpuO && d->gpuO->valid()) drawMesh(*d->gpuO, m);
        if (d->gpuT && d->gpuT->valid()) trans.push_back({d->gpuT.get(), m, (m.map(d->localBounds.center()) - fp.camPos).length(), false});
    }
    // ---- transparentes (traseiro -> frontal) ----
    std::sort(trans.begin(), trans.end(), [](const TItem& a, const TItem& b) { return a.dist > b.dist; });
    GL->glEnable(GL_BLEND); GL->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    GL->glDepthMask(GL_FALSE);
    m_world.set("uPass", 1);
    for (auto& t : trans) {
        if (t.xray) { m_world.set("uXray", 1); m_world.set("uPass", 0); GL->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }
        drawMesh(*t.mesh, t.model);
        if (t.xray) { m_world.set("uXray", 0); m_world.set("uPass", 1); GL->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); }
    }
    if (fp.wireframe) GL->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    drawParticles(w, fp);
    drawBeams(w, fp);
    GL->glDepthMask(GL_TRUE);
    GL->glDisable(GL_BLEND);
    if (fp.debugDraw) drawDebug(w, fp);

    m_post.finish(fp.targetFbo, fp.post);
}

void Renderer::drawParticles(World& w, const FrameParams& fp)
{
    struct PV { float p[3]; float size; float c[4]; };
    std::vector<PV> groups[4];
    for (auto& e : w.emitters()) {
        const EmitterDesc& d = e->d;
        int g = (d.additive ? 2 : 0) + (d.glow ? 1 : 0);
        for (const Particle& p : e->parts) {
            if (!m_frustum.intersects(Aabb(p.pos - Vec3(0.2f, 0.2f, 0.2f), p.pos + Vec3(0.2f, 0.2f, 0.2f)))) continue;
            float t = 1.f - p.life / p.maxLife;
            // fade-in curto
            float fadeIn = std::min(1.f, t * 8.f);
            PV v;
            v.p[0] = p.pos.x(); v.p[1] = p.pos.y(); v.p[2] = p.pos.z(); v.size = p.size;
            v.c[0] = lerpf(d.colorStart.redF(), d.colorEnd.redF(), t); v.c[1] = lerpf(d.colorStart.greenF(), d.colorEnd.greenF(), t);
            v.c[2] = lerpf(d.colorStart.blueF(), d.colorEnd.blueF(), t); v.c[3] = lerpf(d.colorStart.alphaF(), d.colorEnd.alphaF(), t) * fadeIn;
            groups[g].push_back(v);
        }
    }
    bool any = false;
    for (auto& g : groups) any |= !g.empty();
    if (!any) return;
    m_particle.bind();
    m_particle.set("uVP", fp.proj * fp.view); m_particle.set("uCamPos", fp.camPos);
    m_particle.set("uProjScale", fp.proj(1, 1) * fp.h * 0.5f);
    Vec3 amb = (fp.zone.ambientColor + fp.zone.ambientGround) * 0.5f * fp.zone.ambientStrength;
    m_particle.set("uAmb", amb);
    m_particle.set("uTorchPos", QVector4D(fp.torchPos, fp.torchOn ? fp.torchRange : 0.1f));
    m_particle.set("uTorchCol", fp.torchOn ? fp.torchColor * fp.torchIntensity : Vec3());
    m_particle.set("uFogColor", fp.zone.fogColor); m_particle.set("uFogDensity", fp.noFog || fp.fullbright ? 0.f : fp.zone.fogDensity);
    m_particle.set("uFullbright", fp.fullbright ? 1 : 0);
    m_particle.set("uClipOn", fp.clipOn ? 1 : 0); m_particle.set("uClip", fp.clip);
    GL->glEnable(GL_PROGRAM_POINT_SIZE);
    GL->glEnable(GL_BLEND);
    GL->glBindVertexArray(m_pVao); GL->glBindBuffer(GL_ARRAY_BUFFER, m_pVbo);
    for (int g = 0; g < 4; ++g) {
        auto& v = groups[g];
        if (v.empty()) continue;
        if (v.size() > m_pCap) { m_pCap = v.size() * 2; GL->glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(m_pCap * sizeof(PV)), nullptr, GL_DYNAMIC_DRAW); }
        GL->glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(v.size() * sizeof(PV)), v.data());
        bool add = g >= 2;
        m_particle.set("uAdditive", add ? 1 : 0); m_particle.set("uGlow", (g & 1) ? 1 : 0);
        if (add) GL->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); else GL->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        GL->glDrawArrays(GL_POINTS, 0, (GLsizei)v.size());
        ++drawCalls;
    }
    GL->glDisable(GL_PROGRAM_POINT_SIZE);
    GL->glBindVertexArray(0);
}

void Renderer::drawBeams(World& w, const FrameParams& fp)
{
    struct BV { float p[3]; float uv[2]; float c[4]; };
    std::vector<BV> v;
    for (auto& b : w.beams()) {
        if (!b->visible) continue;
        Vec3 d = b->b - b->a;
        if (d.lengthSquared() < 1e-6f) continue;
        Vec3 mid = (b->a + b->b) * 0.5f;
        Vec3 r = Vec3::crossProduct(d, fp.camPos - mid);
        if (r.lengthSquared() < 1e-8f) continue;
        r = r.normalized() * (b->width * 0.5f);
        float c[4] = {(float)b->color.redF(), (float)b->color.greenF(), (float)b->color.blueF(), b->intensity};
        float len = d.length();
        auto push = [&](Vec3 p, float u, float vv) { BV x{{p.x(), p.y(), p.z()}, {u, vv}, {c[0], c[1], c[2], c[3]}}; v.push_back(x); };
        push(b->a - r, 0, 0); push(b->a + r, 1, 0); push(b->b + r, 1, len); push(b->a - r, 0, 0); push(b->b + r, 1, len); push(b->b - r, 0, len);
    }
    if (v.empty()) return;
    m_beam.bind(); m_beam.set("uVP", fp.proj * fp.view); m_beam.set("uTime", fp.time);
    GL->glEnable(GL_BLEND); GL->glBlendFunc(GL_ONE, GL_ONE);
    GL->glBindVertexArray(m_bVao); GL->glBindBuffer(GL_ARRAY_BUFFER, m_bVbo);
    if (v.size() > m_bCap) { m_bCap = v.size() * 2; GL->glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(m_bCap * sizeof(BV)), nullptr, GL_DYNAMIC_DRAW); }
    GL->glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(v.size() * sizeof(BV)), v.data());
    GL->glDrawArrays(GL_TRIANGLES, 0, (GLsizei)v.size());
    GL->glBindVertexArray(0);
    ++drawCalls;
}

void Renderer::drawDebug(World& w, const FrameParams& fp)
{
    DebugDraw& d = m_dbg;
    for (auto& z : w.zones()) {
        QColor c = z->fx.hazard ? QColor(255, 60, 40, 200) : z->fx.water ? QColor(60, 140, 255, 200) : z->fx.climb ? QColor(60, 255, 120, 200) : z->fx.gravity ? QColor(220, 90, 255, 200) : QColor(90, 160, 255, 110);
        const ZoneShape& s = z->shape;
        if (s.type == ZoneShape::BoxShape) d.box(s.box, c);
        else if (s.type == ZoneShape::SphereShape) d.sphere(s.center, s.radius, c);
        else d.cylinderY(s.center, s.radius, s.y0, s.y1, c);
    }
    for (auto& t : w.triggers()) {
        QColor c(255, 230, 60, 200);
        const ZoneShape& s = t->shape;
        if (s.type == ZoneShape::BoxShape) d.box(s.box, c);
        else if (s.type == ZoneShape::SphereShape) d.sphere(s.center, s.radius, c);
        else d.cylinderY(s.center, s.radius, s.y0, s.y1, c);
    }
    for (auto& c : w.colliders()) d.box(c->bounds, c->enabled ? (c->openable ? QColor(255, 150, 40, 220) : QColor(255, 50, 50, 220)) : QColor(120, 120, 120, 120));
    for (auto& i : w.interactables()) d.cross(i->pos, 0.25f, QColor(60, 255, 255, 230));
    for (auto& p : w.pois()) d.cross(p->pos, 0.3f, poiCategoryColor(p->category));
    for (auto& l : w.lights()) d.cross(l->pos, 0.15f, QColor(255, 220, 90, 200));
    const auto& rt = w.route();
    for (size_t i = 0; i < rt.size(); ++i) { d.cross(rt[i].pos, 0.4f, QColor(90, 255, 90, 230)); if (i && !rt[i].chainStart) d.line(rt[i - 1].pos, rt[i].pos, QColor(90, 255, 90, 120)); }
    d.flush(fp.proj * fp.view);
}
