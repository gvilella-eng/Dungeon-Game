// Malha na GPU (VAO/VBO/IBO) no formato unico de Vertex.
#pragma once
#include "MeshBuilder.h"
#include <QOpenGLFunctions_3_3_Core>

extern QOpenGLFunctions_3_3_Core* GL;   // definido em GpuMesh.cpp; atribuido em GameWindow::initializeGL

class GpuMesh {
public:
    GpuMesh() = default;
    ~GpuMesh();
    GpuMesh(const GpuMesh&) = delete;
    GpuMesh& operator=(const GpuMesh&) = delete;
    void upload(const MeshData& m);
    void draw() const;
    void release();
    bool valid() const { return m_vao != 0 && m_count > 0; }
    Aabb bounds;
    int indexCount() const { return m_count; }
private:
    unsigned m_vao = 0, m_vbo = 0, m_ibo = 0;
    int m_count = 0;
};
