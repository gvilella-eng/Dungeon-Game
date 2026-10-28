// Utilitarios compartilhados das camadas 2 e 3 (Poco do Eco / Veias Vitreas).
#pragma once
#include "world/World.h"
#include "Prefabs.h"
#include <cmath>
#include <random>
#include <vector>

namespace L23 {

inline float rad(float d) { return d * 0.0174532925f; }
inline float deg(float r) { return r * 57.2957795f; }
inline Vec3 V3(float x, float y, float z) { return Vec3(x, y, z); }
// yaw (graus) para olhar de 'from' para 'to' no plano XZ (0 = -Z, positivo = esquerda)
inline float yawToward(const Vec3& from, const Vec3& to) { return deg(std::atan2(-(to.x() - from.x()), -(to.z() - from.z()))); }
// yaw para uma direcao horizontal
inline float yawDir(float dx, float dz) { return deg(std::atan2(-dx, -dz)); }

// tunel com o CHAO nos pontos dados (eixo sobe raio*0.55)
inline void tun(World& w, std::vector<Vec3> fp, float r, int mat, float noise = 0.35f, float k = 1.2f)
{
    for (auto& p : fp) p.setY(p.y() + r * 0.55f);
    TunnelOpts o; o.floorFrac = 0.55f; o.material = mat; o.noise = noise; o.k = k;
    w.carveTunnel(fp, r, o);
}
inline void tunR(World& w, std::vector<Vec3> fp, std::vector<float> radii, int mat, float noise = 0.35f, float k = 1.2f)
{
    for (size_t i = 0; i < fp.size(); ++i) fp[i].setY(fp[i].y() + radii[std::min(i, radii.size() - 1)] * 0.55f);
    TunnelOpts o; o.floorFrac = 0.55f; o.material = mat; o.noise = noise; o.k = k;
    w.carveTunnel(fp, radii, o);
}

// altura do chao sob (x, yHint, z) por raycast para baixo (ignora colisores 'openable')
inline float groundY(const World& w, float x, float yHint, float z, float up = 2.5f, float maxd = 10.f)
{
    RayHit h = w.raycast(Vec3(x, yHint + up, z), Vec3(0, -1, 0), maxd + up, false);
    return h.hit ? h.pos.y() : yHint;
}

// quaternion que leva +Y para 'dir'
inline QQuaternion rotY2(const Vec3& dir) { return QQuaternion::rotationTo(Vec3(0, 1, 0), dir.normalized()); }

// referencial com X ao longo de t (tangente) e Y aproximadamente para cima
inline QQuaternion frameTangent(const Vec3& t)
{
    Vec3 X = t.normalized();
    Vec3 Z = Vec3::crossProduct(X, Vec3(0, 1, 0)).normalized();
    Vec3 Y = Vec3::crossProduct(Z, X);
    return QQuaternion::fromAxes(X, Y, Z);
}

// aleatorio deterministico
struct Rnd {
    std::mt19937 g;
    explicit Rnd(unsigned s) : g(s) {}
    float f(float a = 0.f, float b = 1.f) { return a + (b - a) * ((g() & 0xFFFFFF) / 16777216.f); }
    int i(int a, int b) { return a + (int)(g() % (unsigned)(b - a + 1)); }
};

inline QColor shadeC(const QColor& c, float f)
{
    auto cl = [](float v) { return std::max(0, std::min(255, (int)std::lround(v))); };
    return QColor(cl(c.red() * f), cl(c.green() * f), cl(c.blue() * f), c.alpha());
}

} // namespace L23
