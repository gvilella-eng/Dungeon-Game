#include "GpuMesh.h"
#include <QOpenGLContext>

QOpenGLFunctions_3_3_Core* GL = nullptr;

GpuMesh::~GpuMesh() { release(); }

void GpuMesh::release()
{
    if (!GL || !QOpenGLContext::currentContext()) { m_vao = m_vbo = m_ibo = 0; return; }
    if (m_ibo) GL->glDeleteBuffers(1, &m_ibo);
    if (m_vbo) GL->glDeleteBuffers(1, &m_vbo);
    if (m_vao) GL->glDeleteVertexArrays(1, &m_vao);
    m_vao = m_vbo = m_ibo = 0; m_count = 0;
}

void GpuMesh::upload(const MeshData& m)
{
    release();
    bounds = m.bounds;
    if (m.i.empty()) return;
    GL->glGenVertexArrays(1, &m_vao);
    GL->glBindVertexArray(m_vao);
    GL->glGenBuffers(1, &m_vbo);
    GL->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    GL->glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(m.v.size() * sizeof(Vertex)), m.v.data(), GL_STATIC_DRAW);
    GL->glGenBuffers(1, &m_ibo);
    GL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    GL->glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(m.i.size() * 4), m.i.data(), GL_STATIC_DRAW);
    GL->glEnableVertexAttribArray(0); GL->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    GL->glEnableVertexAttribArray(1); GL->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)12);
    GL->glEnableVertexAttribArray(2); GL->glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)24);
    GL->glEnableVertexAttribArray(3); GL->glVertexAttribPointer(3, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), (void*)28);
    GL->glEnableVertexAttribArray(4); GL->glVertexAttribPointer(4, 2, GL_BYTE, GL_TRUE, sizeof(Vertex), (void*)30);
    GL->glBindVertexArray(0);
    m_count = (int)m.i.size();
}

void GpuMesh::draw() const
{
    if (!m_vao) return;
    GL->glBindVertexArray(m_vao);
    GL->glDrawElements(GL_TRIANGLES, m_count, GL_UNSIGNED_INT, nullptr);
}
