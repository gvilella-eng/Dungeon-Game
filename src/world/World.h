// World: a API de conteudo. Camadas da masmorra e a Sala de Testes constroem tudo por aqui.
// Veja docs/ENGINE_API.md.
#pragma once
#include "Sdf.h"
#include "Gameplay.h"
#include "render/Particles.h"
#include "game/GameState.h"
#include <map>
#include <tuple>
#include <memory>
#include <vector>

class Player;

struct RayHit {
    bool hit = false;
    float t = 0.f;
    Vec3 pos, normal;
    QString collider;      // nome do colisor atingido ("" = rocha)
};

struct StaticBatch {
    MeshData opaque, transparent;
    Aabb bounds;
    std::unique_ptr<GpuMesh> gpuO, gpuT;
    bool dirty = true;
    StaticBatch();
    ~StaticBatch();
};

struct CameraInfo {
    Vec3 pos, fwd{0, 0, -1}, up{0, 1, 0};
    QMatrix4x4 view, proj;
    Frustum frustum;
    float fovY = 70.f, aspect = 16.f / 9.f;
    bool valid = false;
};

struct TunnelOpts { float floorFrac = 0.55f; int material = MAT_LIMESTONE; float noise = 0.4f; float k = 1.5f; QString tag; };

class World {
public:
    World();
    ~World();

    // ================= Geometria (SDF) =================
    SdfWorld sdf;
    void carve(const Prim& p);                    // escava ar
    void fill(const Prim& p);                     // adiciona rocha
    // Tunel por polilinha; radii.size()==1 (constante) ou == pts.size() (raio por ponto).
    void carveTunnel(const std::vector<Vec3>& pts, const std::vector<float>& radii, const TunnelOpts& o = TunnelOpts());
    void carveTunnel(const std::vector<Vec3>& pts, float radius, const TunnelOpts& o = TunnelOpts()) { carveTunnel(pts, std::vector<float>{radius}, o); }
    // Camara elipsoidal com chao plano em floorY.
    void carveChamber(const Vec3& center, const Vec3& radii, float floorY, int material = MAT_LIMESTONE, float noise = 0.5f, float k = 2.f, const QString& tag = QString());
    // Estalactites/estalagmites sao cones SDF (Fill) presos no teto/chao encontrados dentro da regiao.
    void stalactite(const Vec3& ceilingPoint, float length, float baseRadius, int material = MAT_LIMESTONE);
    void stalagmite(const Vec3& floorPoint, float length, float baseRadius, int material = MAT_LIMESTONE);
    int scatterStalactites(const Aabb& region, int count, int seed, float minLen = 1.5f, float maxLen = 5.f, float minR = 0.25f, float maxR = 0.8f, int material = MAT_LIMESTONE);
    int scatterStalagmites(const Aabb& region, int count, int seed, float minLen = 1.f, float maxLen = 3.5f, float minR = 0.3f, float maxR = 0.9f, int material = MAT_LIMESTONE);
    // Terreno exterior: altura h(x,z) valida apenas dentro de 'exteriorBox' (fora dela e rocha maciça).
    void setTerrain(SdfWorld::HeightFn h, const Aabb& exteriorBox) { sdf.setTerrain(std::move(h), exteriorBox); }

    // ================= Props e dinamicos =================
    void addStatic(MeshBuilder&& mb);             // props estaticos (buckets espaciais com culling)
    DynHandle addDynamic(const QString& name, MeshBuilder&& mb, const Vec3& pos = Vec3(), const QQuaternion& rot = QQuaternion(), const Vec3& scale = Vec3(1, 1, 1));
    DynHandle findDynamic(const QString& name) const;

    // ================= Colisores =================
    Collider* addBoxCollider(const QString& name, const Vec3& center, const Vec3& half, const QQuaternion& rot = QQuaternion(), bool openable = false);
    Collider* addCylinderCollider(const QString& name, const Vec3& center, float radius, float halfHeight, bool openable = false);   // eixo local Y
    Collider* addCapsuleCollider(const QString& name, const Vec3& center, float radius, float halfSegment, const QQuaternion& rot = QQuaternion());
    Collider* addSphereCollider(const QString& name, const Vec3& center, float radius);
    void attachCollider(Collider* c, DynHandle obj);   // center/rot passam a ser locais ao objeto
    Collider* findCollider(const QString& name) const;
    void refreshColliders();

    // ================= Luzes / particulas / feixes =================
    Light* addLight(const Vec3& pos, const Vec3& color, float intensity, float range, LightAnim anim = LightAnim::None, const QString& name = QString());
    EmitterHandle addEmitter(ParticlePreset preset, const Vec3& pos, const Vec3& halfExtent = Vec3(-1, -1, -1), float rate = -1.f, const QString& name = QString());
    EmitterHandle addEmitter(const EmitterDesc& desc);          // sem aplicar preset
    Beam* addBeam(const QString& name, const Vec3& a, const Vec3& b, const QColor& color = QColor(255, 220, 120), float width = 0.12f);

    // ================= Zonas, gatilhos, interativos =================
    Zone* addZone(const QString& name, const ZoneShape& shape, const ZoneEffects& fx, int priority = 0);
    Zone* findZone(const QString& name) const;
    void setDefaultZone(const ZoneEffects& fx) { m_defaultFx = fx; }
    Trigger* addTrigger(const QString& name, const ZoneShape& shape, std::function<void()> onEnter, std::function<void()> onExit = nullptr, bool once = false);
    Interactable* addInteractable(const QString& name, const Vec3& pos, float radius, const QString& prompt, std::function<void()> cb);

    // ================= Notas do Mestre, pontos de vista, rota, eventos =================
    Poi* addPoi(const Vec3& pos, PoiCategory cat, const QString& title, const QString& text, const QString& session = QString(), const QString& area = QString());
    void addViewpoint(const QString& name, const Vec3& pos, float yawDeg, float pitchDeg = 0.f, bool fly = false, const QString& group = QString());
    // Ponto de partida do jogo (posicao dos OLHOS). Se nao definido, usa o primeiro ponto de vista.
    void setSpawn(const Vec3& eye, float yawDeg, float pitchDeg = 0.f) { m_spawn.pos = eye; m_spawn.yawDeg = yawDeg; m_spawn.pitchDeg = pitchDeg; m_spawn.name = "Spawn"; m_hasSpawn = true; }
    bool hasSpawn() const { return m_hasSpawn; }
    const Viewpoint& spawn() const { return m_spawn; }
    void addRoutePoint(const QString& name, const Vec3& pos) { m_route.push_back({name, pos, m_routeBreak}); m_routeBreak = false; }
    // O proximo ponto de rota comeca um trecho independente (ex.: Sala de Testes).
    void addRouteBreak() { m_routeBreak = true; }
    void registerEvent(const QString& id, const QString& label, int hotkey, std::function<void()> fn);
    void addUpdater(std::function<void(float dt, float time)> fn) { m_updaters.push_back(std::move(fn)); }
    // Limites do overview ('O'): padrao = conteudo do SDF.
    void setOverview(const Vec3& center, float radius) { m_ovCenter = center; m_ovRadius = radius; m_ovSet = true; }

    // ================= Consultas =================
    float solidDistance(const Vec3& p, bool includeOpenable = true) const;   // min(rocha, colisores); >0 = livre
    Vec3 solidGradient(const Vec3& p, float eps = 0.05f, bool includeOpenable = true) const;
    RayHit raycast(const Vec3& origin, const Vec3& dir, float maxDist, bool includeOpenable = true) const;
    bool isVisibleToPlayer(const Vec3& p, float maxDist = 80.f) const;       // no frustum e sem oclusao
    bool lineOfSight(const Vec3& a, const Vec3& b) const;
    Vec3 playerPos() const;                                                   // pes
    Vec3 playerEye() const;
    Player* player() const { return m_player; }
    void teleportPlayer(const Vec3& feetPos, float yawDeg = 0.f, float pitchDeg = 0.f);
    const ZoneState& zoneState() const { return m_zs; }
    float time() const { return m_time; }
    ZoneState& zoneStateMut() { return m_zs; }

    // ================= Estado =================
    GameState state;
    CameraInfo cam;
    bool testLevelOnly = false;

    // ================= Acesso (motor) =================
    const std::vector<std::unique_ptr<Light>>& lights() const { return m_lights; }
    const std::vector<std::unique_ptr<DynamicObject>>& dynamics() const { return m_dyn; }
    const std::vector<std::unique_ptr<Collider>>& colliders() const { return m_colliders; }
    const std::vector<std::unique_ptr<ParticleEmitter>>& emitters() const { return m_emitters; }
    const std::vector<std::unique_ptr<Beam>>& beams() const { return m_beams; }
    const std::vector<std::unique_ptr<Zone>>& zones() const { return m_zones; }
    const std::vector<std::unique_ptr<Trigger>>& triggers() const { return m_triggers; }
    const std::vector<std::unique_ptr<Interactable>>& interactables() const { return m_inter; }
    const std::vector<std::unique_ptr<Poi>>& pois() const { return m_pois; }
    const std::vector<Viewpoint>& viewpoints() const { return m_views; }
    const std::vector<RoutePoint>& route() const { return m_route; }
    const std::vector<GameEvent>& events() const { return m_events; }
    std::map<std::tuple<int, int, int>, StaticBatch>& staticBatches() { return m_static; }
    QString areaAt(const Vec3& p) const;                  // nome da area (zona de maior prioridade com areaName)
    Vec3 overviewCenter() const;
    float overviewRadius() const;

    // ================= Atualizacao por quadro (GameWindow) =================
    void attachPlayer(Player* p) { m_player = p; }
    void update(float dt);                                // zonas, gatilhos, updaters, emissores, colisores
    void fireEvent(const QString& id);
    Interactable* focusedInteractable() const { return m_focusInter; }
    Poi* focusedPoi() const { return m_focusPoi; }
    void updateFocus();                                   // decide interativo/POI sob a mira

private:
    void updateZones(float dt);
    SdfWorld::HeightFn m_dummy;
    std::vector<std::unique_ptr<Light>> m_lights;
    std::vector<std::unique_ptr<DynamicObject>> m_dyn;
    std::vector<std::unique_ptr<Collider>> m_colliders;
    std::vector<std::unique_ptr<ParticleEmitter>> m_emitters;
    std::vector<std::unique_ptr<Beam>> m_beams;
    std::vector<std::unique_ptr<Zone>> m_zones;
    std::vector<std::unique_ptr<Trigger>> m_triggers;
    std::vector<std::unique_ptr<Interactable>> m_inter;
    std::vector<std::unique_ptr<Poi>> m_pois;
    std::vector<Viewpoint> m_views;
    std::vector<RoutePoint> m_route;
    bool m_routeBreak = false;
    std::vector<GameEvent> m_events;
    std::vector<std::function<void(float, float)>> m_updaters;
    std::map<std::tuple<int, int, int>, StaticBatch> m_static;
    ZoneEffects m_defaultFx;
    ZoneState m_zs;
    Player* m_player = nullptr;
    float m_time = 0.f;
    Interactable* m_focusInter = nullptr;
    Poi* m_focusPoi = nullptr;
    Vec3 m_ovCenter; float m_ovRadius = 0.f; bool m_ovSet = false;
    bool m_zoneInit = false;
    QString m_lastArea;
    uint32_t m_scatterRng = 1;
    int m_nextPoi = 1;
    float m_gustPrev = 0.f;
    Viewpoint m_spawn; bool m_hasSpawn = false;
};
