#include "SelfTest.h"
#include "CommandLine.h"
#include "game/Player.h"
#include "world/World.h"
#include <functional>

namespace {
struct Ctx { World& w; Player& p; int fails = 0; };

void sim(Ctx& c, float secs, const std::function<Player::Input(float)>& f)
{
    const float dt = 1.f / 60.f;
    for (float t = 0; t < secs; t += dt) {
        Player::Input in = f ? f(t) : Player::Input();
        c.p.update(dt, in);
        c.w.update(dt);
        c.w.state.update(dt);
    }
}
void check(Ctx& c, bool ok, const QString& what)
{
    outLine(QStringLiteral("%1 %2").arg(ok ? "OK   " : "FALHA").arg(what));
    if (!ok) ++c.fails;
}
void place(Ctx& c, Vec3 feet, float yaw = 0.f, float pitch = 0.f)
{
    c.p.mode = Player::Mode::Walk;
    c.p.teleport(feet, yaw, pitch);
    c.p.snapToGround(3.f);
    for (int i = 0; i < 3; ++i) c.w.update(0.016f);
}
Player::Input fwd(float f = 1.f) { Player::Input i; i.forward = f; return i; }
QString fmtp(const Vec3& p) { return QStringLiteral("(%1, %2, %3)").arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1).arg(p.z(), 0, 'f', 1); }
}

int runSelfTest(World& w)
{
    const float OX = 600.f;
    Player p(w);
    w.attachPlayer(&p);
    Ctx c{w, p};
    outLine(QStringLiteral("=== Auto-teste do jogador (sem GL) ==="));

    // 1) andar do spawn ate a boca e o tunel
    place(c, Vec3(OX, 0.f, 80), 0.f);
    int gcount = 0, total = 0;
    sim(c, 16.f, [&](float) { ++total; if (p.grounded) ++gcount; return fwd(); });
    outLine(QStringLiteral("  (grounded em %1% dos quadros)").arg(100 * gcount / std::max(1, total)));
    check(c, p.pos.z() < 40.f && gcount * 100 / std::max(1, total) > 85, "andar do spawn ate a boca da caverna (z=" + QString::number(p.pos.z(), 'f', 1) + ", y=" + QString::number(p.pos.y(), 'f', 2) + ")");
    sim(c, 22.f, [](float) { Player::Input i; i.forward = 1; i.sprint = true; return i; });
    check(c, p.pos.z() < -3.f, "correr pelo tunel ate a camara principal " + fmtp(p.pos));

    // 2) pulo (~1,1 m)
    place(c, Vec3(OX, 0.f, -12), 0.f);
    float y0 = p.pos.y(), maxY = y0;
    sim(c, 1.2f, [&](float t) { Player::Input i; i.jump = t < 0.02f; maxY = std::max(maxY, p.pos.y()); return i; });
    check(c, maxY - y0 > 0.85f && maxY - y0 < 1.5f, QStringLiteral("altura do pulo = %1 m").arg(maxY - y0, 0, 'f', 2));
    check(c, p.grounded, "pousou apos o pulo");

    // 3) agachar e levantar
    sim(c, 0.6f, [](float) { Player::Input i; i.crouch = true; return i; });
    check(c, p.crouching && p.height < 1.1f, QStringLiteral("agachar (altura %1)").arg(p.height, 0, 'f', 2));
    sim(c, 0.6f, nullptr);
    check(c, !p.crouching && p.height > 1.7f, "levantar com teto livre");

    // 4) degrau/estalagmite: andar leste->oeste pela camara sem travar (30 s de caminhada)
    place(c, Vec3(OX + 12, 0.f, -8), 90.f);
    Vec3 s = p.pos;
    sim(c, 6.f, [](float) { return fwd(); });
    check(c, (p.pos - s).length() > 10.f, QStringLiteral("caminhada continua pela camara: %1 m").arg((p.pos - s).length(), 0, 'f', 1));

    // 5) natacao
    place(c, Vec3(OX - 21.5f, 0.f, -12), 90.f);
    sim(c, 6.f, [](float) { return fwd(); });
    check(c, p.swimming, "nadar na piscina " + fmtp(p.pos));
    sim(c, 3.f, [](float) { Player::Input i; i.crouch = true; return i; });
    check(c, p.eyeUnderwater, "afundar (Ctrl) e ficar submerso; folego = " + QString::number(w.state.breath, 'f', 1));
    sim(c, 4.f, [](float) { Player::Input i; i.jumpHeld = true; return i; });
    check(c, !p.eyeUnderwater, "subir (Espaco) e emergir");

    // 6) escalada
    place(c, Vec3(OX - 36, 0.f, -45.0f), 0.f, 0.f);
    w.update(0.01f);
    // anda ate a parede do poco da chamine
    sim(c, 3.f, [](float) { return fwd(); });
    float climbStartY = p.pos.y();
    sim(c, 14.f, [](float) { return fwd(); });
    check(c, p.pos.y() - climbStartY > 10.f, QStringLiteral("escalar a chamine: subiu %1 m").arg(p.pos.y() - climbStartY, 0, 'f', 1));

    // 7) queda no poco helicoidal (dano de queda)
    w.state.toasts.clear();
    place(c, Vec3(OX + 45, 0.f, -12), 0.f);   // centro do poco (vazio) -> cai 17 m
    p.pos = Vec3(OX + 45, 0.1f, -12); p.vel = Vec3();
    sim(c, 4.f, nullptr);
    bool dmg = false;
    for (auto& t : w.state.toasts) if (t.text.contains("Queda")) dmg = true;
    check(c, dmg && p.pos.y() < -15.f, "queda de 17 m gera dano: " + (w.state.toasts.empty() ? QString("(sem toast)") : w.state.toasts.back().text));

    // 8) gravidade invertida: pula da plataforma
    place(c, Vec3(OX + 45, 2.0f, -52), 0.f);
    sim(c, 0.3f, nullptr);
    sim(c, 0.9f, [](float t) { Player::Input i; i.jump = t < 0.02f; i.jumpHeld = true; return i; });
    sim(c, 4.f, nullptr);
    check(c, p.up.y() < -0.9f, QStringLiteral("gravidade invertida: up.y = %1, pos %2").arg(p.up.y(), 0, 'f', 2).arg(fmtp(p.pos)));
    check(c, p.grounded && p.pos.y() > 8.f, "andando no teto (grounded=" + QString(p.grounded ? "sim" : "nao") + ")");
    Vec3 s2 = p.pos;
    sim(c, 2.f, [](float) { return fwd(); });
    check(c, (p.pos - s2).length() > 2.f, "andar de cabeca para baixo " + fmtp(p.pos));

    // 9) lava: hazard + respawn
    place(c, Vec3(OX + 7, 0.f, -52), 0.f);
    sim(c, 1.f, nullptr);
    Vec3 safe = p.lastSafe;
    p.pos = Vec3(OX, -2.5f, -52); p.vel = Vec3();
    w.state.toasts.clear();
    sim(c, 3.5f, nullptr);
    bool hz = false;
    for (auto& t : w.state.toasts) if (t.text.contains("Lava")) hz = true;
    check(c, hz && (p.pos - safe).length() < 6.f, "lava: dano e retorno ao ultimo ponto seguro " + fmtp(p.pos) + " (seguro " + fmtp(safe) + ")");

    // 10) ponte: caminhar sobre as tabuas (colisor) e colapsar
    w.fireEvent("restore");
    place(c, Vec3(OX - 8, 0.f, -52), -90.f);   // olhando para leste
    sim(c, 5.f, [](float) { return fwd(); });
    check(c, p.pos.x() > OX + 5.f && p.pos.y() > -0.5f, "atravessar a ponte de tabuas " + fmtp(p.pos));

    // 11) raycast e visibilidade
    RayHit h = w.raycast(Vec3(OX, 1.6f, -12), Vec3(1, 0, -1), 50.f);
    check(c, h.hit && h.t > 8.f && h.t < 20.f, QStringLiteral("raycast ate a parede norte da camara: t=%1").arg(h.t, 0, 'f', 1));

    outLine(c.fails ? QStringLiteral("AUTO-TESTE: %1 falha(s)").arg(c.fails) : QStringLiteral("AUTO-TESTE: tudo OK"));
    return c.fails ? 1 : 0;
}
