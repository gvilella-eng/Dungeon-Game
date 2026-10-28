#include "DebugDraw.h"

bool DebugDraw::init()
{
    bool ok = m_sh.load("line.vert", "line.frag");
    GL->glGenVertexArrays(1, &m_vao);
    GL->glGenBuffers(1, &m_vbo);
    GL->glBindVertexArray(m_vao);
    GL->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    GL->glEnableVertexAttribArray(0); GL->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), (void*)0);
    GL->glEnableVertexAttribArray(1); GL->glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(V), (void*)12);
    GL->glBindVertexArray(0);
    return ok;
}

void DebugDraw::line(const Vec3& a, const Vec3& b, const QColor& c)
{
    V va{{a.x(), a.y(), a.z()}, {(uint8_t)c.red(), (uint8_t)c.green(), (uint8_t)c.blue(), (uint8_t)c.alpha()}};
    V vb{{b.x(), b.y(), b.z()}, {(uint8_t)c.red(), (uint8_t)c.green(), (uint8_t)c.blue(), (uint8_t)c.alpha()}};
    m_v.push_back(va); m_v.push_back(vb);
}

void DebugDraw::box(const Aabb& b, const QColor& c)
{
    Vec3 p[8];
    for (int i = 0; i < 8; ++i) p[i] = Vec3((i & 1) ? b.mx.x() : b.mn.x(), (i & 2) ? b.mx.y() : b.mn.y(), (i & 4) ? b.mx.z() : b.mn.z());
    static const int e[12][2] = {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {0, 2}, {1, 3}, {4, 6}, {5, 7}, {0, 4}, {1, 5}, {2, 6}, {3, 7}};
    for (auto& k : e) line(p[k[0]], p[k[1]], c);
}

void DebugDraw::circleY(const Vec3& c, float r, const QColor& col, int seg)
{
    for (int i = 0; i < seg; ++i) {
        float a0 = i * 6.2831853f / seg, a1 = (i + 1) * 6.2831853f / seg;
        line(c + Vec3(std::cos(a0) * r, 0, std::sin(a0) * r), c + Vec3(std::cos(a1) * r, 0, std::sin(a1) * r), col);
    }
}

void DebugDraw::cylinderY(const Vec3& c, float r, float y0, float y1, const QColor& col)
{
    circleY(Vec3(c.x(), y0, c.z()), r, col); circleY(Vec3(c.x(), y1, c.z()), r, col);
    for (int i = 0; i < 4; ++i) {
        float a = i * 1.5707963f;
        line(Vec3(c.x() + std::cos(a) * r, y0, c.z() + std::sin(a) * r), Vec3(c.x() + std::cos(a) * r, y1, c.z() + std::sin(a) * r), col);
    }
}

void DebugDraw::sphere(const Vec3& c, float r, const QColor& col)
{
    circleY(c, r, col);
    for (int i = 0; i < 32; ++i) {
        float a0 = i * 6.2831853f / 32, a1 = (i + 1) * 6.2831853f / 32;
        line(c + Vec3(std::cos(a0) * r, std::sin(a0) * r, 0), c + Vec3(std::cos(a1) * r, std::sin(a1) * r, 0), col);
        line(c + Vec3(0, std::cos(a0) * r, std::sin(a0) * r), c + Vec3(0, std::cos(a1) * r, std::sin(a1) * r), col);
    }
}

void DebugDraw::cross(const Vec3& p, float s, const QColor& c)
{
    line(p - Vec3(s, 0, 0), p + Vec3(s, 0, 0), c); line(p - Vec3(0, s, 0), p + Vec3(0, s, 0), c); line(p - Vec3(0, 0, s), p + Vec3(0, 0, s), c);
}

void DebugDraw::flush(const QMatrix4x4& vp)
{
    if (m_v.empty()) return;
    GL->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    if (m_v.size() > m_cap) { m_cap = m_v.size() * 2; GL->glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(m_cap * sizeof(V)), nullptr, GL_DYNAMIC_DRAW); }
    GL->glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(m_v.size() * sizeof(V)), m_v.data());
    m_sh.bind(); m_sh.set("uVP", vp);
    GL->glDisable(GL_DEPTH_TEST);
    GL->glEnable(GL_BLEND); GL->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GL->glBindVertexArray(m_vao);
    GL->glDrawArrays(GL_LINES, 0, (GLsizei)m_v.size());
    GL->glBindVertexArray(0);
    GL->glDisable(GL_BLEND); GL->glEnable(GL_DEPTH_TEST);
    m_v.clear();
}
