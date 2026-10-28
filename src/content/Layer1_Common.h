// Camada 1 — A Garganta Erodida: utilitarios e estado compartilhado entre os arquivos Layer1_*.cpp.
#pragma once
#include "world/World.h"
#include "Prefabs.h"
#include <memory>
#include <vector>
#include <functional>

#define Q(s) QStringLiteral(s)

namespace L1 {

inline Vec3 V(float x, float y, float z) { return Vec3(x, y, z); }

// hash deterministico em [0,1)
inline float hrand(int a, int b = 0, int c = 0, int d = 0)
{
    uint32_t h = (uint32_t)a * 374761393u + (uint32_t)b * 668265263u + (uint32_t)c * 2147483647u + (uint32_t)d * 40503u + 1013904223u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xFFFFFF) / 16777216.f;
}

// Chao da antecamara (z 0 -> -28 desce de 0 a -2) e do corredor (z -28 -> -70 desce de -2 a -4)
inline float anteFloor(float z) { return z / 14.f; }
inline float corrFloor(float z) { return -2.f + 2.f * (z + 28.f) / 42.f; }

// Tunel de rocha cujos pontos sao no CHAO; raios por ponto (ou 1 raio). Chao em y_chao exato.
inline void tun(World& w, std::vector<Vec3> fp, std::vector<float> radii, int mat = MAT_LIMESTONE, float noise = 0.4f, float k = 0.8f)
{
    for (size_t i = 0; i < fp.size(); ++i) {
        float r = radii.size() == 1 ? radii[0] : radii[std::min(i, radii.size() - 1)];
        fp[i].setY(fp[i].y() + r * 0.55f);
    }
    TunnelOpts o; o.floorFrac = 0.55f; o.material = mat; o.noise = noise; o.k = k;
    w.carveTunnel(fp, radii, o);
}
inline void tun(World& w, std::vector<Vec3> fp, float r, int mat = MAT_LIMESTONE, float noise = 0.4f, float k = 0.8f)
{
    tun(w, std::move(fp), std::vector<float>{r}, mat, noise, k);
}

// Sala/corredor alto de largura 2*halfW e altura 'height' acima do chao, de fa a fb (pontos no chao do eixo).
// Feita de fileiras de capsulas com leve variacao (paredes escalonadas, nao um tubo liso).
inline void room(World& w, Vec3 fa, Vec3 fb, float halfW, float height, int mat, float noise, int seed, float k = 0.7f, float rScale = 0.5f)
{
    Vec3 d = fb - fa;
    Vec3 dh(d.x(), 0, d.z());
    float L = std::max(0.01f, dh.length());
    dh /= L;
    Vec3 side(dh.z(), 0, -dh.x());
    float R = std::min(std::max(height * rScale, 1.6f), halfW * 1.0f);
    int cols = 1;
    if (halfW > R * 1.15f) cols = (int)std::ceil((2 * halfW - 2 * R) / (1.5f * R)) + 1;
    float yA = 0.35f * R, yT = height - R;
    int rows = (yT - yA > 0.3f) ? (int)std::ceil((yT - yA) / (1.0f * R)) + 1 : 1;
    int nseg = std::max(1, (int)std::ceil(L / 11.f));
    for (int c = 0; c < cols; ++c) {
        float u = cols == 1 ? 0.f : -halfW + R + c * (2 * halfW - 2 * R) / (cols - 1);
        for (int j = 0; j < rows; ++j) {
            float yAx = rows == 1 ? yA : lerpf(yA, std::max(yA, yT), (float)j / (rows - 1));
            std::vector<Vec3> pts;
            for (int s = 0; s <= nseg; ++s) {
                float t = (float)s / nseg;
                float jl = (hrand(seed, c, j, s) - 0.5f) * (cols == 1 ? 0.5f : 0.5f * R);
                float jy = (j > 0) ? (hrand(seed + 7, c, j, s) - 0.5f) * 0.5f : 0.f;
                Vec3 p = fa + d * t + side * (u + jl);
                p.setY(p.y() + yAx + jy);
                pts.push_back(p);
            }
            float rr = R * (0.9f + 0.2f * hrand(seed + 3, c, j));
            for (int s = 0; s < nseg; ++s) {
                Prim pr = Prim::capsule(pts[s], pts[s + 1], rr, rr * (0.92f + 0.16f * hrand(seed + 5, c, j, s)));
                pr.smooth(k).noise(noise).mat(mat);
                if (j == 0) pr.floorRel(0.35f * R / rr);   // chao cortado no nivel dado (eixo - 0.35R)
                w.carve(pr);
            }
        }
    }
}

// Fenda estreita (caixa orientada): pontos no chao, largura total 2*halfW, altura h.
inline void slot(World& w, const std::vector<Vec3>& fp, float halfW, float h, int mat = MAT_LIMESTONE, float noise = 0.15f, float round = 0.35f)
{
    for (size_t i = 0; i + 1 < fp.size(); ++i) {
        Vec3 a = fp[i], b = fp[i + 1];
        Vec3 d = b - a;
        float len = d.length();
        Vec3 dir = d / len;
        QQuaternion q = QQuaternion::rotationTo(Vec3(0, 0, 1), dir);
        Vec3 up = q.rotatedVector(Vec3(0, 1, 0));
        Vec3 c = (a + b) * 0.5f + up * (h * 0.5f);
        Prim p = Prim::box(c, Vec3(halfW, h * 0.5f, len * 0.5f + 0.3f), q, round);
        p.smooth(0.5f).noise(noise).mat(mat);
        w.carve(p);
    }
}

// Altura do chao em (x,z): raio para baixo a partir de y0 (deve ser ar); fallback = y0.
inline float groundY(World& w, float x, float z, float y0, float up = 1.5f)
{
    RayHit h = w.raycast(Vec3(x, y0 + up, z), Vec3(0, -1, 0), up + 3.f, false);
    return h.hit ? h.pos.y() : y0;
}

// Estado compartilhado dos eventos/armadilhas (criado em props, usado no gameplay).
struct State {
    std::vector<std::pair<DynHandle, DynHandle>> braziers;   // (aceso, apagado)
    std::vector<Light*> embers;                               // luzes que se apagam na emboscada
    std::vector<DynHandle> eyes;                              // olhos verdes (seteiras, galeria)
    std::vector<DynHandle> archers;                           // arqueiros da galeria (ocultos ate o evento G)
    DynHandle bridge = nullptr;
    std::vector<Collider*> bridgeCols;
    DynHandle pitCover = nullptr; Collider* pitCol = nullptr;
    DynHandle iceSheet = nullptr; Collider* iceCol = nullptr;
    DynHandle hideoutRocks = nullptr; Collider* hideoutCol = nullptr;
    std::vector<DynHandle> pots, rocks;
    DynHandle oilPuddle = nullptr;
    // flags de execucao
    bool ambushed = false, bridgeCut = false, bridgeAuto = false, pitOpen = false, wireTripped = false, iceBroken = false, hideoutOpen = false;
};
State& st();             // estado unico
void resetState();

// Fases de construcao (cada uma em seu arquivo)
void buildGeometry(World& w);    // Layer1_Geometry.cpp
void buildProps(World& w);       // Layer1_Props.cpp
void buildGameplay(World& w);    // Layer1_Gameplay.cpp  (zonas, armadilhas, eventos, rota)
void buildPois(World& w);        // Layer1_Pois.cpp      (POIs e pontos de vista)

} // namespace L1
