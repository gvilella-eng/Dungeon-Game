// Pos-processamento: cena HDR RGBA16F com MSAA 4x -> resolve -> bloom -> ACES + efeitos de gameplay.
#pragma once
#include "Shader.h"
#include "GpuMesh.h"

struct PostParams {
    float time = 0.f;
    float exposure = 1.0f;
    float bloom = 0.28f;
    float darkvision = 0.f;
    float hallucination = 0.f;
    float psychic = 0.f;
    float damageFlash = 0.f;
    QColor flashColor{200, 20, 20};
    QColor fadeColor{0, 0, 0};
    float fadeAlpha = 0.f;
    float underwater = 0.f;
    float vignette = 0.85f;
    float grain = 0.03f;
};

class PostFx {
public:
    bool init();
    void resize(int w, int h);
    void bindScene();                                   // desenha a cena aqui
    void finish(unsigned targetFbo, const PostParams& p);
    int width() const { return m_w; }
    int height() const { return m_h; }
    unsigned sceneDepthFbo() const { return m_msaaFbo; }
private:
    void destroy();
    int m_w = 0, m_h = 0;
    unsigned m_msaaFbo = 0, m_msaaColor = 0, m_msaaDepth = 0;
    unsigned m_resFbo = 0, m_resTex = 0;
    static constexpr int kLevels = 5;
    unsigned m_bloomTex[kLevels] = {}, m_bloomFbo[kLevels] = {};
    int m_bw[kLevels] = {}, m_bh[kLevels] = {};
    unsigned m_vao = 0;
    Shader m_down, m_up, m_comp;
};
