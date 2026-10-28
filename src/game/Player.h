// Jogador: capsula (r 0.3, 1.75 m), colisao contra rocha SDF + colisores, nado, escalada,
// gravidade arbitraria por zona (a camera rola suavemente para -gravidade) e voo livre (noclip).
#pragma once
#include "world/Geom.h"

class World;

class Player {
public:
    struct Input {
        float forward = 0.f, strafe = 0.f;   // -1..1
        bool jump = false;                    // borda de subida (pulo) / segurando (nado/voo: sobe)
        bool jumpHeld = false;
        bool crouch = false;                  // Ctrl (agacha; desce no nado/voo)
        bool sprint = false;
    };
    enum class Mode { Walk, Fly };

    explicit Player(World& w);

    // ---- estado publico ----
    Mode mode = Mode::Walk;
    Vec3 pos;                // pes
    Vec3 vel;
    Vec3 up{0, 1, 0};        // "cima" suavizado (= -gravidade)
    Vec3 fwd{0, 0, -1};      // direcao do olhar
    float height = 1.75f;    // altura atual da capsula (agachar)
    bool crouching = false, grounded = false, swimming = false, climbing = false, eyeUnderwater = false;
    bool sprinting = false;
    float flySpeedScale = 1.f;
    float walkSpeed = 3.0f, sprintSpeed = 5.5f, crouchSpeed = 1.4f, flyBase = 8.f;
    Vec3 lastSafe;

    // ---- controle ----
    void update(float dt, const Input& in);
    void look(float dxPixels, float dyPixels);
    void teleport(const Vec3& feet, float yawDeg, float pitchDeg);
    void setMode(Mode m);
    void toggleFly() { setMode(mode == Mode::Fly ? Mode::Walk : Mode::Fly); }
    void snapToGround(float maxDist = 4.f);          // desce ate o chao (spawn/teleporte)

    // ---- consultas ----
    Vec3 eyePos() const;           // com bob
    Vec3 bodyCenter() const { return pos + up * (height * 0.5f); }
    Vec3 right() const;
    float yawDeg() const;          // 0 = -Z, positivo = esquerda (oeste)
    float pitchDeg() const;
    float speed() const { return vel.length(); }
    QMatrix4x4 viewMatrix() const; // inclui tremor de tela

    static constexpr float kRadius = 0.3f, kStand = 1.75f, kCrouch = 0.95f;

private:
    void updateWalk(float dt, const Input& in);
    void updateFly(float dt, const Input& in);
    void rotateUp(const Vec3& target, float dt);
    bool resolve();                      // empurra para fora da geometria; retorna se houve contato
    bool blockedAt(const Vec3& p, float h) const;
    bool probeGround(Vec3* normal = nullptr) const;
    void stepMove(const Vec3& move, bool wantStep);
    void handleHazards(float dt);
    void handleBreath(float dt);

    World& w;
    bool m_airborne = false;
    float m_fallTop = 0.f;
    float m_bobPhase = 0.f, m_bobAmp = 0.f;
    float m_safeTimer = 0.f;
    float m_hazardAccum = 0.f, m_respawnTimer = -1.f;
    float m_drownTimer = 0.f;
    bool m_contactGround = false;
    Vec3 m_prevGravity{0, -9.81f, 0};
    float m_jumpCooldown = 0.f;
    float m_coyote = 0.f;
};
