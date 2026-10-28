// Wrapper fino sobre QOpenGLShaderProgram com cache de uniform locations.
#pragma once
#include "world/Geom.h"
#include <QHash>
#include <QOpenGLShaderProgram>
#include <memory>

class Shader {
public:
    bool load(const QString& vertRes, const QString& fragRes);
    void bind() { m_p->bind(); }
    void release() { m_p->release(); }
    int loc(const char* name) {
        auto it = m_loc.find(name);
        if (it != m_loc.end()) return it.value();
        int l = m_p->uniformLocation(name);
        m_loc.insert(name, l);
        return l;
    }
    void set(const char* n, int v) { m_p->setUniformValue(loc(n), v); }
    void set(const char* n, float v) { m_p->setUniformValue(loc(n), v); }
    void set(const char* n, const QVector2D& v) { m_p->setUniformValue(loc(n), v); }
    void set(const char* n, const QVector3D& v) { m_p->setUniformValue(loc(n), v); }
    void set(const char* n, const QVector4D& v) { m_p->setUniformValue(loc(n), v); }
    void set(const char* n, const QMatrix4x4& v) { m_p->setUniformValue(loc(n), v); }
    void set(const char* n, const QMatrix3x3& v) { m_p->setUniformValue(loc(n), v); }
    void setArray(const char* n, const QVector4D* v, int count) { m_p->setUniformValueArray(loc(n), v, count); }
private:
    std::unique_ptr<QOpenGLShaderProgram> m_p;
    QHash<QByteArray, int> m_loc;
};
