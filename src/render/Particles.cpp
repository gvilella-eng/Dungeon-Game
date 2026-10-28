#include "Particles.h"
#include <cmath>

ParticleEmitter::ParticleEmitter(const EmitterDesc& desc) : d(desc) { rng = 2463534242u ^ (uint32_t)(d.pos.x() * 131.f) ^ (uint32_t)(d.pos.z() * 7919.f); }

float ParticleEmitter::rnd()
{
    rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
    return (rng & 0xFFFFFF) / 16777216.f;
}

void ParticleEmitter::applyPreset(EmitterDesc& d)
{
    switch (d.preset) {
    case ParticlePreset::Snow:
        d.halfExtent = Vec3(20, 0.5f, 20); d.rate = 260; d.lifeMin = 7; d.lifeMax = 11; d.sizeMin = 0.035f; d.sizeMax = 0.075f;
        d.velocity = Vec3(0.5f, -1.3f, 0.3f); d.velJitter = 0.25f; d.turbulence = 0.6f; d.gravity = Vec3(0, 0, 0);
        d.colorStart = QColor(235, 240, 255, 210); d.colorEnd = QColor(235, 240, 255, 160); d.additive = false; d.glow = false; break;
    case ParticlePreset::Embers:
        d.halfExtent = Vec3(0.4f, 0.1f, 0.4f); d.rate = 14; d.lifeMin = 1.4f; d.lifeMax = 3.2f; d.sizeMin = 0.025f; d.sizeMax = 0.055f;
        d.velocity = Vec3(0, 0.9f, 0); d.velJitter = 0.35f; d.turbulence = 0.9f; d.gravity = Vec3(0, 0.1f, 0);
        d.colorStart = QColor(255, 170, 60, 255); d.colorEnd = QColor(255, 40, 0, 0); d.additive = true; d.glow = true; break;
    case ParticlePreset::Spores:
        d.halfExtent = Vec3(3, 1.5f, 3); d.rate = 12; d.lifeMin = 6; d.lifeMax = 10; d.sizeMin = 0.04f; d.sizeMax = 0.09f;
        d.velocity = Vec3(0, 0.12f, 0); d.velJitter = 0.1f; d.turbulence = 0.25f; d.gravity = Vec3(0, 0, 0);
        d.colorStart = QColor(120, 255, 200, 160); d.colorEnd = QColor(80, 255, 220, 0); d.additive = true; d.glow = true; break;
    case ParticlePreset::Steam:
        d.halfExtent = Vec3(0.3f, 0.05f, 0.3f); d.rate = 24; d.lifeMin = 2.f; d.lifeMax = 3.5f; d.sizeMin = 0.45f; d.sizeMax = 0.9f;
        d.velocity = Vec3(0, 1.5f, 0); d.velJitter = 0.3f; d.turbulence = 0.5f; d.gravity = Vec3(0, 0.2f, 0);
        d.colorStart = QColor(225, 225, 230, 70); d.colorEnd = QColor(225, 225, 230, 0); d.additive = false; d.glow = false; break;
    case ParticlePreset::Dust:
        d.halfExtent = Vec3(4, 2, 4); d.rate = 8; d.lifeMin = 5; d.lifeMax = 9; d.sizeMin = 0.025f; d.sizeMax = 0.05f;
        d.velocity = Vec3(0, 0.03f, 0); d.velJitter = 0.06f; d.turbulence = 0.12f; d.gravity = Vec3(0, 0, 0);
        d.colorStart = QColor(210, 195, 170, 80); d.colorEnd = QColor(210, 195, 170, 0); d.additive = false; d.glow = false; break;
    case ParticlePreset::Sparks:
        d.halfExtent = Vec3(0.1f, 0.05f, 0.1f); d.rate = 30; d.lifeMin = 0.4f; d.lifeMax = 1.0f; d.sizeMin = 0.02f; d.sizeMax = 0.04f;
        d.velocity = Vec3(0, 2.8f, 0); d.velJitter = 1.6f; d.turbulence = 0.f; d.gravity = Vec3(0, -6.f, 0);
        d.colorStart = QColor(255, 230, 140, 255); d.colorEnd = QColor(255, 80, 10, 0); d.additive = true; d.glow = true; break;
    case ParticlePreset::VoidStream:
        d.halfExtent = Vec3(2.5f, 0.5f, 2.5f); d.rate = 50; d.lifeMin = 2.5f; d.lifeMax = 4.f; d.sizeMin = 0.08f; d.sizeMax = 0.18f;
        d.velocity = Vec3(0, 1.2f, 0); d.velJitter = 0.4f; d.turbulence = 0.4f; d.gravity = Vec3(0, 0, 0);
        d.colorStart = QColor(255, 120, 40, 230); d.colorEnd = QColor(170, 60, 255, 0); d.additive = true; d.glow = true; break;
    case ParticlePreset::Custom: break;
    }
}

void ParticleEmitter::update(float dt, const Vec3& cam, size_t& budget)
{
    if ((d.pos - cam).length() > d.activeRange) { parts.clear(); accum = 0; return; }
    // simula
    for (size_t i = 0; i < parts.size();) {
        Particle& p = parts[i];
        p.life -= dt;
        bool dead = p.life <= 0.f;
        if (d.preset == ParticlePreset::VoidStream) {
            Vec3 to = d.target - p.pos;
            float dist = to.length();
            if (dist < 0.5f) dead = true;
            else p.vel += to / dist * (2.5f + 6.f / (dist + 1.f)) * dt;
            p.vel *= (1.f - 0.8f * dt);
        }
        if (dead) { parts[i] = parts.back(); parts.pop_back(); continue; }
        if (d.turbulence > 0.f) {
            float t = p.life * 1.7f + (p.seed & 255);
            p.vel += Vec3(std::sin(t), std::sin(t * 1.3f + 2.f) * 0.5f, std::cos(t * 0.9f)) * d.turbulence * dt * 2.f;
        }
        p.vel += d.gravity * dt;
        p.pos += p.vel * dt;
        ++i;
    }
    if (!d.enabled) return;
    accum += d.rate * dt;
    int n = (int)accum; accum -= n;
    for (int k = 0; k < n && budget > 0; ++k, --budget) {
        Particle p;
        p.pos = d.pos + Vec3((rnd() * 2 - 1) * d.halfExtent.x(), (rnd() * 2 - 1) * d.halfExtent.y(), (rnd() * 2 - 1) * d.halfExtent.z());
        p.vel = d.velocity + Vec3(rnd() * 2 - 1, rnd() * 2 - 1, rnd() * 2 - 1) * d.velJitter;
        p.maxLife = p.life = d.lifeMin + (d.lifeMax - d.lifeMin) * rnd();
        p.size = d.sizeMin + (d.sizeMax - d.sizeMin) * rnd();
        p.seed = (uint32_t)(rnd() * 65535.f);
        parts.push_back(p);
    }
}
