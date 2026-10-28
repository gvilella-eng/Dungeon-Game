// Ruido de gradiente 3D deterministico (sem tabelas), saida aproximada em [-1, 1].
#pragma once
#include "Geom.h"

namespace Noise {

inline uint32_t ihash(int x, int y, int z)
{
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + (uint32_t)z * 2147483647u + 1013904223u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

inline float grad(uint32_t h, float x, float y, float z)
{
    switch (h & 15) {
    case 0: return x + y;  case 1: return -x + y;  case 2: return x - y;  case 3: return -x - y;
    case 4: return x + z;  case 5: return -x + z;  case 6: return x - z;  case 7: return -x - z;
    case 8: return y + z;  case 9: return -y + z;  case 10: return y - z; case 11: return -y - z;
    case 12: return x + y; case 13: return -y + z; case 14: return -x + y; default: return -y - z;
    }
}

inline float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }

inline float gnoise(float x, float y, float z)
{
    float fx = std::floor(x), fy = std::floor(y), fz = std::floor(z);
    int ix = (int)fx, iy = (int)fy, iz = (int)fz;
    x -= fx; y -= fy; z -= fz;
    float u = fade(x), v = fade(y), w = fade(z);
    auto g = [&](int dx, int dy, int dz) { return grad(ihash(ix + dx, iy + dy, iz + dz), x - dx, y - dy, z - dz); };
    float a = lerpf(lerpf(g(0, 0, 0), g(1, 0, 0), u), lerpf(g(0, 1, 0), g(1, 1, 0), u), v);
    float b = lerpf(lerpf(g(0, 0, 1), g(1, 0, 1), u), lerpf(g(0, 1, 1), g(1, 1, 1), u), v);
    return lerpf(a, b, w) * 0.9f;
}

// Ruido de mundo usado pelas primitivas SDF: warp de baixa frequencia + 3 oitavas.
// Gradiente maximo ~0.35 por unidade de amplitude (mantem o campo ~Lipschitz 1 com amp <= ~1.5 m).
inline float worldNoise(const Vec3& p)
{
    float w = gnoise(p.x() * 0.045f + 11.3f, p.y() * 0.045f - 4.1f, p.z() * 0.045f + 7.7f) * 3.0f;
    float x = p.x() + w, y = p.y() + w * 0.6f, z = p.z() - w * 0.8f;
    float n = 0.62f * gnoise(x * 0.11f, y * 0.11f, z * 0.11f)
            + 0.30f * gnoise(x * 0.37f + 5.2f, y * 0.37f + 1.7f, z * 0.37f + 9.1f)
            + 0.10f * gnoise(x * 0.95f + 3.3f, y * 0.95f + 8.8f, z * 0.95f + 2.2f);
    return n;
}

} // namespace Noise
