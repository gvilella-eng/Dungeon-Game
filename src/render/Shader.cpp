#include "Shader.h"
#include <QFile>
#include <QDebug>

bool Shader::load(const QString& vertRes, const QString& fragRes)
{
    m_p = std::make_unique<QOpenGLShaderProgram>();
    auto readFile = [](const QString& r) {
        QFile f(":/shaders/" + r);
        if (!f.open(QIODevice::ReadOnly)) { qWarning().noquote() << "Shader nao encontrado:" << f.fileName(); return QByteArray(); }
        return f.readAll();
    };
    if (!m_p->addShaderFromSourceCode(QOpenGLShader::Vertex, readFile(vertRes))) { qWarning().noquote() << "Shader" << vertRes << m_p->log(); return false; }
    if (!m_p->addShaderFromSourceCode(QOpenGLShader::Fragment, readFile(fragRes))) { qWarning().noquote() << "Shader" << fragRes << m_p->log(); return false; }
    if (!m_p->link()) { qWarning().noquote() << "Link" << vertRes << fragRes << m_p->log(); return false; }
    return true;
}
