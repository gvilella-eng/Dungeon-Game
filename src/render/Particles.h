// Particulas CPU (billboards redondos suaves). Teto global ~20k.
#pragma once
#include "world/Geom.h"
#include <memory>
#include <vector>
#include <QString>

enum class ParticlePreset { Snow, Embers, Spores, Steam, Dust, Sparks, VoidStream, Custom };

struct EmitterDesc {
    QString name;
    ParticlePreset preset = ParticlePreset::Dust;
    Vec3 pos;                       // centro
    Vec3 halfExtent{0.5f, 0.1f, 0.5f}; // caixa de nascimento
    float rate = 20.f;              // particulas/s
    float lifeMin = 2.f, lifeMax = 4.f;
    float sizeMin = 0.05f, sizeMax = 0.1f;
    Vec3 velocity{0, 0.5f, 0};      // velocidade media
    float velJitter = 0.3f;         // variacao aleatoria
    float turbulence = 0.3f;        // balanço
    Vec3 gravity{0, 0, 0};          // aceleracao
    QColor colorStart{255, 255, 255, 200}, colorEnd{255, 255, 255, 0};
    bool additive = false;
    bool glow = false;              // brilha (multiplica cor)
    Vec3 target;                    // VoidStream: ponto de atracao
    float activeRange = 90.f;       // so simula perto da camera
    bool followPlayer = false;      // pos = jogador + followOffset (neve/poeira ao redor)
    Vec3 followOffset{0, 8, 0};
    bool enabled = true;
};

struct Particle { Vec3 pos, vel; float life, maxLife, size; uint32_t seed; };

class ParticleEmitter {
public:
    EmitterDesc d;
    std::vector<Particle> parts;
    float accum = 0.f;
    uint32_t rng = 12345;
    ParticleEmitter(const EmitterDesc& desc);
    void setEnabled(bool e) { d.enabled = e; }
    void setPosition(const Vec3& p) { d.pos = p; }
    void update(float dt, const Vec3& camPos, size_t& budget);
    // Aplica valores padrao do preset (chamado pelo World::addEmitter).
    static void applyPreset(EmitterDesc& d);
private:
    float rnd();
};
using EmitterHandle = ParticleEmitter*;
