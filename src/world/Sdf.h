// Rocha como campo de distancia com sinal (SDF). Convencao: d > 0 = AR, d < 0 = ROCHA.
// O mesh visual e a colisao usam exatamente esta mesma funcao.
//
// Composicao: air = smin(carve_i); rock = -air; rock = max(M, rock) (terreno exterior, so dentro da caixa);
// rock = smin(rock, fill_j).  (Fills sao aplicados DEPOIS do terreno para funcionar tambem no exterior.)
#pragma once
#include "Geom.h"
#include "render/Materials.h"
#include <functional>
#include <vector>
#include <QString>

enum class Shape { Sphere, Ellipsoid, Box, Capsule, Cylinder, Cone, Helix, Annulus };

struct HelixDesc {
    Vec3 center;               // eixo vertical em (center.x, center.z)
    float rInner = 19.f, rOuter = 22.f;
    float startY = 0.f;        // altura da SUPERFICIE de cima em t = 0
    float dropPerTurn = 32.f;  // queda por volta (m)
    float startAngleDeg = 0.f; // x = cos, z = sin
    float t0 = 0.f, t1 = 1.f;  // intervalo em voltas
    float thickness = 2.5f;    // espessura vertical (abaixo da superficie)
    float dir = 1.f;           // +1: angulo cresce com t
};

// Primitiva SDF. Use as fabricas estaticas e os modificadores encadeados.
struct Prim {
    Shape shape = Shape::Sphere;
    bool carve = true;         // true = ar (escava); false = rocha (preenche)
    float k = 0.f;             // raio de mistura suave (smooth min)
    float noiseAmp = 0.35f;    // amplitude do ruido (m)
    int material = MAT_LIMESTONE;
    int floorMode = 0;         // 0 nenhum, 1 absoluto (floorY), 2 relativo ao eixo (capsula)
    float floorY = 0.f;        // chao absoluto
    float floorFrac = 0.6f;    // chao relativo: eixo.y - raio*floorFrac
    QString tag;
    // geometria
    Vec3 a, b;
    float r0 = 1.f, r1 = 1.f;
    float y0 = 0.f, y1 = 0.f;
    float round = 0.f;
    Vec3 rotRows[3];           // caixa: matriz mundo->local (linhas)
    HelixDesc hx;
    Aabb bounds;               // AABB cru (calculado nas fabricas)

    // ---- fabricas ----
    static Prim sphere(Vec3 c, float r);
    static Prim ellipsoid(Vec3 c, Vec3 radii);
    static Prim box(Vec3 c, Vec3 half, QQuaternion rot = QQuaternion(), float round = 0.15f);
    static Prim capsule(Vec3 a, Vec3 b, float r0, float r1);        // tunel/round-cone
    static Prim cone(Vec3 apex, Vec3 base, float baseRadius, float tipRadius = 0.05f); // estalactites
    static Prim cylinder(Vec3 center, float radius, float yMin, float yMax);     // vertical (usa center.x/z)
    static Prim helix(const HelixDesc& d);
    static Prim annulus(Vec3 center, float rInner, float rOuter, float yMin, float yMax);

    // ---- modificadores ----
    Prim& smooth(float kk) { k = kk; return *this; }
    Prim& noise(float amp) { noiseAmp = amp; return *this; }
    Prim& mat(int m) { material = m; return *this; }
    Prim& floorAt(float y) { floorMode = 1; floorY = y; return *this; }
    Prim& floorRel(float frac) { floorMode = 2; floorFrac = frac; return *this; }
    Prim& named(const QString& t) { tag = t; return *this; }
    Prim& asFill() { carve = false; return *this; }
    Prim& asCarve() { carve = true; return *this; }
};

class SdfWorld {
public:
    struct Sample { float d; int mat; };
    using HeightFn = std::function<float(float, float)>;

    SdfWorld();
    // Limites da grade de aceleracao (tudo fora e rocha maciça). Chamar antes de adicionar prims.
    void setBounds(const Aabb& b, float cellSize = 12.f);
    Aabb bounds() const { return m_bounds; }

    int add(const Prim& p);                       // carve ou fill conforme p.carve
    void setTerrain(HeightFn h, const Aabb& exteriorBox);
    bool hasTerrain() const { return !m_terr.empty(); }
    bool overlapsExterior(const Aabb& b) const { for (auto& t : m_terr) if (t.box.overlaps(b)) return true; return false; }

    // d > 0 ar / d < 0 rocha (distancia aproximada). Thread-safe depois de montado.
    float distance(const Vec3& p) const { return eval(p).d; }
    Sample eval(const Vec3& p) const;
    Vec3 gradient(const Vec3& p, float eps = 0.12f) const;   // normalizado, aponta para o ar
    // Pode haver superficie dentro deste AABB? (conservador, por grade de aceleracao + terreno)
    bool maybeSurface(const Aabb& b) const;

    uint64_t hash() const;                        // chave de cache
    const std::vector<Prim>& prims() const { return m_prims; }
    size_t primCount() const { return m_prims.size(); }
    Aabb contentBounds() const { return m_content; }   // uniao dos AABBs das prims e do terreno

    static constexpr float kMargin = 3.0f;        // distancia minima ao ar em celulas vazias

private:
    float primDist(const Prim& pr, const Vec3& p, float noise, bool needNoise, int cellMode) const;
    float evalPrim(const Prim& pr, const Vec3& p, float* nz, bool* nzDone) const;
    inline int cellIndex(const Vec3& p) const;

    Aabb m_bounds, m_content;
    float m_cs = 12.f;
    int m_nx = 1, m_ny = 1, m_nz = 1;
    std::vector<Prim> m_prims;
    std::vector<std::vector<int>> m_carveCells, m_fillCells;
    struct Terrain { HeightFn h; Aabb box; };
    std::vector<Terrain> m_terr;
};
