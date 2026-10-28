// MeshBuilder: constroi malhas de props/criaturas na CPU, com pilha de transformacao.
// Cada primitiva recebe cor + id de material (Materials.h). Materiais transparentes
// vao automaticamente para a malha "transparent". Estilo: low-poly facetado.
#pragma once
#include "world/Geom.h"
#include <vector>
#include <cstdint>

#pragma pack(push, 1)
struct Vertex {                 // 32 bytes (formato unico: rocha SDF + props)
    float px, py, pz;
    float nx, ny, nz;
    uint8_t r, g, b, a;         // cor (a = opacidade nos materiais transparentes)
    uint8_t mat;                // MatId
    uint8_t ao;                 // oclusao ambiente (255 = sem oclusao)
    int8_t fx, fy;              // direcao de fluxo (liquidos), -127..127 (x,z)
};
#pragma pack(pop)
static_assert(sizeof(Vertex) == 32, "Vertex deve ter 32 bytes");

struct MeshData {
    std::vector<Vertex> v;
    std::vector<uint32_t> i;
    Aabb bounds;
    bool empty() const { return i.empty(); }
    void clear() { v.clear(); i.clear(); bounds = Aabb(); }
    void append(const MeshData& o);   // concatena
};

class MeshBuilder {
public:
    MeshBuilder();

    // ---- pilha de transformacao ----
    void push();
    void pop();
    void identity();
    void translate(const Vec3& t);
    void translate(float x, float y, float z) { translate(Vec3(x, y, z)); }
    void rotate(float degrees, const Vec3& axis);
    void rotate(const QQuaternion& q);
    void rotateY(float degrees) { rotate(degrees, Vec3(0, 1, 0)); }
    void scale(float s);
    void scale(const Vec3& s);
    void setMatrix(const QMatrix4x4& m) { m_m = m; }
    const QMatrix4x4& matrix() const { return m_m; }
    // Posiciona o resto do desenho em (pos, yaw em graus, escala uniforme).
    void place(const Vec3& pos, float yawDeg = 0.f, float s = 1.f);

    // ---- estado ----
    void setFlow(float dx, float dz);          // direcao de fluxo para liquidos (normalizada)
    void setAo(uint8_t ao) { m_ao = ao; }

    // ---- primitivas (coordenadas locais, transformadas pela pilha) ----
    void box(const Vec3& center, const Vec3& halfSize, const QColor& c, int mat);
    void boxMinMax(const Vec3& mn, const Vec3& mx, const QColor& c, int mat);
    // cilindro/cone de a para b (r0 em a, r1 em b); r=0 numa ponta gera cone.
    void cylinder(const Vec3& a, const Vec3& b, float r0, float r1, int segments, const QColor& c, int mat,
                  bool caps = true, bool smooth = true);
    void cylinder(const Vec3& a, const Vec3& b, float r, int segments, const QColor& c, int mat) { cylinder(a, b, r, r, segments, c, mat); }
    void cone(const Vec3& baseCenter, const Vec3& apex, float baseRadius, int segments, const QColor& c, int mat) { cylinder(baseCenter, apex, baseRadius, 0.f, segments, c, mat); }
    void sphere(const Vec3& center, float radius, const QColor& c, int mat, int segments = 10);
    void ellipsoid(const Vec3& center, const Vec3& radii, const QColor& c, int mat, int segments = 10);
    void capsule(const Vec3& a, const Vec3& b, float r, const QColor& c, int mat, int segments = 8);
    // toro: eixo = normal do plano do anel.
    void torus(const Vec3& center, const Vec3& axis, float majorR, float minorR, const QColor& c, int mat, int segMajor = 16, int segMinor = 6);
    void hexPrism(const Vec3& center, float radius, float height, const QColor& c, int mat);  // eixo Y
    void quad(const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const QColor& c, int mat);
    void triangle(const Vec3& p0, const Vec3& p1, const Vec3& p2, const QColor& c, int mat);
    // tubo ao longo de uma polilinha (cordas, costelas, canos); radii.size()==pts.size() ou 1.
    void tube(const std::vector<Vec3>& pts, const std::vector<float>& radii, int segments, const QColor& c, int mat, bool caps = true);
    void tube(const std::vector<Vec3>& pts, float radius, int segments, const QColor& c, int mat) { tube(pts, std::vector<float>{radius}, segments, c, mat); }

    // ---- helpers ----
    // pontos de uma catenaria aproximada de a ate b com flecha 'sag' (m) para baixo (-Y).
    static std::vector<Vec3> catenary(const Vec3& a, const Vec3& b, float sag, int n = 12);

    // ---- resultado ----
    const MeshData& opaque() const { return m_opaque; }
    const MeshData& transparent() const { return m_trans; }
    MeshData& opaque() { return m_opaque; }
    MeshData& transparent() { return m_trans; }
    bool empty() const { return m_opaque.empty() && m_trans.empty(); }
    Aabb bounds() const;
    void append(const MeshBuilder& o);   // junta (malhas ja transformadas)
    void clear();

private:
    uint32_t addV(const Vec3& p, const Vec3& n, const QColor& c, int mat);
    void tri(uint32_t a, uint32_t b, uint32_t c);
    void quadIdx(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
    void begin(int mat);

    QMatrix4x4 m_m;
    QMatrix4x4 m_nmat;
    std::vector<QMatrix4x4> m_stack;
    MeshData m_opaque, m_trans;
    MeshData* m_cur;
    uint8_t m_ao = 255;
    int8_t m_fx = 0, m_fy = 0;
    void updateNm();
};
