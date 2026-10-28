#include "PostFx.h"
#include <QDebug>

bool PostFx::init()
{
    bool ok = m_down.load("fullscreen.vert", "bloom_down.frag");
    ok &= m_up.load("fullscreen.vert", "bloom_up.frag");
    ok &= m_comp.load("fullscreen.vert", "composite.frag");
    GL->glGenVertexArrays(1, &m_vao);
    return ok;
}

void PostFx::destroy()
{
    if (m_msaaFbo) { GL->glDeleteFramebuffers(1, &m_msaaFbo); GL->glDeleteRenderbuffers(1, &m_msaaColor); GL->glDeleteRenderbuffers(1, &m_msaaDepth); }
    if (m_resFbo) { GL->glDeleteFramebuffers(1, &m_resFbo); GL->glDeleteTextures(1, &m_resTex); }
    for (int i = 0; i < kLevels; ++i) if (m_bloomFbo[i]) { GL->glDeleteFramebuffers(1, &m_bloomFbo[i]); GL->glDeleteTextures(1, &m_bloomTex[i]); }
    m_msaaFbo = m_resFbo = 0;
    for (int i = 0; i < kLevels; ++i) m_bloomFbo[i] = 0;
}

static unsigned makeTex(int w, int h)
{
    unsigned t;
    GL->glGenTextures(1, &t);
    GL->glBindTexture(GL_TEXTURE_2D, t);
    GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return t;
}

void PostFx::resize(int w, int h)
{
    if (w == m_w && h == m_h && m_msaaFbo) return;
    destroy();
    m_w = w; m_h = h;
    GL->glGenRenderbuffers(1, &m_msaaColor);
    GL->glBindRenderbuffer(GL_RENDERBUFFER, m_msaaColor);
    GL->glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA16F, w, h);
    GL->glGenRenderbuffers(1, &m_msaaDepth);
    GL->glBindRenderbuffer(GL_RENDERBUFFER, m_msaaDepth);
    GL->glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, w, h);
    GL->glGenFramebuffers(1, &m_msaaFbo);
    GL->glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);
    GL->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_msaaColor);
    GL->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_msaaDepth);
    if (GL->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) qWarning() << "FBO MSAA incompleto";

    m_resTex = makeTex(w, h);
    GL->glGenFramebuffers(1, &m_resFbo);
    GL->glBindFramebuffer(GL_FRAMEBUFFER, m_resFbo);
    GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resTex, 0);
    int bw = w, bh = h;
    for (int i = 0; i < kLevels; ++i) {
        bw = std::max(1, bw / 2); bh = std::max(1, bh / 2);
        m_bw[i] = bw; m_bh[i] = bh;
        m_bloomTex[i] = makeTex(bw, bh);
        GL->glGenFramebuffers(1, &m_bloomFbo[i]);
        GL->glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFbo[i]);
        GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bloomTex[i], 0);
    }
    GL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostFx::bindScene()
{
    GL->glBindFramebuffer(GL_FRAMEBUFFER, m_msaaFbo);
    GL->glViewport(0, 0, m_w, m_h);
}

void PostFx::finish(unsigned target, const PostParams& p)
{
    // resolve MSAA
    GL->glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaaFbo);
    GL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_resFbo);
    GL->glBlitFramebuffer(0, 0, m_w, m_h, 0, 0, m_w, m_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    GL->glDisable(GL_DEPTH_TEST); GL->glDisable(GL_BLEND); GL->glDisable(GL_CULL_FACE);
    GL->glBindVertexArray(m_vao);
    GL->glActiveTexture(GL_TEXTURE0);
    // descida
    m_down.bind();
    unsigned srcTex = m_resTex; int sw = m_w, sh = m_h;
    for (int i = 0; i < kLevels; ++i) {
        GL->glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFbo[i]);
        GL->glViewport(0, 0, m_bw[i], m_bh[i]);
        GL->glBindTexture(GL_TEXTURE_2D, srcTex);
        m_down.set("uTex", 0); m_down.set("uTexel", QVector2D(1.f / sw, 1.f / sh)); m_down.set("uFirst", i == 0 ? 1 : 0);
        GL->glDrawArrays(GL_TRIANGLES, 0, 3);
        srcTex = m_bloomTex[i]; sw = m_bw[i]; sh = m_bh[i];
    }
    // subida aditiva
    m_up.bind();
    GL->glEnable(GL_BLEND); GL->glBlendFunc(GL_ONE, GL_ONE);
    for (int i = kLevels - 2; i >= 0; --i) {
        GL->glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFbo[i]);
        GL->glViewport(0, 0, m_bw[i], m_bh[i]);
        GL->glBindTexture(GL_TEXTURE_2D, m_bloomTex[i + 1]);
        m_up.set("uTex", 0); m_up.set("uTexel", QVector2D(1.f / m_bw[i + 1], 1.f / m_bh[i + 1])); m_up.set("uRadius", 1.0f);
        GL->glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    GL->glDisable(GL_BLEND);
    // composicao
    GL->glBindFramebuffer(GL_FRAMEBUFFER, target);
    GL->glViewport(0, 0, m_w, m_h);
    m_comp.bind();
    GL->glActiveTexture(GL_TEXTURE0); GL->glBindTexture(GL_TEXTURE_2D, m_resTex);
    GL->glActiveTexture(GL_TEXTURE1); GL->glBindTexture(GL_TEXTURE_2D, m_bloomTex[0]);
    m_comp.set("uScene", 0); m_comp.set("uBloom", 1);
    m_comp.set("uTime", p.time); m_comp.set("uExposure", p.exposure); m_comp.set("uBloomStr", p.bloom);
    m_comp.set("uDark", p.darkvision); m_comp.set("uHallu", p.hallucination); m_comp.set("uPsychic", p.psychic);
    m_comp.set("uFlash", p.damageFlash);
    m_comp.set("uFlashColor", QVector3D(p.flashColor.redF(), p.flashColor.greenF(), p.flashColor.blueF()));
    m_comp.set("uFade", QVector4D(p.fadeColor.redF(), p.fadeColor.greenF(), p.fadeColor.blueF(), p.fadeAlpha));
    m_comp.set("uUnder", p.underwater); m_comp.set("uVignette", p.vignette); m_comp.set("uGrain", p.grain);
    GL->glDrawArrays(GL_TRIANGLES, 0, 3);
    GL->glActiveTexture(GL_TEXTURE0);
    GL->glBindVertexArray(0);
}
