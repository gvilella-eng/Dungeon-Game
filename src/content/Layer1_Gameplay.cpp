// Camada 1 — zonas, armadilhas interativas, eventos do Mestre (G, B), atualizacoes por quadro e rota.
#include "Layer1_Common.h"
#include "game/Player.h"

namespace L1 {
namespace {

struct Runtime {
    // emboscada
    float ambT = -1.f; bool arrowDone = false;
    // ponte
    float brT = -1.f, brVy = 0.f, brPending = -1.f;
    // armadilha do fio
    float wireT = -1.f; std::vector<float> potVy, rockVy; std::vector<bool> potBroken, rockLanded;
    // gelo
    float iceT = -1.f;
};

ZoneEffects areaFx(const QString& name, const QString& sub, float temp, Vec3 fog, float dens, Vec3 amb)
{
    ZoneEffects fx;
    fx.areaName = name; fx.subtitle = sub; fx.temperature = temp;
    fx.fogColor = fog; fx.fogDensity = dens;
    fx.ambientColor = amb; fx.ambientGround = amb * 0.5f; fx.ambientStrength = 1.f;
    return fx;
}

bool inBox(const Vec3& p, const Vec3& mn, const Vec3& mx) { return Aabb(mn, mx).contains(p); }

} // namespace

void buildGameplay(World& w)
{
    State& S = st();
    std::shared_ptr<Runtime> R = std::make_shared<Runtime>();
    World* W = &w;

    // ================================================================ zonas de area (banner, temperatura, nevoa)
    // luz azul da boca sangrando para dentro da antecamara
    {
        ZoneEffects fx; fx.ambientColor = Vec3(0.085f, 0.11f, 0.19f); fx.ambientGround = Vec3(0.035f, 0.045f, 0.075f); fx.fogColor = Vec3(0.03f, 0.04f, 0.06f); fx.fogDensity = 0.03f;
        w.addZone(Q("boca_luz_1"), ZoneShape::makeBox(Vec3(-13, -4, -7), Vec3(13, 10, 0)), fx, 4);
        ZoneEffects f2; f2.ambientColor = Vec3(0.045f, 0.058f, 0.10f); f2.ambientGround = Vec3(0.02f, 0.025f, 0.04f); f2.fogColor = Vec3(0.02f, 0.025f, 0.04f);
        w.addZone(Q("boca_luz_2"), ZoneShape::makeBox(Vec3(-13, -4, -14), Vec3(13, 10, -7)), f2, 4);
    }
    w.addZone(Q("ante"), ZoneShape::makeBox(Vec3(-14, -4, -31), Vec3(14, 10, 0)),
              areaFx(Q("1.1 Antecâmara do Falso Ritual"), Q("Sessão 1 · Nível 6 · −10 °C junto aos braseiros"), -10.f, Vec3(0.010f, 0.009f, 0.010f), 0.026f, Vec3(0.022f, 0.018f, 0.018f)), 3);
    w.addZone(Q("corredor"), ZoneShape::makeBox(Vec3(-6.5f, -7, -72), Vec3(6.5f, 7, -27)),
              areaFx(Q("1.2 Corredor das Fendas"), Q("Seteiras a 5 m de altura · −10 °C"), -10.f, Vec3(0.007f, 0.007f, 0.010f), 0.026f, Vec3(0.014f, 0.014f, 0.020f)), 3);
    w.addZone(Q("cascata"), ZoneShape::makeBox(Vec3(-18.5f, -5, -46.5f), Vec3(-5.5f, 5, -34.5f)),
              areaFx(Q("1.5 A Cascata Congelada"), Q("Passagem secreta · −14 °C"), -14.f, Vec3(0.010f, 0.015f, 0.022f), 0.030f, Vec3(0.016f, 0.022f, 0.034f)), 4);
    w.addZone(Q("fenda"), ZoneShape::makeBox(Vec3(-22, -6, -71), Vec3(-10.5f, 1, -45.5f)),
              areaFx(Q("Fenda natural atrás do gelo"), Q("1,0 m de largura · 2,2 m de altura · −12 °C"), -12.f, Vec3(0.009f, 0.012f, 0.018f), 0.045f, Vec3(0.012f, 0.016f, 0.024f)), 4);
    w.addZone(Q("abismo"), ZoneShape::makeBox(Vec3(-33, -22, -85), Vec3(30, 7, -69.5f)),
              areaFx(Q("1.3 O Abismo do Rio Violento"), Q("A Ponte Podre · −6 °C"), -6.f, Vec3(0.008f, 0.011f, 0.015f), 0.017f, Vec3(0.016f, 0.020f, 0.028f)), 3);
    w.addZone(Q("toca"), ZoneShape::makeBox(Vec3(-26, -6.5f, -113.5f), Vec3(28, 3, -85)),
              areaFx(Q("1.4 A Toca"), Q("Acampamento goblin · −9 °C"), -9.f, Vec3(0.012f, 0.009f, 0.007f), 0.026f, Vec3(0.026f, 0.018f, 0.014f)), 3);
    w.addZone(Q("sifao_passagem"), ZoneShape::makeBox(Vec3(2, -6, -64), Vec3(25.8f, 0, -60)),
              areaFx(Q("Passagem do Sifão"), Q("Túnel úmido para o poço turvo · −6 °C"), -6.f, Vec3(0.007f, 0.012f, 0.014f), 0.035f, Vec3(0.012f, 0.020f, 0.024f)), 4);
    w.addZone(Q("sifao"), ZoneShape::makeBox(Vec3(25.5f, -11.5f, -98.5f), Vec3(39.5f, 0, -57.5f)),
              areaFx(Q("1.6 O Sifão Turvo"), Q("Totalmente submerso · água a 2 °C"), 2.f, Vec3(0.006f, 0.017f, 0.021f), 0.05f, Vec3(0.010f, 0.022f, 0.028f)), 4);
    w.addZone(Q("labirinto"), ZoneShape::makeBox(Vec3(-39, -18, -158.5f), Vec3(29, 0.5f, -110)),
              areaFx(Q("1.7 O Labirinto dos Leitos Secos"), Q("Leitos de rio ressecados · −12 °C"), -12.f, Vec3(0.010f, 0.009f, 0.009f), 0.034f, Vec3(0.016f, 0.015f, 0.016f)), 3);
    w.addZone(Q("lixao"), ZoneShape::makeBox(Vec3(-32, -9, -138), Vec3(-20, -2, -126)),
              areaFx(Q("O Lixão (beco sem saída)"), Q("Esconderijo secreto do Chefe a oeste"), -12.f, Vec3(0.012f, 0.010f, 0.008f), 0.042f, Vec3(0.018f, 0.015f, 0.012f)), 4);
    w.addZone(Q("despensa"), ZoneShape::makeBox(Vec3(16, -10, -127), Vec3(28, -3, -117)),
              areaFx(Q("A Despensa Congelada"), Q("Carcaças de cabras e um urso · −18 °C"), -18.f, Vec3(0.012f, 0.017f, 0.022f), 0.038f, Vec3(0.020f, 0.028f, 0.036f)), 4);
    w.addZone(Q("galeria_desabada"), ZoneShape::makeBox(Vec3(-22, -12, -154), Vec3(-10, -4, -141)),
              areaFx(Q("A Galeria Desabada"), Q("Beco soterrado · poeira no ar"), -12.f, Vec3(0.016f, 0.014f, 0.012f), 0.05f, Vec3(0.016f, 0.014f, 0.013f)), 4);
    {
        ZoneEffects fx = areaFx(Q("1.8 A Fronteira dos Ossos"), Q("O último vestígio goblin · −20 °C"), -20.f, Vec3(0.008f, 0.009f, 0.013f), 0.04f, Vec3(0.012f, 0.014f, 0.022f));
        fx.psychic = 0.12f;
        w.addZone(Q("fronteira"), ZoneShape::makeBox(Vec3(13, -22, -173), Vec3(33, -8, -153.5f)), fx, 4);
    }

    // agua: rio violento, sifao (tres volumes com a mesma superficie), sumidouro, escalada do fosso
    {
        ZoneEffects wf; wf.water = true; wf.waterSurfaceY = -18.5f;
        w.addZone(Q("agua_rio"), ZoneShape::makeBox(Vec3(-32, -21.5f, -84), Vec3(28, -18.5f, -70)), wf, 6);
        ZoneEffects sf; sf.water = true; sf.waterSurfaceY = -5.5f;
        w.addZone(Q("agua_sifao_entrada"), ZoneShape::makeBox(Vec3(25.8f, -10.5f, -66), Vec3(34.2f, -5.5f, -58)), sf, 6);
        w.addZone(Q("agua_sifao_tubo"), ZoneShape::makeBox(Vec3(28.5f, -11.5f, -96.f), Vec3(39.f, -5.5f, -60)), sf, 6);
        w.addZone(Q("agua_sifao_saida"), ZoneShape::makeBox(Vec3(25.8f, -10.5f, -98.f), Vec3(34.2f, -5.5f, -89.5f)), sf, 6);
        ZoneEffects hz; hz.hazard = true; hz.hazardText = Q("Arrastado para o sumidouro! (Atletismo CD 14 falhou) 5d6 de dano e fim do percurso.");
        hz.hazardDelay = 0.5f;
        w.addZone(Q("sumidouro"), ZoneShape::makeBox(Vec3(24.5f, -21.5f, -80), Vec3(33, -17.8f, -74)), hz, 7);
        ZoneEffects cf; cf.climb = true;
        w.addZone(Q("fosso_escalada"), ZoneShape::makeBox(Vec3(-1.6f, corrFloor(-47.5f) - 3.2f, -49.1f), Vec3(1.6f, corrFloor(-47.5f) - 0.1f, -45.9f)), cf, 5);
    }

    // ================================================================ luzes extras
    w.addLight(Vec3(0.f, 5.f, -3.f), Vec3(0.35f, 0.5f, 0.95f), 2.4f, 14.f, LightAnim::None, Q("luz da boca na antecamara"));

    // ================================================================ armadilha: Fosso de Estacas Coberto
    {
        const float fy = corrFloor(-47.5f);
        auto collapse = [W, &S]() {
            if (S.pitOpen) return;
            S.pitOpen = true;
            if (S.pitCol) S.pitCol->enabled = false;
            if (S.pitCover) S.pitCover->visible = false;
            W->state.screenShake(0.09f, 0.6f);
            W->state.toast(Q("A lona cede! Você despenca num fosso de estacas (Percepção passiva 14 falhou)."), QColor(255, 190, 120), 4.f);
        };
        w.addTrigger(Q("fosso_cobertura"), ZoneShape::makeBox(Vec3(-1.5f, fy + 0.05f, -49.f), Vec3(1.5f, fy + 2.2f, -46.f)), collapse);
        w.addTrigger(Q("fosso_estacas"), ZoneShape::makeBox(Vec3(-1.6f, fy - 3.3f, -49.1f), Vec3(1.6f, fy - 1.0f, -45.9f)), [W]() {
            W->state.damageText(Q("Estacas! 2d6 de dano perfurante (somado ao 1d6 da queda de 3 m). Escalar para sair: Atletismo CD 10."));
        });
    }

    // ================================================================ armadilha: Fio de Tropeco (z -59)
    {
        const float wy = corrFloor(-59.f);
        R->potVy.assign(S.pots.size(), 0.f); R->potBroken.assign(S.pots.size(), false);
        R->rockVy.assign(S.rocks.size(), 0.f); R->rockLanded.assign(S.rocks.size(), false);
        w.addTrigger(Q("fio_de_tropeco"), ZoneShape::makeBox(Vec3(-2.45f, wy + 0.05f, -59.3f), Vec3(2.45f, wy + 1.9f, -58.7f)), [W, &S, R]() {
            if (S.wireTripped) return;
            S.wireTripped = true; R->wireT = 0.f;
            W->state.screenShake(0.12f, 0.8f);
            W->state.damageText(Q("Fio de tropeço! Potes de óleo e pedras desabam do teto: Destreza CD 13 ou 2d10 contundente. Você fica coberto de óleo (vulnerável a fogo até se limpar)."));
            W->state.setFlag(Q("oleado"), 1.0);
        });
    }

    // ================================================================ cascata congelada e esconderijo: interativos [E]
    w.addInteractable(Q("quebrar_gelo"), Vec3(-12.f, -1.7f, -43.7f), 2.6f, Q("Quebrar o gelo da cascata (faz barulho!)"), [W, &S, R]() {
        if (S.iceBroken) return;
        S.iceBroken = true; R->iceT = 0.f;
        if (S.iceCol) S.iceCol->enabled = false;
        W->state.screenShake(0.09f, 0.7f);
        W->state.setFlag(Q("goblins_alertados"), 1.0);
        W->state.toast(Q("O gelo estilhaça com um estrondo que ecoa pelo corredor! Os goblins foram alertados. Atrás: uma fenda estreita."), QColor(190, 225, 255), 5.f);
    });
    w.addInteractable(Q("pedras_soltas"), Vec3(-29.5f, -5.9f, -133.6f), 2.0f, Q("Afastar as pedras soltas (Investigação CD 14)"), [W, &S]() {
        if (S.hideoutOpen) return;
        S.hideoutOpen = true;
        if (S.hideoutCol) S.hideoutCol->enabled = false;
        if (S.hideoutRocks) S.hideoutRocks->visible = false;
        W->state.toast(Q("Atrás das pedras: o Esconderijo do Chefe, com o resto dos tesouros do templo e 120 po."), QColor(255, 220, 130), 5.f);
        W->state.addItem(Q("120 po e relíquias do templo"));
    });

    // ================================================================ ponte: corte das cordas
    auto cutBridge = [W, &S, R](bool autoCut) {
        if (S.bridgeCut) return;
        S.bridgeCut = true; R->brT = 0.f; R->brVy = 0.f;
        for (Collider* c : S.bridgeCols) c->enabled = false;
        W->state.screenShake(0.14f, 0.9f);
        if (autoCut) {
            for (size_t i = 0; i < 2 && i < S.archers.size(); ++i) S.archers[i]->visible = true;   // ninho e alcova
            if (!S.eyes.empty()) S.eyes.back()->visible = true;
            W->state.damageText(Q("Os goblins cortam as cordas! A Ponte Podre desaba: Destreza CD 15 para se agarrar; falha = queda de 16 m (5d6) na água gelada e Atletismo CD 14 contra a correnteza."));
        } else
            W->state.toast(Q("As cordas da Ponte Podre foram cortadas. As tábuas despencam no abismo."), QColor(255, 190, 130), 4.f);
    };
    w.addTrigger(Q("meio_da_ponte"), ZoneShape::makeBox(Vec3(-1.3f, -4.6f, -79.8f), Vec3(1.3f, -1.f, -74.2f)), [&S, R]() {
        if (S.bridgeAuto || S.bridgeCut) return;
        S.bridgeAuto = true; R->brPending = 0.9f;
    });
    w.registerEvent(Q("cortar_cordas"), Q("Cortar as cordas da Ponte Podre"), Qt::Key_B, [cutBridge]() { cutBridge(false); });

    // ================================================================ emboscada goblin
    w.registerEvent(Q("emboscada_goblin"), Q("Emboscada goblin (luzes apagam, olhos acendem)"), Qt::Key_G, [W, &S, R]() {
        if (S.ambushed) { W->state.toast(Q("A emboscada já está em curso. Use 'Rearmar Camada 1' para repetir."), QColor(220, 220, 220), 3.f); return; }
        S.ambushed = true; R->ambT = 0.f; R->arrowDone = false;
        for (auto& b : S.braziers) { b.first->visible = false; b.second->visible = true; }
        for (Light* l : S.embers) l->enabled = false;
        for (DynHandle e : S.eyes) e->visible = true;
        for (DynHandle a : S.archers) a->visible = true;
        W->state.screenShake(0.03f, 0.5f);
        W->state.toast(Q("As luzes dos goblins se apagam! Olhos verdes acendem nas seteiras e na galeria."), QColor(170, 255, 90), 4.f);
    });

    // rearmar tudo
    w.registerEvent(Q("rearmar_camada1"), Q("Rearmar Camada 1 (ponte, armadilhas, emboscada, gelo)"), 0, [W, &S, R]() {
        S.ambushed = false; R->ambT = -1.f;
        for (auto& b : S.braziers) { b.first->visible = true; b.second->visible = false; }
        for (Light* l : S.embers) l->enabled = true;
        for (DynHandle e : S.eyes) e->visible = false;
        for (DynHandle a : S.archers) a->visible = false;
        S.bridgeCut = false; S.bridgeAuto = false; R->brT = -1.f; R->brPending = -1.f;
        if (S.bridge) { S.bridge->visible = true; S.bridge->pos = Vec3(0, -4.f, -77.f); S.bridge->rot = QQuaternion(); }
        for (Collider* c : S.bridgeCols) c->enabled = true;
        S.pitOpen = false;
        if (S.pitCol) S.pitCol->enabled = true;
        if (S.pitCover) S.pitCover->visible = true;
        S.wireTripped = false; R->wireT = -1.f;
        const float shelfY = corrFloor(-59.f) + 7.55f - 0.03f;
        float px[6] = {-1.9f, -1.1f, -0.3f, 0.5f, 1.3f, 2.0f};
        for (size_t i = 0; i < S.pots.size(); ++i) { S.pots[i]->visible = true; S.pots[i]->rot = QQuaternion(); S.pots[i]->pos = Vec3(px[i], shelfY, -60.2f + (i % 2 ? 0.2f : -0.15f)); R->potVy[i] = 0; R->potBroken[i] = false; }
        float rx[4] = {-1.5f, -0.7f, 0.9f, 1.7f};
        for (size_t i = 0; i < S.rocks.size(); ++i) { S.rocks[i]->visible = true; S.rocks[i]->rot = QQuaternion(); S.rocks[i]->pos = Vec3(rx[i], shelfY, -60.45f); R->rockVy[i] = 0; R->rockLanded[i] = false; }
        if (S.oilPuddle) S.oilPuddle->visible = false;
        W->state.setFlag(Q("oleado"), 0.0);
        S.iceBroken = false; R->iceT = -1.f;
        if (S.iceSheet) { S.iceSheet->visible = true; S.iceSheet->scl = Vec3(1, 1, 1); }
        if (S.iceCol) S.iceCol->enabled = true;
        S.hideoutOpen = false;
        if (S.hideoutCol) S.hideoutCol->enabled = true;
        if (S.hideoutRocks) S.hideoutRocks->visible = true;
        W->state.toast(Q("Camada 1 rearmada."), QColor(200, 255, 200), 2.5f);
    });

    // ================================================================ atualizacoes por quadro
    w.addUpdater([W, &S, R, cutBridge](float dt, float) {
        World& w = *W;
        Vec3 pp = w.playerPos();
        // ---- emboscada: flecha de barro apaga a tocha
        if (R->ambT >= 0.f) {
            R->ambT += dt;
            if (!R->arrowDone && R->ambT > 1.8f) {
                R->arrowDone = true;
                bool inZone = inBox(pp, Vec3(-14, -4, -31), Vec3(14, 10, 0)) || inBox(pp, Vec3(-6.5f, -7, -72), Vec3(6.5f, 7, -27)) || inBox(pp, Vec3(-26, -6.5f, -113.5f), Vec3(28, 3, -85));
                if (inZone) {
                    w.state.screenShake(0.05f, 0.4f);
                    if (w.state.torchLit) w.state.extinguishTorch(Q("Uma flecha de barro cheia de água estoura contra a sua tocha!"));
                    w.state.toast(Q("Fffft! Uma flecha de barro cheia de água apaga a sua tocha. Escuridão total: acenda outra com [R]."), QColor(255, 200, 140), 5.f);
                } else
                    w.state.toast(Q("Flechas de barro assobiam na escuridão (fora do alcance do corredor/toca)."), QColor(220, 220, 220), 3.f);
            }
        }
        // ---- ponte: corte automatico no meio
        if (R->brPending >= 0.f) {
            R->brPending -= dt;
            if (R->brPending <= 0.f) { R->brPending = -1.f; cutBridge(true); }
        }
        if (R->brT >= 0.f && S.bridge && R->brT < 3.f) {
            R->brT += dt; R->brVy -= 9.8f * dt;
            S.bridge->pos.setY(S.bridge->pos.y() + R->brVy * dt);
            S.bridge->setRotation(QQuaternion::fromAxisAndAngle(Vec3(0.15f, 0.1f, 1.f), R->brT * 48.f));
            if (S.bridge->pos.y() < -19.f || R->brT >= 3.f) { S.bridge->visible = false; R->brT = 3.5f; }
        }
        // ---- fio de tropeco: potes e pedras despencam
        if (R->wireT >= 0.f) {
            R->wireT += dt;
            for (size_t i = 0; i < S.pots.size(); ++i) {
                if (R->wireT < 0.08f * i || R->potBroken[i]) continue;
                R->potVy[i] -= 9.8f * dt;
                DynHandle d = S.pots[i];
                d->pos.setY(d->pos.y() + R->potVy[i] * dt);
                d->setRotation(QQuaternion::fromAxisAndAngle(Vec3(1, 0, 0.3f), R->wireT * 200.f));
                float fl = corrFloor(d->pos.z());
                if (d->pos.y() <= fl + 0.05f) {
                    R->potBroken[i] = true; d->visible = false;
                    if (S.oilPuddle) S.oilPuddle->visible = true;
                }
            }
            for (size_t i = 0; i < S.rocks.size(); ++i) {
                if (R->wireT < 0.12f * i || R->rockLanded[i]) continue;
                R->rockVy[i] -= 9.8f * dt;
                DynHandle d = S.rocks[i];
                d->pos.setY(d->pos.y() + R->rockVy[i] * dt);
                d->setRotation(QQuaternion::fromAxisAndAngle(Vec3(0.2f, 1, 0.5f), R->wireT * 120.f));
                float fl = corrFloor(d->pos.z());
                if (d->pos.y() <= fl) { d->pos.setY(fl); R->rockLanded[i] = true; }
            }
        }
        // ---- gelo: encolhe e some
        if (R->iceT >= 0.f && S.iceSheet) {
            R->iceT += dt;
            float s = std::max(0.02f, 1.f - R->iceT / 0.6f);
            S.iceSheet->scl = Vec3(1.f + (1.f - s) * 0.2f, s, 1.f);
            if (R->iceT >= 0.6f) { S.iceSheet->visible = false; R->iceT = -1.f; }
        }
        // ---- correnteza do rio violento empurra quem nada para o sumidouro (leste)
        Player* pl = w.player();
        if (pl && pl->swimming && pp.y() > -22.f && pp.y() < -17.5f && pp.z() < -70.f && pp.z() > -84.f && pp.x() > -32.f && pp.x() < 28.f)
            pl->pos.setX(pl->pos.x() + 1.7f * dt);
    });

    // ================================================================ rota (continuacao: a Camada 0 ja adicionou o inicio)
    w.addRoutePoint(Q("1.1 Antecâmara do Falso Ritual"), Vec3(0.f, -0.7f, -10.f));
    w.addRoutePoint(Q("1.1 Saída da antecâmara"), Vec3(0.f, -1.3f, -26.f));
    w.addRoutePoint(Q("1.2 Corredor das Fendas"), Vec3(0.f, -2.4f, -38.f));
    w.addRoutePoint(Q("1.2 Corredor (fim)"), Vec3(0.f, -3.2f, -55.f));
    w.addRoutePoint(Q("1.3 Cabeceira da Ponte Podre"), Vec3(0.f, -3.4f, -69.f));
    w.addRoutePoint(Q("1.3 Ponte Podre (meio)"), Vec3(0.f, -3.5f, -77.f));
    w.addRoutePoint(Q("1.4 Entrada da Toca"), Vec3(0.f, -4.1f, -86.5f));
    w.addRoutePoint(Q("1.4 A Toca (acampamento)"), Vec3(0.f, -4.4f, -98.f));
    w.addRoutePoint(Q("1.7 N0 · saída norte"), Vec3(0.f, -4.5f, -112.5f));
    w.addRoutePoint(Q("1.7 N1"), Vec3(0.f, -5.4f, -120.f));
    w.addRoutePoint(Q("1.7 N4"), Vec3(12.f, -6.4f, -127.f));
    w.addRoutePoint(Q("1.7 N8"), Vec3(6.f, -7.4f, -140.f));
    w.addRoutePoint(Q("1.7 N9"), Vec3(12.f, -9.4f, -150.f));
    w.addRoutePoint(Q("1.7 N10"), Vec3(18.f, -12.4f, -157.f));
    w.addRoutePoint(Q("1.8 Fronteira dos Ossos"), Vec3(22.f, -15.4f, -162.f));
    w.addRoutePoint(Q("Borda do Poço"), Vec3(27.f, -18.4f, -167.f));
}

} // namespace L1
