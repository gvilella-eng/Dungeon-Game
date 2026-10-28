// Linhas de depuracao (zonas, gatilhos, colisores, feixes, caminhos).
#pragma once
#include "Shader.h"
#include "GpuMesh.h"
#include <vector>

class DebugDraw {
public:
    bool init();
    void clear() { m_v.clear(); }
    void line(const Vec3& a, const Vec3& b, const QColor& c);
    void box(const Aabb& b, const QColor& c);
    void circleY(const Vec3& center, float r, const QColor& c, int seg = 32);
    void cylinderY(const Vec3& center, float r, float y0, float y1, const QColor& c);
    void sphere(const Vec3& center, float r, const QColor& c);
    void cross(const Vec3& p, float s, const QColor& c);
    void flush(const QMatrix4x4& vp);   // desenha sem teste de profundidade
private:
    struct V { float p[3]; uint8_t c[4]; };
    std::vector<V> m_v;
    Shader m_sh;
    unsigned m_vao = 0, m_vbo = 0;
    size_t m_cap = 0;
};
