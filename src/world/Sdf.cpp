#include "Sdf.h"
#include "Noise.h"

static const float INF = 1e30f;

static inline float smin(float a, float b, float k)
{
    if (k <= 0.f) return std::min(a, b);
    float h = std::max(k - std::fabs(a - b), 0.f) / k;
    return std::min(a, b) - h * h * k * 0.25f;
}
static inline float smax(float a, float b, float k) { return -smin(-a, -b, k); }
static inline float dot3(const Vec3& a, const Vec3& b) { return a.x() * b.x() + a.y() * b.y() + a.z() * b.z(); }

// ---- fabricas ----
Prim Prim::sphere(Vec3 c, float r)
{
    Prim p; p.shape = Shape::Sphere; p.a = c; p.r0 = r;
    p.bounds = Aabb(c - Vec3(r, r, r), c + Vec3(r, r, r));
    return p;
}
Prim Prim::ellipsoid(Vec3 c, Vec3 radii)
{
    Prim p; p.shape = Shape::Ellipsoid; p.a = c; p.b = radii;
    p.bounds = Aabb(c - radii, c + radii);
    return p;
}
Prim Prim::box(Vec3 c, Vec3 half, QQuaternion rot, float round)
{
    Prim p; p.shape = Shape::Box; p.a = c; p.b = half; p.round = round;
    Vec3 cols[3] = {rot.rotatedVector(Vec3(1, 0, 0)), rot.rotatedVector(Vec3(0, 1, 0)), rot.rotatedVector(Vec3(0, 0, 1))};
    for (int j = 0; j < 3; ++j) p.rotRows[j] = cols[j];
    Vec3 e;
    for (int i = 0; i < 3; ++i) {
        float s = round;
        for (int j = 0; j < 3; ++j) s += std::fabs(cols[j][i]) * half[j];
        e[i] = s;
    }
    p.bounds = Aabb(c - e, c + e);
    return p;
}
Prim Prim::capsule(Vec3 a, Vec3 b, float r0, float r1)
{
    Prim p; p.shape = Shape::Capsule; p.a = a; p.b = b; p.r0 = r0; p.r1 = r1;
    float r = std::max(r0, r1);
    p.bounds.expand(a); p.bounds.expand(b); p.bounds.inflate(r);
    return p;
}
Prim Prim::cone(Vec3 apex, Vec3 base, float baseRadius, float tipRadius)
{
    Prim p = capsule(apex, base, tipRadius, baseRadius);
    p.shape = Shape::Cone;
    return p;
}
Prim Prim::cylinder(Vec3 c, float r, float yMin, float yMax)
{
    Prim p; p.shape = Shape::Cylinder; p.a = c; p.r0 = r; p.y0 = yMin; p.y1 = yMax;
    p.bounds = Aabb(Vec3(c.x() - r, yMin, c.z() - r), Vec3(c.x() + r, yMax, c.z() + r));
    return p;
}
Prim Prim::helix(const HelixDesc& d)
{
    Prim p; p.shape = Shape::Helix; p.hx = d; p.a = d.center;
    float yTop = d.startY - d.dropPerTurn * std::min(d.t0, d.t1) + 0.5f;
    float yBot = d.startY - d.dropPerTurn * std::max(d.t0, d.t1) - d.thickness - 0.5f;
    p.bounds = Aabb(Vec3(d.center.x() - d.rOuter, yBot, d.center.z() - d.rOuter), Vec3(d.center.x() + d.rOuter, yTop, d.center.z() + d.rOuter));
    return p;
}
Prim Prim::annulus(Vec3 c, float rIn, float rOut, float yMin, float yMax)
{
    Prim p; p.shape = Shape::Annulus; p.a = c; p.r0 = rIn; p.r1 = rOut; p.y0 = yMin; p.y1 = yMax;
    p.bounds = Aabb(Vec3(c.x() - rOut, yMin, c.z() - rOut), Vec3(c.x() + rOut, yMax, c.z() + rOut));
    return p;
}

// ---- SdfWorld ----
SdfWorld::SdfWorld() { setBounds(Aabb(Vec3(-300, -420, -740), Vec3(780, 140, 280)), 12.f); }

void SdfWorld::setBounds(const Aabb& b, float cs)
{
    m_bounds = b; m_cs = cs;
    Vec3 s = b.size();
    m_nx = std::max(1, (int)std::ceil(s.x() / cs)); m_ny = std::max(1, (int)std::ceil(s.y() / cs)); m_nz = std::max(1, (int)std::ceil(s.z() / cs));
    m_carveCells.assign((size_t)m_nx * m_ny * m_nz, {});
    m_fillCells.assign((size_t)m_nx * m_ny * m_nz, {});
    m_prims.clear();
    m_content = Aabb();
}

inline int SdfWorld::cellIndex(const Vec3& p) const
{
    int ix = (int)std::floor((p.x() - m_bounds.mn.x()) / m_cs);
    int iy = (int)std::floor((p.y() - m_bounds.mn.y()) / m_cs);
    int iz = (int)std::floor((p.z() - m_bounds.mn.z()) / m_cs);
    if ((unsigned)ix >= (unsigned)m_nx || (unsigned)iy >= (unsigned)m_ny || (unsigned)iz >= (unsigned)m_nz) return -1;
    return (iz * m_ny + iy) * m_nx + ix;
}

int SdfWorld::add(const Prim& p)
{
    int id = (int)m_prims.size();
    m_prims.push_back(p);
    m_content.expand(p.bounds);
    Aabb inf = p.bounds.inflated(p.k + p.noiseAmp * 1.5f + kMargin);
    int x0 = std::max(0, (int)std::floor((inf.mn.x() - m_bounds.mn.x()) / m_cs)), x1 = std::min(m_nx - 1, (int)std::floor((inf.mx.x() - m_bounds.mn.x()) / m_cs));
    int y0 = std::max(0, (int)std::floor((inf.mn.y() - m_bounds.mn.y()) / m_cs)), y1 = std::min(m_ny - 1, (int)std::floor((inf.mx.y() - m_bounds.mn.y()) / m_cs));
    int z0 = std::max(0, (int)std::floor((inf.mn.z() - m_bounds.mn.z()) / m_cs)), z1 = std::min(m_nz - 1, (int)std::floor((inf.mx.z() - m_bounds.mn.z()) / m_cs));
    auto& cells = p.carve ? m_carveCells : m_fillCells;
    for (int z = z0; z <= z1; ++z)
        for (int y = y0; y <= y1; ++y)
            for (int x = x0; x <= x1; ++x) cells[((size_t)z * m_ny + y) * m_nx + x].push_back(id);
    return id;
}

void SdfWorld::setTerrain(HeightFn h, const Aabb& box)
{
    m_terr.push_back({std::move(h), box});
    m_content.expand(box);
}

// distancia da primitiva (negativa dentro), com ruido e corte de chao
float SdfWorld::evalPrim(const Prim& pr, const Vec3& p, float* nz, bool* nzDone) const
{
    float s = 0.f, level = -INF;
    float t = 0.f;
    switch (pr.shape) {
    case Shape::Sphere: s = (p - pr.a).length() - pr.r0; break;
    case Shape::Ellipsoid: {
        Vec3 q = p - pr.a;
        Vec3 qa(q.x() / pr.b.x(), q.y() / pr.b.y(), q.z() / pr.b.z());
        Vec3 qb(qa.x() / pr.b.x(), qa.y() / pr.b.y(), qa.z() / pr.b.z());
        float k0 = qa.length(), k1 = qb.length();
        s = k1 < 1e-8f ? -std::min(pr.b.x(), std::min(pr.b.y(), pr.b.z())) : k0 * (k0 - 1.f) / k1;
        break;
    }
    case Shape::Box: {
        Vec3 d = p - pr.a;
        float lx = std::fabs(dot3(pr.rotRows[0], d)) - pr.b.x() + pr.round;
        float ly = std::fabs(dot3(pr.rotRows[1], d)) - pr.b.y() + pr.round;
        float lz = std::fabs(dot3(pr.rotRows[2], d)) - pr.b.z() + pr.round;
        float ox = std::max(lx, 0.f), oy = std::max(ly, 0.f), oz = std::max(lz, 0.f);
        s = std::sqrt(ox * ox + oy * oy + oz * oz) + std::min(std::max(lx, std::max(ly, lz)), 0.f) - pr.round;
        break;
    }
    case Shape::Capsule:
    case Shape::Cone: {
        Vec3 ba = pr.b - pr.a;
        float l2 = dot3(ba, ba);
        if (l2 < 1e-8f) { s = (p - pr.a).length() - pr.r0; break; }
        float r1 = pr.r0, r2 = pr.r1;
        float rr = r1 - r2, a2 = std::max(l2 - rr * rr, 1e-6f), il2 = 1.f / l2;
        Vec3 pa = p - pr.a;
        float y = dot3(pa, ba), z = y - l2;
        Vec3 q = pa * l2 - ba * y;
        float x2 = dot3(q, q), y2 = y * y * l2, z2 = z * z * l2;
        float k = (rr > 0 ? 1.f : (rr < 0 ? -1.f : 0.f)) * rr * rr * x2;
        if ((z > 0 ? 1.f : -1.f) * a2 * z2 > k) s = std::sqrt(x2 + z2) * il2 - r2;
        else if ((y > 0 ? 1.f : -1.f) * a2 * y2 < k) s = std::sqrt(x2 + y2) * il2 - r1;
        else s = (std::sqrt(x2 * a2 * il2) + y * rr) * il2 - r1;
        t = clampf(y * il2, 0.f, 1.f);
        if (pr.floorMode == 2) {
            float axisY = pr.a.y() + (pr.b.y() - pr.a.y()) * t;
            level = axisY - lerpf(r1, r2, t) * pr.floorFrac;
        }
        break;
    }
    case Shape::Cylinder: {
        float dx = p.x() - pr.a.x(), dz = p.z() - pr.a.z();
        float qx = std::sqrt(dx * dx + dz * dz) - pr.r0;
        float qy = std::fabs(p.y() - (pr.y0 + pr.y1) * 0.5f) - (pr.y1 - pr.y0) * 0.5f;
        s = std::sqrt(std::max(qx, 0.f) * std::max(qx, 0.f) + std::max(qy, 0.f) * std::max(qy, 0.f)) + std::min(std::max(qx, qy), 0.f);
        break;
    }
    case Shape::Annulus: {
        float dx = p.x() - pr.a.x(), dz = p.z() - pr.a.z();
        float qx = std::fabs(std::sqrt(dx * dx + dz * dz) - (pr.r0 + pr.r1) * 0.5f) - (pr.r1 - pr.r0) * 0.5f;
        float qy = std::fabs(p.y() - (pr.y0 + pr.y1) * 0.5f) - (pr.y1 - pr.y0) * 0.5f;
        s = std::sqrt(std::max(qx, 0.f) * std::max(qx, 0.f) + std::max(qy, 0.f) * std::max(qy, 0.f)) + std::min(std::max(qx, qy), 0.f);
        break;
    }
    case Shape::Helix: {
        const HelixDesc& h = pr.hx;
        float dx = p.x() - h.center.x(), dz = p.z() - h.center.z();
        float r = std::sqrt(dx * dx + dz * dz);
        float th = std::atan2(dz, dx) * 57.2957795f;
        float u = (th - h.startAngleDeg) * h.dir / 360.f;
        u -= std::floor(u);
        float tTarget = (h.startY - h.thickness * 0.5f - p.y()) / h.dropPerTurn;
        float n = std::round(tTarget - u);
        float tt = u + n;
        float yTop = h.startY - h.dropPerTurn * tt;
        float rc = (h.rInner + h.rOuter) * 0.5f, hw = (h.rOuter - h.rInner) * 0.5f;
        float slope = h.dropPerTurn / (6.2831853f * rc);
        float cs = 1.f / std::sqrt(1.f + slope * slope);
        float qx = std::fabs(r - rc) - hw;
        float qy = std::fabs(p.y() - (yTop - h.thickness * 0.5f)) * cs - h.thickness * 0.5f;
        float qa = (std::fabs(tt - (h.t0 + h.t1) * 0.5f) - (h.t1 - h.t0) * 0.5f) * 6.2831853f * rc * cs;
        float ox = std::max(qx, 0.f), oy = std::max(qy, 0.f), oa = std::max(qa, 0.f);
        s = std::sqrt(ox * ox + oy * oy + oa * oa) + std::min(std::max(qx, std::max(qy, qa)), 0.f);
        break;
    }
    }
    float nzv = 0.f;
    if (pr.noiseAmp > 0.f) {
        if (!*nzDone) { *nz = Noise::worldNoise(p); *nzDone = true; }
        nzv = *nz;
    }
    if (pr.floorMode != 0) {
        if (pr.floorMode == 1) level = pr.floorY;
        if (level > -INF * 0.5f) {
            float wall = lerpf(0.25f, 1.f, smoothstepf(0.f, 1.5f, p.y() - level));
            s += pr.noiseAmp * wall * nzv;
            s = smax(s, level + 0.08f * nzv - p.y(), 0.4f);
            return s;
        }
    }
    return s + pr.noiseAmp * nzv;
}

SdfWorld::Sample SdfWorld::eval(const Vec3& p) const
{
    int ci = cellIndex(p);
    float nz = 0.f; bool nzd = false;
    float rock = -kMargin;
    int mat = MAT_LIMESTONE;
    if (ci >= 0) {
        const auto& cl = m_carveCells[ci];
        if (!cl.empty()) {
            float air = INF, best = INF;
            for (int id : cl) {
                const Prim& pr = m_prims[id];
                float s = evalPrim(pr, p, &nz, &nzd);
                if (s < best) { best = s; mat = pr.material; }
                air = (air >= INF) ? s : smin(air, s, pr.k);
            }
            rock = -air;
        }
    }
    for (const Terrain& t : m_terr) {
        if (!t.box.contains(p)) continue;
        const float e = 0.5f;
        float h = t.h(p.x(), p.z());
        float hx = (t.h(p.x() + e, p.z()) - t.h(p.x() - e, p.z())) / (2 * e);
        float hz = (t.h(p.x(), p.z() + e) - t.h(p.x(), p.z() - e)) / (2 * e);
        float M = (p.y() - h) / std::sqrt(1.f + hx * hx + hz * hz);
        if (M > rock) { rock = M; mat = MAT_SNOW; }
    }
    if (ci >= 0) {
        const auto& fl = m_fillCells[ci];
        for (int id : fl) {
            const Prim& pr = m_prims[id];
            float f = evalPrim(pr, p, &nz, &nzd);
            if (f < rock) mat = pr.material;
            rock = smin(rock, f, pr.k);
        }
    }
    return Sample{rock, mat};
}

Vec3 SdfWorld::gradient(const Vec3& p, float e) const
{
    const float k0 = 1.f, k1 = -1.f;
    Vec3 g(0, 0, 0);
    // tetraedro
    static const float sx[4] = {k0, k1, k1, k0}, sy[4] = {k1, k1, k0, k0}, sz[4] = {k1, k0, k1, k0};
    for (int i = 0; i < 4; ++i) {
        float d = distance(p + Vec3(sx[i], sy[i], sz[i]) * e);
        g += Vec3(sx[i], sy[i], sz[i]) * d;
    }
    float l = g.length();
    return l > 1e-9f ? g / l : Vec3(0, 1, 0);
}

bool SdfWorld::maybeSurface(const Aabb& b) const
{
    if (overlapsExterior(b)) return true;
    int x0 = std::max(0, (int)std::floor((b.mn.x() - m_bounds.mn.x()) / m_cs)), x1 = std::min(m_nx - 1, (int)std::floor((b.mx.x() - m_bounds.mn.x()) / m_cs));
    int y0 = std::max(0, (int)std::floor((b.mn.y() - m_bounds.mn.y()) / m_cs)), y1 = std::min(m_ny - 1, (int)std::floor((b.mx.y() - m_bounds.mn.y()) / m_cs));
    int z0 = std::max(0, (int)std::floor((b.mn.z() - m_bounds.mn.z()) / m_cs)), z1 = std::min(m_nz - 1, (int)std::floor((b.mx.z() - m_bounds.mn.z()) / m_cs));
    for (int z = z0; z <= z1; ++z)
        for (int y = y0; y <= y1; ++y)
            for (int x = x0; x <= x1; ++x) {
                size_t i = ((size_t)z * m_ny + y) * m_nx + x;
                if (!m_carveCells[i].empty()) return true;
            }
    return false;
}

uint64_t SdfWorld::hash() const
{
    Hasher h;
    h.add(m_cs); h.add(m_bounds.mn); h.add(m_bounds.mx);
    for (const Prim& p : m_prims) {
        h.add((int)p.shape); h.add(p.carve); h.add(p.k); h.add(p.noiseAmp); h.add(p.material);
        h.add(p.floorMode); h.add(p.floorY); h.add(p.floorFrac);
        h.add(p.a); h.add(p.b); h.add(p.r0); h.add(p.r1); h.add(p.y0); h.add(p.y1); h.add(p.round);
        for (int j = 0; j < 3; ++j) h.add(p.rotRows[j]);
        h.add(p.hx.center); h.add(p.hx.rInner); h.add(p.hx.rOuter); h.add(p.hx.startY); h.add(p.hx.dropPerTurn);
        h.add(p.hx.startAngleDeg); h.add(p.hx.t0); h.add(p.hx.t1); h.add(p.hx.thickness); h.add(p.hx.dir);
    }
    for (const Terrain& t : m_terr) {
        h.add(t.box.mn); h.add(t.box.mx);
        for (int j = 0; j < 40; ++j)
            for (int i = 0; i < 40; ++i) {
                float x = lerpf(t.box.mn.x(), t.box.mx.x(), i / 39.f), z = lerpf(t.box.mn.z(), t.box.mx.z(), j / 39.f);
                h.add(t.h(x, z));
            }
    }
    return h.h;
}
