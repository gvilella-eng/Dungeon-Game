// Tipos geometricos basicos compartilhados.
#pragma once
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QColor>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

using Vec3 = QVector3D;

struct Aabb {
    Vec3 mn{1e30f, 1e30f, 1e30f};
    Vec3 mx{-1e30f, -1e30f, -1e30f};
    Aabb() = default;
    Aabb(Vec3 a, Vec3 b) : mn(a), mx(b) {}
    bool valid() const { return mn.x() <= mx.x(); }
    void expand(const Vec3& p) {
        mn = Vec3(std::min(mn.x(), p.x()), std::min(mn.y(), p.y()), std::min(mn.z(), p.z()));
        mx = Vec3(std::max(mx.x(), p.x()), std::max(mx.y(), p.y()), std::max(mx.z(), p.z()));
    }
    void expand(const Aabb& o) { if (o.valid()) { expand(o.mn); expand(o.mx); } }
    void inflate(float m) { mn -= Vec3(m, m, m); mx += Vec3(m, m, m); }
    Aabb inflated(float m) const { Aabb r = *this; r.inflate(m); return r; }
    Vec3 center() const { return (mn + mx) * 0.5f; }
    Vec3 size() const { return mx - mn; }
    bool contains(const Vec3& p) const {
        return p.x() >= mn.x() && p.x() <= mx.x() && p.y() >= mn.y() && p.y() <= mx.y() && p.z() >= mn.z() && p.z() <= mx.z();
    }
    bool overlaps(const Aabb& o) const {
        return mn.x() <= o.mx.x() && mx.x() >= o.mn.x() && mn.y() <= o.mx.y() && mx.y() >= o.mn.y() && mn.z() <= o.mx.z() && mx.z() >= o.mn.z();
    }
    static Aabb fromCenterHalf(Vec3 c, Vec3 h) { return Aabb(c - h, c + h); }
};

// Transforma um AABB por uma matriz (retorna AABB envolvente).
inline Aabb transformAabb(const QMatrix4x4& m, const Aabb& b)
{
    Aabb r;
    for (int i = 0; i < 8; ++i)
        r.expand(m.map(Vec3((i & 1) ? b.mx.x() : b.mn.x(), (i & 2) ? b.mx.y() : b.mn.y(), (i & 4) ? b.mx.z() : b.mn.z())));
    return r;
}

struct Frustum {
    QVector4D planes[6];
    void set(const QMatrix4x4& vp) {
        const float* m = vp.constData(); // coluna-maior
        auto row = [&](int r) { return QVector4D(m[0 + r], m[4 + r], m[8 + r], m[12 + r]); };
        QVector4D r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);
        planes[0] = r3 + r0; planes[1] = r3 - r0; planes[2] = r3 + r1;
        planes[3] = r3 - r1; planes[4] = r3 + r2; planes[5] = r3 - r2;
        for (auto& p : planes) { float l = Vec3(p.x(), p.y(), p.z()).length(); if (l > 0) p /= l; }
    }
    bool intersects(const Aabb& b) const {
        for (const auto& p : planes) {
            Vec3 v(p.x() > 0 ? b.mx.x() : b.mn.x(), p.y() > 0 ? b.mx.y() : b.mn.y(), p.z() > 0 ? b.mx.z() : b.mn.z());
            if (p.x() * v.x() + p.y() * v.y() + p.z() * v.z() + p.w() < 0) return false;
        }
        return true;
    }
};

inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
inline float smoothstepf(float a, float b, float x) { float t = clampf((x - a) / (b - a), 0.f, 1.f); return t * t * (3 - 2 * t); }
inline Vec3 lerp3(const Vec3& a, const Vec3& b, float t) { return a + (b - a) * t; }

// Hash FNV-1a incremental (cache do mundo).
struct Hasher {
    uint64_t h = 1469598103934665603ULL;
    void bytes(const void* d, size_t n) { auto p = (const uint8_t*)d; for (size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ULL; } }
    template <class T> void add(const T& v) { bytes(&v, sizeof(T)); }
    void add(const Vec3& v) { add(v.x()); add(v.y()); add(v.z()); }
    void add(const QString& s) { auto u = s.toUtf8(); bytes(u.constData(), (size_t)u.size()); }
};
