#include "MeshBuilder.h"
#include "Materials.h"

void MeshData::append(const MeshData& o)
{
    uint32_t base = (uint32_t)v.size();
    v.insert(v.end(), o.v.begin(), o.v.end());
    i.reserve(i.size() + o.i.size());
    for (uint32_t k : o.i) i.push_back(k + base);
    bounds.expand(o.bounds);
}

MeshBuilder::MeshBuilder() : m_cur(&m_opaque) { m_m.setToIdentity(); updateNm(); }

void MeshBuilder::updateNm() { m_nmat = m_m.inverted().transposed(); }
void MeshBuilder::push() { m_stack.push_back(m_m); }
void MeshBuilder::pop() { if (!m_stack.empty()) { m_m = m_stack.back(); m_stack.pop_back(); updateNm(); } }
void MeshBuilder::identity() { m_m.setToIdentity(); updateNm(); }
void MeshBuilder::translate(const Vec3& t) { m_m.translate(t); updateNm(); }
void MeshBuilder::rotate(float d, const Vec3& axis) { m_m.rotate(d, axis); updateNm(); }
void MeshBuilder::rotate(const QQuaternion& q) { m_m.rotate(q); updateNm(); }
void MeshBuilder::scale(float s) { m_m.scale(s); updateNm(); }
void MeshBuilder::scale(const Vec3& s) { m_m.scale(s); updateNm(); }
void MeshBuilder::place(const Vec3& pos, float yawDeg, float s) { translate(pos); if (yawDeg != 0.f) rotateY(yawDeg); if (s != 1.f) scale(s); }
void MeshBuilder::setFlow(float dx, float dz)
{
    float l = std::sqrt(dx * dx + dz * dz);
    if (l < 1e-6f) { m_fx = m_fy = 0; return; }
    m_fx = (int8_t)std::lround(dx / l * 127.f); m_fy = (int8_t)std::lround(dz / l * 127.f);
}

void MeshBuilder::begin(int mat) { m_cur = materialIsTransparent(mat) ? &m_trans : &m_opaque; }

uint32_t MeshBuilder::addV(const Vec3& p, const Vec3& n, const QColor& c, int mat)
{
    Vec3 wp = m_m.map(p);
    Vec3 wn = m_nmat.mapVector(n);
    wn.normalize();
    Vertex v;
    v.px = wp.x(); v.py = wp.y(); v.pz = wp.z();
    v.nx = wn.x(); v.ny = wn.y(); v.nz = wn.z();
    v.r = (uint8_t)c.red(); v.g = (uint8_t)c.green(); v.b = (uint8_t)c.blue(); v.a = (uint8_t)c.alpha();
    v.mat = (uint8_t)mat; v.ao = m_ao; v.fx = m_fx; v.fy = m_fy;
    m_cur->v.push_back(v);
    m_cur->bounds.expand(wp);
    return (uint32_t)m_cur->v.size() - 1;
}

// Triangulo com correcao automatica de enrolamento (CCW visto de fora, usando as normais dos vertices).
void MeshBuilder::tri(uint32_t a, uint32_t b, uint32_t c)
{
    const Vertex &A = m_cur->v[a], &B = m_cur->v[b], &C = m_cur->v[c];
    Vec3 pa(A.px, A.py, A.pz), pb(B.px, B.py, B.pz), pc(C.px, C.py, C.pz);
    Vec3 gn = Vec3::crossProduct(pb - pa, pc - pa);
    Vec3 vn(A.nx + B.nx + C.nx, A.ny + B.ny + C.ny, A.nz + B.nz + C.nz);
    if (Vec3::dotProduct(gn, vn) < 0) std::swap(b, c);
    m_cur->i.push_back(a); m_cur->i.push_back(b); m_cur->i.push_back(c);
}
void MeshBuilder::quadIdx(uint32_t a, uint32_t b, uint32_t c, uint32_t d) { tri(a, b, c); tri(a, c, d); }

void MeshBuilder::box(const Vec3& c, const Vec3& h, const QColor& col, int mat) { boxMinMax(c - h, c + h, col, mat); }

void MeshBuilder::boxMinMax(const Vec3& mn, const Vec3& mx, const QColor& col, int mat)
{
    begin(mat);
    const Vec3 n[6] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    for (int f = 0; f < 6; ++f) {
        int ax = f / 2; float sgn = (f & 1) ? -1.f : 1.f;
        int u = (ax + 1) % 3, w = (ax + 2) % 3;
        float cv[3];
        uint32_t idx[4];
        for (int k = 0; k < 4; ++k) {
            cv[ax] = sgn > 0 ? mx[ax] : mn[ax];
            cv[u] = (k == 1 || k == 2) ? mx[u] : mn[u];
            cv[w] = (k >= 2) ? mx[w] : mn[w];
            idx[k] = addV(Vec3(cv[0], cv[1], cv[2]), n[f], col, mat);
        }
        quadIdx(idx[0], idx[1], idx[2], idx[3]);
    }
}

static void basis(const Vec3& d, Vec3& u, Vec3& v)
{
    Vec3 ref = std::fabs(d.y()) < 0.9f ? Vec3(0, 1, 0) : Vec3(1, 0, 0);
    u = Vec3::crossProduct(d, ref).normalized();
    v = Vec3::crossProduct(d, u).normalized();
}

void MeshBuilder::cylinder(const Vec3& a, const Vec3& b, float r0, float r1, int segs, const QColor& col, int mat, bool caps, bool smooth)
{
    begin(mat);
    Vec3 ax = b - a; float L = ax.length();
    if (L < 1e-5f) return;
    Vec3 d = ax / L, u, v; basis(d, u, v);
    segs = std::max(3, segs);
    float dr = r1 - r0;
    auto radial = [&](int i) { float t = (float)i / segs * 6.2831853f; return u * std::cos(t) + v * std::sin(t); };
    for (int i = 0; i < segs; ++i) {
        Vec3 ra = radial(i), rb = radial(i + 1);
        if (smooth) {
            Vec3 na = (ra * L - d * dr), nb = (rb * L - d * dr);
            uint32_t a0 = addV(a + ra * r0, na, col, mat), a1 = addV(b + ra * r1, na, col, mat);
            uint32_t b0 = addV(a + rb * r0, nb, col, mat), b1 = addV(b + rb * r1, nb, col, mat);
            quadIdx(a0, b0, b1, a1);
        } else {
            Vec3 rm = (ra + rb).normalized();
            Vec3 nn = rm * L - d * dr;
            uint32_t a0 = addV(a + ra * r0, nn, col, mat), a1 = addV(b + ra * r1, nn, col, mat);
            uint32_t b0 = addV(a + rb * r0, nn, col, mat), b1 = addV(b + rb * r1, nn, col, mat);
            quadIdx(a0, b0, b1, a1);
        }
    }
    if (caps) {
        for (int e = 0; e < 2; ++e) {
            float r = e ? r1 : r0; if (r <= 1e-5f) continue;
            Vec3 c = e ? b : a; Vec3 n = e ? d : -d;
            uint32_t ci = addV(c, n, col, mat);
            for (int i = 0; i < segs; ++i) {
                uint32_t p0 = addV(c + radial(i) * r, n, col, mat), p1 = addV(c + radial(i + 1) * r, n, col, mat);
                tri(ci, p0, p1);
            }
        }
    }
}

void MeshBuilder::ellipsoid(const Vec3& c, const Vec3& rad, const QColor& col, int mat, int segs)
{
    begin(mat);
    segs = std::max(4, segs);
    int rings = std::max(3, segs / 2 + 1);
    std::vector<uint32_t> idx;
    for (int i = 0; i <= rings; ++i) {
        float th = (float)i / rings * 3.14159265f;
        for (int j = 0; j <= segs; ++j) {
            float ph = (float)j / segs * 6.2831853f;
            Vec3 n(std::sin(th) * std::cos(ph), std::cos(th), std::sin(th) * std::sin(ph));
            Vec3 p = c + Vec3(n.x() * rad.x(), n.y() * rad.y(), n.z() * rad.z());
            Vec3 nn(n.x() / rad.x(), n.y() / rad.y(), n.z() / rad.z());
            idx.push_back(addV(p, nn, col, mat));
        }
    }
    int W = segs + 1;
    for (int i = 0; i < rings; ++i)
        for (int j = 0; j < segs; ++j) {
            uint32_t a = idx[i * W + j], b = idx[i * W + j + 1], c2 = idx[(i + 1) * W + j + 1], d = idx[(i + 1) * W + j];
            if (i > 0) tri(a, b, c2);
            if (i < rings - 1) tri(a, c2, d);
        }
}
void MeshBuilder::sphere(const Vec3& c, float r, const QColor& col, int mat, int segs) { ellipsoid(c, Vec3(r, r, r), col, mat, segs); }

void MeshBuilder::capsule(const Vec3& a, const Vec3& b, float r, const QColor& col, int mat, int segs)
{
    cylinder(a, b, r, r, segs, col, mat, false);
    sphere(a, r, col, mat, segs);
    sphere(b, r, col, mat, segs);
}

void MeshBuilder::torus(const Vec3& c, const Vec3& axis, float R, float r, const QColor& col, int mat, int sM, int sm)
{
    begin(mat);
    Vec3 ax = axis.normalized(), u, v; basis(ax, u, v);
    std::vector<uint32_t> idx;
    for (int i = 0; i < sM; ++i) {
        float t = (float)i / sM * 6.2831853f;
        Vec3 dir = u * std::cos(t) + v * std::sin(t);
        for (int j = 0; j < sm; ++j) {
            float p = (float)j / sm * 6.2831853f;
            Vec3 n = dir * std::cos(p) + ax * std::sin(p);
            idx.push_back(addV(c + dir * R + n * r, n, col, mat));
        }
    }
    for (int i = 0; i < sM; ++i)
        for (int j = 0; j < sm; ++j) {
            int i2 = (i + 1) % sM, j2 = (j + 1) % sm;
            quadIdx(idx[i * sm + j], idx[i2 * sm + j], idx[i2 * sm + j2], idx[i * sm + j2]);
        }
}

void MeshBuilder::hexPrism(const Vec3& c, float radius, float height, const QColor& col, int mat)
{
    cylinder(c - Vec3(0, height * 0.5f, 0), c + Vec3(0, height * 0.5f, 0), radius, radius, 6, col, mat, true, false);
}

void MeshBuilder::quad(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const QColor& col, int mat)
{
    begin(mat);
    Vec3 n = Vec3::crossProduct(p1 - p0, p3 - p0);
    if (n.lengthSquared() < 1e-12f) n = Vec3::crossProduct(p2 - p1, p2 - p3);
    uint32_t a = addV(p0, n, col, mat), b = addV(p1, n, col, mat), c = addV(p2, n, col, mat), d = addV(p3, n, col, mat);
    m_cur->i.insert(m_cur->i.end(), {a, b, c, a, c, d}); // enrolamento do chamador (de dois lados no shader)
}

void MeshBuilder::triangle(const Vec3& p0, const Vec3& p1, const Vec3& p2, const QColor& col, int mat)
{
    begin(mat);
    Vec3 n = Vec3::crossProduct(p1 - p0, p2 - p0);
    uint32_t a = addV(p0, n, col, mat), b = addV(p1, n, col, mat), c = addV(p2, n, col, mat);
    m_cur->i.insert(m_cur->i.end(), {a, b, c});
}

void MeshBuilder::tube(const std::vector<Vec3>& pts, const std::vector<float>& radii, int segs, const QColor& col, int mat, bool caps)
{
    begin(mat);
    size_t n = pts.size();
    if (n < 2) return;
    segs = std::max(3, segs);
    auto rad = [&](size_t i) { return radii.empty() ? 0.05f : (radii.size() == 1 ? radii[0] : radii[std::min(i, radii.size() - 1)]); };
    Vec3 d0 = (pts[1] - pts[0]).normalized(), u, v; basis(d0, u, v);
    std::vector<std::vector<uint32_t>> rings(n);
    std::vector<std::vector<Vec3>> ringPos(n);
    Vec3 prevD = d0;
    for (size_t i = 0; i < n; ++i) {
        Vec3 d = i == 0 ? d0 : (i == n - 1 ? (pts[i] - pts[i - 1]).normalized() : (pts[i + 1] - pts[i - 1]).normalized());
        if (d.lengthSquared() < 0.5f) d = prevD;
        Vec3 axis = Vec3::crossProduct(prevD, d);
        float s = axis.length();
        if (s > 1e-5f) { // transporte paralelo do referencial
            float ang = std::asin(std::min(1.f, s)) * 57.2957795f;
            if (Vec3::dotProduct(prevD, d) < 0) ang = 180.f - ang;
            QQuaternion q = QQuaternion::fromAxisAndAngle(axis / s, ang);
            u = q.rotatedVector(u); v = q.rotatedVector(v);
        }
        prevD = d;
        for (int k = 0; k < segs; ++k) {
            float t = (float)k / segs * 6.2831853f;
            Vec3 rdir = u * std::cos(t) + v * std::sin(t);
            Vec3 p = pts[i] + rdir * rad(i);
            ringPos[i].push_back(p);
            rings[i].push_back(addV(p, rdir, col, mat));
        }
    }
    for (size_t i = 0; i + 1 < n; ++i)
        for (int k = 0; k < segs; ++k) {
            int k2 = (k + 1) % segs;
            quadIdx(rings[i][k], rings[i][k2], rings[i + 1][k2], rings[i + 1][k]);
        }
    if (caps) {
        for (int e = 0; e < 2; ++e) {
            size_t i = e ? n - 1 : 0;
            Vec3 nn = e ? (pts[n - 1] - pts[n - 2]).normalized() : (pts[0] - pts[1]).normalized();
            uint32_t c = addV(pts[i], nn, col, mat);
            for (int k = 0; k < segs; ++k) {
                uint32_t a = addV(ringPos[i][k], nn, col, mat);
                uint32_t b = addV(ringPos[i][(k + 1) % segs], nn, col, mat);
                tri(c, a, b);
            }
        }
    }
}

std::vector<Vec3> MeshBuilder::catenary(const Vec3& a, const Vec3& b, float sag, int n)
{
    std::vector<Vec3> r;
    for (int i = 0; i <= n; ++i) {
        float t = (float)i / n;
        Vec3 p = a + (b - a) * t;
        p.setY(p.y() - sag * 4.f * t * (1.f - t));
        r.push_back(p);
    }
    return r;
}

Aabb MeshBuilder::bounds() const { Aabb b = m_opaque.bounds; b.expand(m_trans.bounds); return b; }
void MeshBuilder::append(const MeshBuilder& o) { m_opaque.append(o.m_opaque); m_trans.append(o.m_trans); }
void MeshBuilder::clear() { m_opaque.clear(); m_trans.clear(); }
