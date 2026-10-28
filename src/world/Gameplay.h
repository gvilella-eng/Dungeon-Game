// Estruturas de gameplay do mundo: luzes, objetos dinamicos, colisores, zonas, gatilhos,
// interativos, POIs (notas do Mestre), pontos de vista, eventos.
#pragma once
#include "Geom.h"
#include "render/MeshBuilder.h"
#include <functional>
#include <memory>
#include <optional>
#include <QString>

class GpuMesh;

// ---------------------------------------------------------------- Luzes
enum class LightAnim { None, TorchFlicker, Pulse, Custom };
struct Light {
    QString name;
    Vec3 pos;
    Vec3 color{1.f, 0.7f, 0.4f};      // RGB linear
    float intensity = 6.f;            // multiplicador; tocha do jogador = 5.5 (forte ate ~9 m)
    float range = 12.f;               // alcance (m); atenuacao suave ate 0 nesse raio
    bool enabled = true;
    LightAnim anim = LightAnim::None;
    float animFreq = 1.f;             // Hz (Pulse)
    float animDepth = 0.3f;           // 0..1 (profundidade da variacao)
    float phase = 0.f;
    std::function<float(float)> custom; // Custom: recebe o tempo, devolve multiplicador de intensidade
    // intensidade efetiva no tempo t
    float intensityAt(float t) const;
};

// ---------------------------------------------------------------- Objetos dinamicos
struct DynamicObject {
    QString name;
    MeshData opaque, transparent;      // CPU (liberada apos o upload)
    std::unique_ptr<GpuMesh> gpuO, gpuT;
    bool dirty = true;
    Aabb localBounds;
    Vec3 pos;
    QQuaternion rot;
    Vec3 scl{1, 1, 1};
    bool visible = true;

    QMatrix4x4 matrix() const {
        QMatrix4x4 m; m.translate(pos); m.rotate(rot); m.scale(scl); return m;
    }
    void setTransform(const Vec3& p, const QQuaternion& q, const Vec3& s = Vec3(1, 1, 1)) { pos = p; rot = q; scl = s; }
    void setPosition(const Vec3& p) { pos = p; }
    void setRotation(const QQuaternion& q) { rot = q; }
    void setYaw(float deg) { rot = QQuaternion::fromAxisAndAngle(0, 1, 0, deg); }
    void setVisible(bool v) { visible = v; }
    ~DynamicObject();
    DynamicObject();
};
using DynHandle = DynamicObject*;

// ---------------------------------------------------------------- Colisores
struct Collider {
    enum Type { Box, Cylinder, Capsule, Sphere };
    QString name;
    Type type = Box;
    Vec3 center;                       // posicao local (ou mundo, se estatico)
    QQuaternion rot;                   // rotacao local (ou mundo)
    Vec3 half{1, 1, 1};                // Box: meias dimensoes
    float radius = 1.f;                // Cylinder/Capsule/Sphere
    float halfHeight = 1.f;            // Cylinder: meia altura; Capsule: meio segmento (eixo local Y)
    bool enabled = true;
    bool openable = false;             // portas/muros de gelo: ignorados pelo --validate
    DynamicObject* follow = nullptr;   // se definido, segue a transformacao do objeto
    // cache (atualizado por World::refreshColliders)
    QMatrix4x4 inv;
    Aabb bounds;
    void refresh();
    float distance(const Vec3& p) const;   // SDF (positivo fora)
};

// ---------------------------------------------------------------- Zonas
struct ZoneShape {
    enum Type { BoxShape, SphereShape, CylinderShape };
    Type type = BoxShape;
    Aabb box;
    Vec3 center; float radius = 1.f; float y0 = 0.f, y1 = 0.f;
    static ZoneShape makeBox(const Vec3& mn, const Vec3& mx) { ZoneShape s; s.type = BoxShape; s.box = Aabb(mn, mx); return s; }
    static ZoneShape makeBoxCenter(const Vec3& c, const Vec3& half) { return makeBox(c - half, c + half); }
    static ZoneShape makeSphere(const Vec3& c, float r) { ZoneShape s; s.type = SphereShape; s.center = c; s.radius = r; return s; }
    static ZoneShape makeCylinder(const Vec3& c, float r, float yMin, float yMax) { ZoneShape s; s.type = CylinderShape; s.center = c; s.radius = r; s.y0 = yMin; s.y1 = yMax; return s; }
    bool contains(const Vec3& p) const;
    Aabb bounds() const;
    float topY() const;
};

struct ZoneEffects {
    QString areaName, subtitle;                 // banner ao entrar (ex.: "Camada 1 · Sessao 1 · Nivel 6")
    std::optional<float> temperature;           // graus C
    std::optional<Vec3> fogColor;
    std::optional<float> fogDensity;            // exponencial (0.02 = visibilidade ~60 m)
    std::optional<Vec3> ambientColor;           // ceu (cima)
    std::optional<Vec3> ambientGround;          // chao (baixo); padrao = ambientColor * 0.5
    std::optional<float> ambientStrength;       // multiplicador
    std::optional<Vec3> gravity;                // m/s^2
    std::optional<float> torchScale;            // 1 = normal; <1 = ar rarefeito (chama encolhe)
    std::optional<float> hallucination;         // 0..1
    std::optional<float> psychic;               // 0..1
    bool water = false;                         // nadar
    float waterSurfaceY = std::numeric_limits<float>::quiet_NaN(); // NaN = topo da zona
    bool climb = false;                         // escalar
    bool wind = false;                          // rajadas periodicas
    float windPeriod = 20.f, windDuration = 2.f;
    bool windExtinguishes = true;
    bool hazard = false;                        // dano + respawn
    QString hazardText;                         // ex.: "Lava: 10d10 de dano de fogo"
    float hazardDelay = 0.f;                    // segundos ate o dano (0 = imediato)
};

struct Zone {
    QString name;
    ZoneShape shape;
    int priority = 0;                           // maior vence por propriedade
    bool enabled = true;
    ZoneEffects fx;
};

// Estado de zona misturado suavemente (o que o resto do jogo consulta).
struct ZoneState {
    QString areaName, subtitle;
    float temperature = 10.f;
    Vec3 fogColor{0.02f, 0.02f, 0.03f};
    float fogDensity = 0.03f;
    Vec3 ambientColor{0.012f, 0.012f, 0.016f};
    Vec3 ambientGround{0.006f, 0.006f, 0.008f};
    float ambientStrength = 1.f;
    Vec3 gravity{0, -9.81f, 0};
    float torchScale = 1.f;
    float hallucination = 0.f, psychic = 0.f;
    bool water = false; float waterSurfaceY = 0.f;
    bool climb = false;
    bool windGust = false;                      // true durante a rajada
    bool windExtinguishes = true;
    bool inWindZone = false;
    bool hazard = false; QString hazardText; float hazardDelay = 0.f;
};

// ---------------------------------------------------------------- Gatilhos / interativos
struct Trigger {
    QString name;
    ZoneShape shape;
    std::function<void()> onEnter, onExit;
    std::function<void(float)> onStay;
    bool once = false;
    bool enabled = true;
    bool fired = false;
    bool inside = false;
};

struct Interactable {
    QString name;
    Vec3 pos;
    float radius = 1.0f;                        // raio de selecao ao olhar (m)
    QString prompt;                             // "[E] <prompt>"
    std::function<QString()> promptFn;          // opcional: prompt dinamico
    std::function<void()> callback;
    bool enabled = true;
};

// ---------------------------------------------------------------- POIs (notas do Mestre)
enum class PoiCategory { Area, Trap, Puzzle, Secret, Creature, Treasure, Lore, Rest };
QString poiCategoryName(PoiCategory c);           // pt-BR
QColor poiCategoryColor(PoiCategory c);

struct Poi {
    int id = 0;
    Vec3 pos;
    PoiCategory category = PoiCategory::Area;
    QString title;
    QString text;                                // multi-linha
    QString session;                             // opcional: "Sessao 1 · Nivel 6"
    QString area;                                // opcional (senao deriva da zona)
};

struct Viewpoint {
    QString name;
    Vec3 pos;
    float yawDeg = 0.f;                          // 0 = olhando para -Z (norte); positivo = gira para a esquerda (oeste)
    float pitchDeg = 0.f;
    bool fly = false;                            // true: modo voo livre (noclip) ao teleportar
    QString group;                               // agrupamento no menu (opcional)
};

struct RoutePoint { QString name; Vec3 pos; bool chainStart = false; };   // chainStart: inicia um trecho novo (nao liga ao anterior)

struct GameEvent {
    QString id, label;
    int hotkey = 0;                              // Qt::Key (0 = sem atalho)
    std::function<void()> fn;
};

struct Beam {
    QString name;
    Vec3 a, b;
    QColor color{255, 220, 120};
    float width = 0.12f;
    float intensity = 1.5f;
    bool visible = true;
};
