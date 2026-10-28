// CAMADA 3 — 3.5 A Fenda da Guerra Elemental: Tubo C (ao norte) e a fenda alta e estreita, parede leste de gelo,
// parede oeste de magma, 8 Mephits de Gelo contra 8 Mephits de Magma (objetos dinamicos animados por updater).
#include "Layer3_Parts.h"
#include "Prefabs.h"
#include <memory>

#define Q(s) QStringLiteral(s)
using namespace L23;
using namespace L3;

namespace {

void mephit(MeshBuilder& mb, bool ice)
{
    const QColor body = ice ? QColor(190, 225, 255, 215) : QColor(46, 32, 30);
    const int bm = ice ? MAT_CLEAR_ICE : MAT_COAL;
    const QColor eye = ice ? QColor(170, 240, 255) : QColor(255, 180, 40);
    mb.ellipsoid(V3(0, 0, 0), V3(0.17f, 0.24f, 0.15f), body, bm, 7);
    mb.sphere(V3(0, 0.3f, -0.03f), 0.12f, body, bm, 7);
    for (int k = -1; k <= 1; k += 2) {
        mb.cone(V3(k * 0.07f, 0.38f, -0.03f), V3(k * 0.13f, 0.54f, -0.07f), 0.035f, 5, ice ? QColor(230, 245, 255) : QColor(255, 120, 30), ice ? MAT_CLEAR_ICE : MAT_EMISSIVE);
        mb.sphere(V3(k * 0.05f, 0.32f, -0.13f), 0.025f, eye, MAT_EMISSIVE, 5);
        // asas (duas faces)
        Vec3 p0(k * 0.1f, 0.18f, 0.08f), p1(k * 0.1f, -0.08f, 0.1f), p2(k * 0.62f, 0.02f, 0.38f), p3(k * 0.74f, 0.42f, 0.28f);
        QColor wc = ice ? QColor(225, 242, 255, 140) : QColor(255, 110, 30);
        int wm = ice ? MAT_CRYSTAL : MAT_EMISSIVE;
        mb.quad(p0, p1, p2, p3, wc, wm); mb.quad(p3, p2, p1, p0, wc, wm);
        mb.cylinder(p0, p3, 0.015f, 4, ice ? QColor(200, 225, 245) : QColor(60, 30, 20), ice ? MAT_CLEAR_ICE : MAT_COAL);
    }
    mb.cone(V3(0, -0.15f, 0.05f), V3(0, -0.5f, 0.12f), 0.07f, 5, body, bm);
    if (!ice) for (int i = 0; i < 5; ++i)                       // rachaduras de magma no corpo
        mb.box(V3(std::sin(i * 2.3f) * 0.13f, std::cos(i * 1.7f) * 0.14f, -0.12f), V3(0.03f, 0.008f, 0.01f), QColor(255, 130, 30), MAT_EMISSIVE);
}

struct MephitSt { DynHandle d; bool ice; int k; float phase; };

} // namespace

void L3::buildFissure(World& w)
{
    const QString kSess = Q("Sessão 3 · Nível 7→8");
    const QString kGrp = Q("Camada 3 · As Veias Vítreas");
    using namespace Prefabs;
    Rnd rg(606);

    // eixo da fenda (pontos do chao)
    const Vec3 S = kFissS, E = kFissE;
    Vec3 hd = E - S; hd.setY(0); const float hlen = hd.length(); hd /= hlen;
    const Vec3 right(-hd.z(), 0.f, hd.x());                          // leste = direita ao olhar para o norte
    auto axisFloor = [S, E](float u) {
        Vec3 p = S + (E - S) * u;
        return V3(p.x(), p.y(), p.z());
    };
    auto groundAt = [&](const Vec3& p) { return groundY(w, p.x(), p.y(), p.z(), 3.f, 12.f); };

    // ---- decoracao das paredes ----
    {
        MeshBuilder mb;
        // veios de magma na parede oeste
        for (int i = 0; i < 16; ++i) {
            float u = rg.f(0.04f, 0.96f), y0 = rg.f(0.5f, 20.f);
            std::vector<Vec3> pts; bool ok = true;
            for (int j = 0; j < 6 && ok; ++j) {
                Vec3 o = axisFloor(u) + V3(0, y0 + j * 1.6f, 0) + hd * rg.f(-0.25f, 0.25f);
                RayHit h = w.raycast(o, -right, 6.f, false);
                if (!h.hit) { ok = false; break; }
                pts.push_back(h.pos + h.normal * 0.05f);
            }
            if (ok) mb.tube(pts, std::vector<float>{0.07f, 0.12f, 0.1f, 0.13f, 0.09f, 0.05f}, 5, QColor(255, 110, 20), MAT_LAVA);
        }
        // cristais de gelo e geada na parede leste
        for (int i = 0; i < 20; ++i) {
            float u = rg.f(0.03f, 0.97f), y0 = rg.f(0.5f, 24.f);
            Vec3 o = axisFloor(u) + V3(0, y0, 0);
            RayHit h = w.raycast(o, right, 6.f, false);
            if (!h.hit) continue;
            crystalCluster(mb, h.pos + h.normal * 0.02f, h.normal, rg.f(0.35f, 0.7f), rg.i(3, 5), 600 + i, QColor(175, 228, 255), true);
        }
        for (int i = 0; i < 14; ++i) {                                  // placas de geada (finas) na parede leste
            float u = rg.f(0.03f, 0.97f), y0 = rg.f(0.5f, 20.f);
            Vec3 o = axisFloor(u) + V3(0, y0, 0);
            RayHit h = w.raycast(o, right, 6.f, false);
            if (!h.hit) continue;
            mb.push(); mb.translate(h.pos + h.normal * 0.04f); mb.rotate(QQuaternion::rotationTo(Vec3(0, 1, 0), h.normal));
            mb.ellipsoid(V3(0, 0, 0), V3(rg.f(0.8f, 1.8f), 0.04f, rg.f(0.8f, 1.6f)), QColor(215, 240, 255, 150), MAT_CLEAR_ICE, 6);
            mb.pop();
        }
        w.addStatic(std::move(mb));
        // estalagmites de gelo (leste) e de basalto (oeste) rentes as paredes
        for (int i = 0; i < 9; ++i) {
            float u = rg.f(0.08f, 0.96f);
            Vec3 p = axisFloor(u) + right * rg.f(2.1f, 2.6f);
            w.stalagmite(V3(p.x(), groundAt(p) - 0.1f, p.z()), rg.f(1.2f, 3.2f), rg.f(0.3f, 0.55f), MAT_ICE);
        }
        for (int i = 0; i < 8; ++i) {
            float u = rg.f(0.08f, 0.96f);
            Vec3 p = axisFloor(u) - right * rg.f(2.1f, 2.6f);
            w.stalagmite(V3(p.x(), groundAt(p) - 0.1f, p.z()), rg.f(0.8f, 2.4f), rg.f(0.3f, 0.5f), MAT_HOT_BASALT);
        }
    }

    // ---- luzes: metade azul / metade laranja ----
    for (int i = 0; i < 4; ++i) {
        float u = 0.12f + i * 0.25f;
        Vec3 c = axisFloor(u);
        w.addLight(c + right * 1.9f + V3(0, 6.f + i * 3.f, 0), V3(0.4f, 0.7f, 1.0f), 3.4f, 13.f, LightAnim::Pulse, Q("gelo da fenda"))->animFreq = 0.2f + 0.05f * i;
        w.addLight(c - right * 1.9f + V3(0, 5.f + i * 3.f, 0), V3(1.0f, 0.4f, 0.1f), 6.f, 16.f, LightAnim::Pulse, Q("magma da fenda"))->animFreq = 0.35f + 0.06f * i;
    }
    {
        Vec3 c = axisFloor(0.5f);
        EmitterDesc sn; sn.preset = ParticlePreset::Snow; sn.name = Q("neve da fenda"); ParticleEmitter::applyPreset(sn);
        sn.pos = c + right * 1.6f + V3(0, 12.f, 0); sn.halfExtent = V3(1.1f, 12.f, 14.f); sn.rate = 65.f; sn.velocity = V3(0.2f, -0.9f, 0.f); sn.lifeMin = 6.f; sn.lifeMax = 10.f;
        w.addEmitter(sn);
        EmitterDesc em; em.preset = ParticlePreset::Embers; em.name = Q("brasas da fenda"); ParticleEmitter::applyPreset(em);
        em.pos = c - right * 1.6f + V3(0, 12.f, 0); em.halfExtent = V3(1.1f, 12.f, 14.f); em.rate = 60.f; em.velocity = V3(0.f, 0.6f, 0.f); em.lifeMin = 3.f; em.lifeMax = 6.f;
        w.addEmitter(em);
    }

    // ---- os mephits: 8 de gelo (leste) e 8 de magma (oeste) ----
    auto mephits = std::make_shared<std::vector<MephitSt>>();
    std::vector<EmitterHandle> clashFx;
    for (int k = 0; k < 8; ++k) {
        for (int side = 0; side < 2; ++side) {
            MeshBuilder mb; mephit(mb, side == 0);
            MephitSt m; m.ice = side == 0; m.k = k; m.phase = k * 0.77f + side * 0.3f;
            m.d = w.addDynamic(Q("mephit_%1_%2").arg(side == 0 ? Q("gelo") : Q("magma")).arg(k + 1), std::move(mb), axisFloor(0.5f) + V3(0, 14.f, 0));
            mephits->push_back(m);
        }
        EmitterDesc fx; fx.preset = ParticlePreset::Sparks; fx.name = Q("choque de mephits %1").arg(k + 1); ParticleEmitter::applyPreset(fx);
        fx.rate = 40.f; fx.halfExtent = V3(0.15f, 0.15f, 0.15f); fx.colorStart = QColor(200, 235, 255, 255); fx.colorEnd = QColor(255, 80, 20, 0); fx.enabled = false;
        clashFx.push_back(w.addEmitter(fx));
    }
    auto axisFn = axisFloor;
    w.addUpdater([=, &w](float dt, float t) {
        Vec3 pp = w.playerPos();
        if ((pp - axisFn(0.5f)).length() > 70.f) return;
        for (auto& m : *mephits) {
            int k = m.k; float sd = m.ice ? 1.f : -1.f;
            float u = 0.12f + 0.105f * k;
            float h = 10.f + 2.6f * std::sin(k * 1.7f) + (k % 3) * 3.2f;
            float cl = std::sin(6.2831853f * (t / 7.f + k * 0.125f));
            float clash = cl > 0.f ? std::pow(cl, 5.f) : 0.f;
            float lat = lerpf(sd * 1.85f, sd * 0.18f, clash) + sd * 0.45f * std::sin(t * 1.3f + m.phase);
            float fwd = 1.6f * std::sin(t * 0.9f + k) * (1.f - clash);
            float hh = h + 1.3f * std::sin(t * 1.7f + m.phase) + 0.3f * clash;
            Vec3 pos = axisFn(u) + hd * fwd + right * lat + V3(0, hh, 0);
            m.d->pos = pos;
            QQuaternion yaw = QQuaternion::fromAxisAndAngle(0, 1, 0, yawDir(-sd * right.x(), -sd * right.z()) + 12.f * std::sin(t * 2.f + m.phase));
            m.d->setRotation(yaw * QQuaternion::fromAxisAndAngle(0, 0, 1, std::sin(t * 22.f + m.phase) * 13.f) * QQuaternion::fromAxisAndAngle(1, 0, 0, 10.f * clash));
        }
        for (int k = 0; k < 8; ++k) {
            float cl = std::sin(6.2831853f * (t / 7.f + k * 0.125f));
            float clash = cl > 0.f ? std::pow(cl, 5.f) : 0.f;
            float u = 0.12f + 0.105f * k;
            float h = 10.f + 2.6f * std::sin(k * 1.7f) + (k % 3) * 3.2f;
            clashFx[k]->setPosition(axisFn(u) + V3(0, h + 1.f, 0));
            clashFx[k]->setEnabled(clash > 0.55f);
        }
    });

    // ---- gatilho de entrada ----
    w.addTrigger(Q("fenda_entrada"), ZoneShape::makeBox(V3(-92.f, -192.f, -304.f), V3(-82.f, -186.f, -298.f)),
        [&w]() { w.state.toast(Q("Acima, flocos de neve e brasas chovem: dois exércitos de mephits se estraçalham no alto da fenda."), QColor(200, 225, 255), 5.f); }, nullptr, true);

    // ---- zonas ----
    {
        ZoneEffects fz;
        fz.areaName = Q("3.5 · A Fenda da Guerra Elemental"); fz.subtitle = Q("Leste: gelo e geada · Oeste: magma"); fz.temperature = 8.f;
        fz.fogColor = V3(0.014f, 0.014f, 0.024f); fz.fogDensity = 0.02f;
        fz.ambientColor = V3(0.05f, 0.05f, 0.08f); fz.ambientGround = V3(0.03f, 0.025f, 0.03f);
        w.addZone(Q("fenda"), ZoneShape::makeBox(V3(-96.f, -196.f, -338.f), V3(-80.f, -160.f, -298.f)), fz, 4);
        // metades: leste (gelo, -6 C) e oeste (magma, 30 C) — caixas alinhadas ao eixo medio
        ZoneEffects fe; fe.temperature = -6.f; fe.fogColor = V3(0.018f, 0.03f, 0.05f); fe.ambientColor = V3(0.04f, 0.08f, 0.13f);
        w.addZone(Q("fenda_leste"), ZoneShape::makeBox(V3(-87.5f, -196.f, -338.f), V3(-82.f, -160.f, -298.f)), fe, 5);
        ZoneEffects fo; fo.temperature = 30.f; fo.fogColor = V3(0.04f, 0.014f, 0.006f); fo.ambientColor = V3(0.12f, 0.05f, 0.03f);
        w.addZone(Q("fenda_oeste"), ZoneShape::makeBox(V3(-96.f, -196.f, -338.f), V3(-87.5f, -160.f, -298.f)), fo, 5);
    }

    // ---- POIs ----
    w.addPoi(V3(-80.f, -185.f, -283.f), PoiCategory::Area, Q("3.5 · Tubo C (para o norte)"),
        Q("Tubo C: (−78, −186, −277) → (−82, −188, −292) → início da fenda (−85, −189, −300). Mesma obsidiana vítrea; o ar começa a ficar cortante: uma metade quente e uma metade fria se cruzam no ar. Marcas de arrasto de Kaelen cruzam o tubo (rota até a Máquina do Frio).\n\n"
          "Perceba: um assobio agudo e o tilintar de gelo ao longe; luz azul e laranja tremulando no fundo. Percepção CD 12: eco de pequenas asas batendo e de guinchos."), kSess, kGrp);
    w.addPoi(V3(-86.5f, -187.f, -306.f), PoiCategory::Area, Q("3.5 · A Fenda da Guerra Elemental"),
        Q("Fenda alta e estreita de (−85, −189, −300) a (−90, −194, −335): ~6 m de largura e ~30 m de altura. A parede LESTE é congelada (gelo, geada, cristais); a OESTE é derretida (rachaduras de magma). Neve e brasas caem juntas.\n\n"
          "Temperaturas: lado leste −6 °C, lado oeste +30 °C; o meio do corredor ~8 °C. Quem anda encostado numa parede sofre o efeito: gelo = 1d4 de frio por rodada (só ao tocar), magma = 1d6 de fogo.\n"
          "Chão em declive suave (8°) até uma descida que leva ao Labirinto Alienígena (L0 em (−92, −200, −345)). Tecla [N] não afeta aqui."), kSess, kGrp);
    w.addPoi(V3(-87.5f, -178.f, -317.f), PoiCategory::Creature, Q("3.5 · 8 Mephits de Gelo (facção leste)"),
        Q("8 Mephits de Gelo (MM, CR 1/2; PV 21): pequenos, translúcidos, com asas de cristal e olhos azuis; atraídos pela magia dissipadora do Alhoon (a Máquina do Frio). Voam no alto da fenda, a 8–25 m de altura, lançando Sopro de Gelo (cone 4,5 m, Des CD 10, 1d8 frio) e geada.\n\n"
          "Tática: em bandos de 3–4, descem em mergulho contra os mephits de magma (e contra quem estiver no caminho). Imunes a frio e veneno; vulneráveis a fogo (dano em dobro de qualquer fogo ou magma). Morte: explodem em estilhaços (1d4 cortante em 1,5 m).\n"
          "Manipular: ofereça-lhes calor (tocha acesa no chão, braseiro) e eles caem em fúria; uma chama grande os repele; lançar Cone de Frio em um magma os 'faz' amigos por 1 h."), kSess, kGrp);
    w.addPoi(V3(-88.5f, -178.f, -317.f), PoiCategory::Creature, Q("3.5 · 8 Mephits de Magma (facção oeste)"),
        Q("8 Mephits de Magma (MM, CR 1/2; PV 22): nascidos do calor residual do Dragão, pele de basalto com rachaduras brilhantes, asas de brasa. Sopro de Fogo (cone 4,5 m, Des CD 11, 2d6 fogo) e Explosão Final (1d4 fogo ao morrer, 1,5 m).\n\n"
          "Tática: protegem a parede oeste e atacam em duplas as 'asas de gelo'. Imunes a fogo e veneno; vulneráveis a frio. Se o grupo ataca um lado, o outro lado aproveita para atrapalhar.\n"
          "Manipular: Persuasão/Intimidação CD 13 em Língua Primordial (Ignan/Aquan) para fazer uma facção contra a outra; entregar um item gelado aos magma (ou quente aos gelo) os faz cúmplices."), kSess, kGrp);
    w.addPoi(V3(-87.f, -189.5f, -313.f), PoiCategory::Puzzle, Q("3.5 · Fogo cruzado (furtividade / manipulação)"),
        Q("Passar pelo meio da fenda sob a guerra elemental: Furtividade CD 13 (em grupo: o pior resultado) para não atrair a atenção; falha = 1d4 mephits de cada lado desviam do duelo e atacam o grupo, 2d6 frio/fogo.\n\n"
          "Alternativas: (1) Esperar o 'choque' (a cada ~7 s eles se chocam no centro — janela de 2 s para correr); (2) Manipular uma facção (aliar-se a uma contra a outra); (3) Fogo ou frio em área para dispersar (mas atrai ambas as facções); (4) Pedras jogadas para o lado leste/oeste como isca."), kSess, kGrp);
    w.addPoi(V3(-90.f, -192.5f, -334.f), PoiCategory::Area, Q("3.5 · Fim da Fenda — descida"),
        Q("Aqui a fenda se fecha numa rampa que desce a ~28° em curva suave para L0 do Labirinto Alienígena (−92, −200, −345). O chão passa de basalto escuro para uma pedra lisa demais, sem marcas. O zumbido psíquico começa: sussurros '…a matemática não mente…'.\n\n"
          "Daqui para a frente começa a Camada 3b: geometria alterada por intelecto alienígena (Sabedoria/Inteligência CD 14 ao entrar). Último ponto de descanso 'natural' antes do labirinto: o corredor do Tubo C."), kSess, kGrp);
}
