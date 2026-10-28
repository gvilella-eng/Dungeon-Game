// Renderer: desenha o mundo (chunks SDF, props, dinamicos, transparentes, particulas, feixes, debug)
// em uma cena HDR e chama o pos-processamento.
#pragma once
#include "world/World.h"
#include "world/Mesher.h"
#include "PostFx.h"
#include "DebugDraw.h"
#include <memory>
#include <vector>

struct FrameParams {
    int w = 1280, h = 720;
    unsigned targetFbo = 0;
    QMatrix4x4 view, proj;
    Vec3 camPos, ambUp{0, 1, 0};
    float time = 0.f;
    bool fullbright = false;      // Luz do Mestre
    bool noFog = false;
    bool xray = false;            // casca translucida
    bool hideTerrain = false;
    bool wireframe = false;
    bool clipOn = false;
    QVector4D clip{0, 1, 0, 0};   // dot(p, n) > d => descartado
    bool backTint = false;        // face de tras tingida (vista geral / voo do Mestre)
    bool debugDraw = false;
    bool torchOn = false;
    Vec3 torchPos; Vec3 torchColor{1.f, 0.62f, 0.28f};
    float torchIntensity = 4.5f, torchRange = 18.f;
    ZoneState zone;
    float darkvision = 0.f;
    PostParams post;
    bool showLabels = false;
};

class Renderer {
public:
    bool init();
    void resize(int w, int h) { m_post.resize(w, h); }
    void addChunks(std::vector<ChunkMesh>& chunks);
    void clearChunks();
    void render(World& world, const FrameParams& fp);
    size_t chunkCount() const { return m_chunks.size(); }
    DebugDraw& debug() { return m_dbg; }
    int drawCalls = 0;
    size_t triangles = 0;
    bool ready() const { return m_ready; }
private:
    struct Chunk { GpuMesh mesh; bool exterior = false; };
    struct TItem { const GpuMesh* mesh; QMatrix4x4 model; float dist; bool xray; };
    void setupWorldShader(const FrameParams& fp, World& w);
    void drawMesh(const GpuMesh& m, const QMatrix4x4& model);
    void drawParticles(World& w, const FrameParams& fp);
    void drawBeams(World& w, const FrameParams& fp);
    void drawDebug(World& w, const FrameParams& fp);
    void makeNoise();

    bool m_ready = false;
    PostFx m_post;
    DebugDraw m_dbg;
    Shader m_world, m_particle, m_beam;
    unsigned m_noiseTex = 0;
    unsigned m_pVao = 0, m_pVbo = 0, m_bVao = 0, m_bVbo = 0;
    size_t m_pCap = 0, m_bCap = 0;
    std::vector<std::unique_ptr<Chunk>> m_chunks;
    Frustum m_frustum;
};
