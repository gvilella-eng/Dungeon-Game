#include "Player.h"
#include "world/World.h"
#include <QRandomGenerator>

static const float kCos50 = 0.6428f;

Player::Player(World& world) : w(world) { lastSafe = pos; }

Vec3 Player::right() const
{
    Vec3 r = Vec3::crossProduct(fwd, up);
    if (r.lengthSquared() < 1e-6f) r = Vec3(1, 0, 0);
    return r.normalized();
}

float Player::yawDeg() const
{
    Vec3 f = fwd - up * Vec3::dotProduct(fwd, up);
    // referencia: -Z projetado no plano; usa Y global quando up ~ Y
    return std::atan2(-f.x(), -f.z()) * 57.2957795f;
}
float Player::pitchDeg() const { return std::asin(clampf(Vec3::dotProduct(fwd, up), -1.f, 1.f)) * 57.2957795f; }

Vec3 Player::eyePos() const
{
    float eyeH = mode == Mode::Fly ? 1.65f : height - 0.10f;
    Vec3 e = pos + up * eyeH;
    if (mode == Mode::Walk && m_bobAmp > 0.f)
        e += up * (std::sin(m_bobPhase * 2.f) * 0.028f * m_bobAmp) + right() * (std::sin(m_bobPhase) * 0.014f * m_bobAmp);
    return e;
}

QMatrix4x4 Player::viewMatrix() const
{
    Vec3 e = eyePos();
    Vec3 f = fwd;
    const GameState& st = w.state;
    if (st.shakeTime > 0.f && st.shakeAmp > 0.f) {
        float k = st.shakeAmp * std::min(1.f, st.shakeTime * 3.f);
        auto r = [&]() { return (float)QRandomGenerator::global()->generateDouble() * 2.f - 1.f; };
        e += Vec3(r(), r(), r()) * k;
        f = (f + Vec3(r(), r(), r()) * k * 0.6f).normalized();
    }
    QMatrix4x4 v;
    v.lookAt(e, e + f, up);
    return v;
}

void Player::look(float dx, float dy)
{
    const float sens = 0.0022f;
    QQuaternion q = QQuaternion::fromAxisAndAngle(up, -dx * sens * 57.2957795f);
    fwd = q.rotatedVector(fwd);
    float pitch = std::asin(clampf(Vec3::dotProduct(fwd, up), -1.f, 1.f));
    float np = clampf(pitch - dy * sens, -1.553f, 1.553f);
    Vec3 flat = fwd - up * Vec3::dotProduct(fwd, up);
    if (flat.lengthSquared() < 1e-6f) flat = right();
    flat.normalize();
    fwd = flat * std::cos(np) + up * std::sin(np);
}

void Player::teleport(const Vec3& feet, float yawDeg_, float pitchDeg_)
{
    pos = feet; vel = Vec3();
    float y = yawDeg_ * 0.0174533f, p = pitchDeg_ * 0.0174533f;
    Vec3 flat(-std::sin(y), 0, -std::cos(y));
    // projeta no plano perpendicular a up atual
    flat -= up * Vec3::dotProduct(flat, up);
    if (flat.lengthSquared() < 1e-6f) flat = Vec3(0, 0, -1);
    flat.normalize();
    fwd = flat * std::cos(p) + up * std::sin(p);
    lastSafe = pos; m_airborne = false; m_respawnTimer = -1.f; m_hazardAccum = 0.f;
    height = kStand; crouching = false; swimming = false; climbing = false;
    if (mode == Mode::Walk) { for (int i = 0; i < 4; ++i) resolve(); }
}

void Player::setMode(Mode m)
{
    if (m == mode) return;
    mode = m; vel = Vec3();
    if (m == Mode::Walk) {
        if (w.solidDistance(bodyCenter()) < 0.2f) {
            for (float dy = 0.25f; dy < 4.f; dy += 0.25f)
                if (w.solidDistance(pos + up * (dy + 0.9f)) > 0.5f) { pos += up * dy; break; }
        }
    }
}

void Player::rotateUp(const Vec3& target, float dt)
{
    float c = clampf(Vec3::dotProduct(up, target), -1.f, 1.f);
    float ang = std::acos(c);
    if (ang < 1e-3f) { up = target; return; }
    float step = std::min(ang, std::max(ang * (1.f - std::exp(-dt * 6.f)), dt * 0.9f));
    Vec3 axis = Vec3::crossProduct(up, target);
    if (axis.lengthSquared() < 1e-8f) axis = right();
    axis.normalize();
    QQuaternion q = QQuaternion::fromAxisAndAngle(axis, step * 57.2957795f);
    Vec3 mid = pos + up * (height * 0.5f);
    up = q.rotatedVector(up).normalized();
    fwd = q.rotatedVector(fwd).normalized();
    if (mode == Mode::Walk) pos = mid - up * (height * 0.5f);
}

bool Player::blockedAt(const Vec3& p, float h) const
{
    const float R = kRadius;
    float offs[3] = {R, h * 0.5f, h - R};
    for (float o : offs) if (w.solidDistance(p + up * o) < R - 0.03f) return true;
    return false;
}

bool Player::resolve()
{
    const float R = kRadius;
    bool contact = false;
    m_contactGround = false;
    for (int it = 0; it < 5; ++it) {
        bool any = false;
        float offs[3] = {R, height * 0.5f, height - R};
        int n = height > 2 * R + 0.3f ? 3 : 2;
        if (n == 2) { offs[1] = height - R; }
        for (int i = 0; i < n; ++i) {
            Vec3 c = pos + up * offs[i];
            float d = w.solidDistance(c);
            if (d < R) {
                Vec3 nrm = w.solidGradient(c);
                pos += nrm * (R - d + 0.001f);
                any = contact = true;
                if (Vec3::dotProduct(nrm, up) > kCos50) m_contactGround = true;
                float vn = Vec3::dotProduct(vel, nrm);
                if (vn < 0.f) vel -= nrm * vn;
            }
        }
        if (!any) break;
    }
    return contact;
}

bool Player::probeGround(Vec3* normal) const
{
    const float R = kRadius;
    Vec3 q = pos + up * R;
    float d = w.solidDistance(q);
    if (d > R + 0.12f) return false;
    Vec3 n = w.solidGradient(q - up * 0.03f);
    if (normal) *normal = n;
    return Vec3::dotProduct(n, up) > kCos50;
}

void Player::stepMove(const Vec3& move, bool wantStep)
{
    Vec3 p0 = pos;
    pos += move;
    resolve();
    Vec3 hm = move - up * Vec3::dotProduct(move, up);
    float hl = hm.length();
    if (!wantStep || hl < 1e-4f) return;
    Vec3 hd = hm / hl;
    float achieved = Vec3::dotProduct(pos - p0, hd);
    if (achieved >= hl * 0.6f) return;
    // tenta subir degrau (<= 0.45 m)
    Vec3 save = pos, saveV = vel;
    pos = p0 + up * 0.45f;
    if (blockedAt(pos, height)) { pos = save; return; }
    pos += hm * 1.0f;
    resolve();
    for (int i = 0; i < 11; ++i) {
        Vec3 t = pos - up * 0.05f;
        Vec3 keep = pos;
        pos = t;
        if (resolve() && m_contactGround) break;
        if (i == 10) { pos = keep; }
    }
    float prog = Vec3::dotProduct(pos - p0, hd);
    float rise = Vec3::dotProduct(pos - p0, up);
    if (prog > achieved + 0.02f && rise < 0.55f) vel = saveV; else { pos = save; vel = saveV; }
}

void Player::updateFly(float dt, const Input& in)
{
    float sp = flyBase * flySpeedScale * (in.sprint ? 5.f : 1.f);
    Vec3 f = fwd, r = right();
    Vec3 wish = f * in.forward + r * in.strafe + Vec3(0, 1, 0) * ((in.jumpHeld ? 1.f : 0.f) - (in.crouch ? 1.f : 0.f));
    pos += wish * sp * dt;
    vel = Vec3();
    grounded = swimming = climbing = false; m_airborne = false; m_bobAmp = 0.f; eyeUnderwater = false;
    height = kStand; crouching = false;
}

void Player::updateWalk(float dt, const Input& in)
{
    const ZoneState& zs = w.zoneState();
    const float R = kRadius;
    float gmag = std::max(zs.gravity.length(), 0.01f);
    m_jumpCooldown = std::max(0.f, m_jumpCooldown - dt);
    m_coyote = grounded ? 0.12f : std::max(0.f, m_coyote - dt);

    // ---- agachar ----
    bool wantCrouch = in.crouch && !swimming;
    if (!wantCrouch && crouching) {
        if (!blockedAt(pos, kStand)) crouching = false;
    } else if (wantCrouch) crouching = true;
    float th = crouching ? kCrouch : kStand;
    if (height < th) { float nh = std::min(th, height + 5.f * dt); if (!blockedAt(pos, nh)) height = nh; }
    else height = std::max(th, height - 6.f * dt);

    // ---- referencial de movimento ----
    Vec3 flatF = fwd - up * Vec3::dotProduct(fwd, up);
    if (flatF.lengthSquared() < 1e-6f) flatF = Vec3(0, 0, -1);
    flatF.normalize();
    Vec3 rgt = Vec3::crossProduct(flatF, up).normalized();
    Vec3 wish = flatF * in.forward + rgt * in.strafe;
    if (wish.length() > 1.f) wish.normalize();

    // ---- agua ----
    float feetH = pos.y(), surf = zs.waterSurfaceY;
    bool inWater = zs.water && std::fabs(up.y()) > 0.5f;
    float depth = inWater ? surf - feetH : -1.f;
    if (inWater && !swimming && depth > 1.35f) swimming = true;
    if (swimming && (!inWater || depth < 1.0f)) swimming = false;
    bool wading = inWater && !swimming && depth > 0.2f;
    eyeUnderwater = inWater && eyePos().y() < surf;

    // ---- escalada ----
    bool wasClimb = climbing;
    if (zs.climb && !swimming) {
        Vec3 q = pos + up * 0.9f + flatF * (R + 0.35f);
        bool wall = w.solidDistance(q) < 0.3f;
        if (!climbing && wall && in.forward > 0.2f) climbing = true;
        if (climbing && (!wall || in.jump)) climbing = false;
        if (climbing && !wall) climbing = false;
    } else climbing = false;
    (void)wasClimb;

    // ---- velocidades ----
    float speed = crouching ? crouchSpeed : (in.sprint ? sprintSpeed : walkSpeed);
    sprinting = in.sprint && !crouching && in.forward > 0.1f;
    if (wading) speed *= 0.6f;
    float vUp = Vec3::dotProduct(vel, up);
    Vec3 vH = vel - up * vUp;

    if (swimming) {
        vel *= std::exp(-2.0f * dt);
        Vec3 w3 = fwd * in.forward + rgt * in.strafe + up * ((in.jumpHeld ? 1.f : 0.f) - (in.crouch ? 1.f : 0.f));
        if (w3.length() > 1.f) w3.normalize();
        Vec3 target = w3 * 1.9f - up * 0.12f;
        vel += (target - vel) * (1.f - std::exp(-dt * 4.f));
        grounded = false;
    } else if (climbing) {
        Vec3 target = up * (in.forward * 2.0f) + rgt * (in.strafe * 1.3f) + flatF * 0.35f;
        // saliencia no topo: se acima da cabeca nao ha parede, impulsiona para a borda
        Vec3 headProbe = pos + up * (height + 0.1f) + flatF * (R + 0.4f);
        if (in.forward > 0.2f && w.solidDistance(headProbe) > 0.5f) target += flatF * 1.5f + up * 0.6f;
        vel += (target - vel) * (1.f - std::exp(-dt * 10.f));
        grounded = false;
    } else {
        float f = grounded ? 1.f - std::exp(-dt * 14.f) : 1.f - std::exp(-dt * 2.2f);
        Vec3 target = wish * speed;
        if (grounded || wish.lengthSquared() > 1e-4f) vH += (target - vH) * f;
        vel = vH + up * vUp;
        if (grounded && vUp <= 0.f) vel -= up * Vec3::dotProduct(vel, up);
        else vel += zs.gravity * dt;
        if ((grounded || m_coyote > 0.f) && in.jump && m_jumpCooldown <= 0.f) {
            vel += up * std::sqrt(2.f * gmag * 1.1f);
            grounded = false; m_jumpCooldown = 0.25f;
        }
    }

    // ---- integracao com subpassos ----
    Vec3 delta = vel * dt;
    int steps = std::max(1, (int)std::ceil(delta.length() / 0.1f));
    float sd = dt / steps;
    bool wasGrounded = grounded;
    for (int i = 0; i < steps; ++i) stepMove(vel * sd, wasGrounded && !swimming && !climbing && vel.lengthSquared() > 0.01f);

    // ---- chao ----
    if (!swimming && !climbing) {
        bool g = probeGround();
        if (!g && wasGrounded && Vec3::dotProduct(vel, up) <= 0.1f) {
            Vec3 save = pos;
            for (int i = 0; i < 8 && !g; ++i) { pos -= up * 0.05f; resolve(); g = probeGround(); }
            if (!g) pos = save;
        }
        grounded = g && Vec3::dotProduct(vel, up) <= 0.5f;
        if (grounded && Vec3::dotProduct(vel, up) < 0.f) vel -= up * Vec3::dotProduct(vel, up);
    } else grounded = false;

    // ---- queda ----
    float h = Vec3::dotProduct(pos, up);
    if ((zs.gravity - m_prevGravity).length() > 0.5f) { m_airborne = false; }
    m_prevGravity = zs.gravity;
    if (!grounded && !climbing && !swimming) {
        if (!m_airborne) { m_airborne = true; m_fallTop = h; } else m_fallTop = std::max(m_fallTop, h);
    } else {
        if (m_airborne && grounded) {
            float dist = m_fallTop - h;
            int dice = std::min(20, (int)(dist / 3.f));
            if (dice >= 1) w.state.damageText(QStringLiteral("Queda de %1 m — %2d6 de dano").arg((int)std::lround(dist)).arg(dice));
        }
        m_airborne = false;
    }

    // ---- balanco da cabeca ----
    float hs = (vel - up * Vec3::dotProduct(vel, up)).length();
    float targetAmp = grounded && hs > 0.3f ? clampf(hs / 3.f, 0.f, 1.6f) : 0.f;
    m_bobAmp += (targetAmp - m_bobAmp) * (1.f - std::exp(-dt * 8.f));
    if (grounded) m_bobPhase += hs * dt * 1.9f;

    // ---- ponto seguro ----
    m_safeTimer += dt;
    if (grounded && !zs.hazard && !zs.water && m_safeTimer > 0.5f) { lastSafe = pos; m_safeTimer = 0.f; }
}

void Player::handleHazards(float dt)
{
    const ZoneState& zs = w.zoneState();
    GameState& st = w.state;
    if (m_respawnTimer < 0.f) {
        if (zs.hazard && mode == Mode::Walk) {
            m_hazardAccum += dt;
            if (m_hazardAccum >= zs.hazardDelay) {
                st.damageText(zs.hazardText.isEmpty() ? QStringLiteral("Perigo mortal!") : zs.hazardText);
                st.fade(0.85f, 1.0f, QColor(150, 10, 0));
                m_respawnTimer = 1.5f;
            }
        } else m_hazardAccum = 0.f;
    } else {
        m_respawnTimer -= dt;
        if (m_respawnTimer <= 0.f) {
            pos = lastSafe; vel = Vec3(); m_respawnTimer = -1.f; m_hazardAccum = 0.f; m_airborne = false; swimming = false;
            st.fade(0.f, 0.6f);
            for (int i = 0; i < 3; ++i) resolve();
        }
    }
}

void Player::handleBreath(float dt)
{
    GameState& st = w.state;
    if (eyeUnderwater && mode == Mode::Walk) {
        st.breath -= dt; st.breathVisible = true;
        if (st.breath <= 0.f) {
            st.breath = 0.f; m_drownTimer -= dt;
            if (m_drownTimer <= 0.f) { st.damageText(QStringLiteral("Afogando: 1d10 de dano")); m_drownTimer = 4.f; }
        }
    } else {
        st.breath = std::min(st.maxBreath, st.breath + dt * 8.f);
        if (st.breath >= st.maxBreath - 0.05f) st.breathVisible = false;
        m_drownTimer = 0.f;
    }
}

void Player::update(float dt, const Input& in)
{
    dt = std::min(dt, 0.05f);
    const ZoneState& zs = w.zoneState();
    Vec3 target = up;
    if (mode == Mode::Fly) target = Vec3(0, 1, 0);
    else if (zs.gravity.length() > 0.1f) target = -zs.gravity.normalized();
    rotateUp(target, dt);
    if (mode == Mode::Fly) updateFly(dt, in); else updateWalk(dt, in);
    handleHazards(dt);
    handleBreath(dt);
}

void Player::snapToGround(float maxDist)
{
    if (mode != Mode::Walk) return;
    for (int i = 0; i < 4; ++i) resolve();
    for (float d = 0.f; d < maxDist; d += 0.05f) {
        if (probeGround()) break;
        pos -= up * 0.05f;
    }
    resolve();
    vel = Vec3(); m_airborne = false; grounded = probeGround(); lastSafe = pos;
}
