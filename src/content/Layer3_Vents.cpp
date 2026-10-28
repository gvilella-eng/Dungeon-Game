// CAMADA 3 — 3.3 Os Dutos de Respiro (passagem secreta): Tubo D (beco de magma), chamine escalavel de 16 m,
// duto de respiro por cima da cupula, tres "Olhos do Respiro" sobre o acampamento e a segunda chamine ate o Tubo C.
#include "Layer3_Parts.h"
#include "Prefabs.h"
#include <memory>

#define Q(s) QStringLiteral(s)
using namespace L23;
using namespace L3;

void L3::buildVents(World& w)
{
    const QString kSess = Q("Sessão 3 · Nível 7→8");
    const QString kGrp = Q("Camada 3 · As Veias Vítreas");
    using namespace Prefabs;
    Rnd rg(515);

    // ---- Tubo D: beco de magma colapsado ----
    {
        MeshBuilder mb;
        for (int i = 0; i < 14; ++i) {                       // entulho de magma solidificado
            Vec3 p(-39.f + rg.f(-4.f, 3.f), -186.f, -262.f + rg.f(-2.4f, 2.4f));
            float s = rg.f(0.2f, 0.7f);
            mb.ellipsoid(p + V3(0, s * 0.4f, 0), V3(s, s * 0.6f, s * 0.9f), QColor(30 + rg.i(0, 20), 22, 22), MAT_HOT_BASALT, 6);
        }
        for (int i = 0; i < 7; ++i) {                        // veios de magma brilhando entre as pedras
            Vec3 a(-36.5f + rg.f(-2.f, 1.f), -185.9f, -262.f + rg.f(-2.f, 2.f));
            mb.box(a, V3(rg.f(0.2f, 0.7f), 0.03f, 0.05f), QColor(255, 90, 20), MAT_LAVA);
        }
        bonesPile(mb, V3(-42.f, -186.f, -260.5f), 0.5f, 91);       // um lobo que tentou fugir
        skull(mb, V3(-41.2f, -186.f, -260.f), 70.f, 1.1f);
        w.addStatic(std::move(mb));
        w.addLight(V3(-34.5f, -184.2f, -262.8f), V3(1.0f, 0.3f, 0.08f), 4.5f, 9.f, LightAnim::Pulse, Q("magma do beco"))->animFreq = 0.4f;
        w.addEmitter(ParticlePreset::Embers, V3(-35.f, -184.f, -263.f), V3(0.8f, 0.2f, 0.8f), 5.f, Q("brasas do beco"));
        // corrente de ar subindo pela chamine (pista para Percepcao CD 15)
        EmitterDesc ed; ed.preset = ParticlePreset::Dust; ed.name = Q("corrente de ar da chamine"); ParticleEmitter::applyPreset(ed);
        ed.pos = V3(-36.f, -184.f, -263.f); ed.halfExtent = V3(0.9f, 0.3f, 0.9f); ed.rate = 14.f; ed.velocity = V3(0, 1.8f, 0); ed.velJitter = 0.2f;
        ed.colorStart = QColor(210, 200, 190, 90); ed.sizeMin = 0.03f; ed.sizeMax = 0.06f;
        w.addEmitter(ed);

        ZoneEffects fd;
        fd.areaName = Q("3.3 · Tubo D (beco de magma)"); fd.subtitle = Q("Passagem secreta · ar quente subindo"); fd.temperature = 19.f;
        fd.fogColor = V3(0.02f, 0.008f, 0.004f); fd.fogDensity = 0.022f; fd.ambientColor = V3(0.05f, 0.022f, 0.014f); fd.ambientGround = V3(0.03f, 0.012f, 0.008f);
        w.addZone(Q("tubo_d"), ZoneShape::makeBox(V3(-57.f, -188.f, -268.f), V3(-32.f, -178.f, -256.f)), fd, 5);
        ZoneEffects cl; cl.climb = true;
        w.addZone(Q("chamine_1_escalada"), ZoneShape::makeBox(V3(-38.6f, -185.8f, -265.6f), V3(-33.4f, -170.f, -260.4f)), cl, 7);
        ZoneEffects fc;
        fc.areaName = Q("3.3 · Dutos de Respiro"); fc.subtitle = Q("Rastejar sobre a cúpula · 1,3 m de diâmetro"); fc.temperature = 13.f;
        fc.fogColor = V3(0.012f, 0.012f, 0.02f); fc.fogDensity = 0.03f;
        fc.ambientColor = V3(0.02f, 0.028f, 0.05f);
        w.addZone(Q("duto_respiro"), ZoneShape::makeBox(V3(-100.f, -167.f, -285.f), V3(-33.f, -159.f, -248.f)), fc, 5);
        w.addZone(Q("chamine_2_escalada"), ZoneShape::makeBox(V3(-94.8f, -188.f, -285.8f), V3(-89.2f, -164.f, -280.2f)), cl, 7);
        ZoneEffects fk; fk.areaName = Q("3.3 · Chaminé até o Tubo C"); fk.subtitle = Q("Descida de 23 m"); fk.temperature = 11.f;
        w.addZone(Q("chamine_2_area"), ZoneShape::makeBox(V3(-97.f, -190.f, -288.f), V3(-86.f, -176.f, -278.f)), fk, 4);
    }

    // ---- Olhos do Respiro: luz azul subindo do acampamento, vapor frio e sussurros ----
    static const float ex[3] = {-62.f, -75.f, -86.f}, ez[3] = {-257.f, -255.f, -252.f};
    for (int i = 0; i < 3; ++i) {
        float cy = domeCeilingY(ex[i], ez[i]);
        w.addBeam(Q("olho_respiro_%1").arg(i + 1), V3(ex[i], cy - 0.6f, ez[i]), V3(ex[i], -163.2f, ez[i]), QColor(90, 150, 255, 70), 0.32f);
        w.addLight(V3(ex[i], -162.3f, ez[i]), V3(0.3f, 0.55f, 1.0f), 2.6f, 6.f, LightAnim::Pulse, Q("luz do olho do respiro %1").arg(i + 1))->animFreq = 0.8f;
        EmitterDesc ed; ed.preset = ParticlePreset::Dust; ed.name = Q("poeira do olho %1").arg(i + 1); ParticleEmitter::applyPreset(ed);
        ed.pos = V3(ex[i], -164.f, ez[i]); ed.halfExtent = V3(0.25f, 0.5f, 0.25f); ed.rate = 6.f; ed.velocity = V3(0, 0.6f, 0); ed.colorStart = QColor(140, 180, 255, 90);
        w.addEmitter(ed);
    }
    {   // sussurros de Kaelen ao se debruçar sobre um olho (uma vez por olho)
        auto seen = std::make_shared<std::vector<bool>>(3, false);
        static const char* L[3] = {
            "Lá embaixo, Kaelen murmura sozinho: \"Sim… eu sei. Mais um, Espírito. O menino não vai durar a semana.\"",
            "Kaelen, no pódio, conversa com a chama azul: \"A Máquina do Frio precisa de você… eu carrego o peso.\" Os cães rosnam baixinho.",
            "Você ouve Kaelen rindo e chorando ao mesmo tempo: \"O fogo exige combustível… o fogo exige… a matemática não mente.\""};
        w.addUpdater([&w, seen](float, float) {
            Vec3 p = w.playerPos();
            if (p.y() < -167.f || p.y() > -160.f) return;
            for (int i = 0; i < 3; ++i) {
                if ((*seen)[i]) continue;
                if (std::hypot(p.x() - ex[i], p.z() - ez[i]) < 1.6f) {
                    (*seen)[i] = true;
                    w.state.toast(QString::fromUtf8(L[i]), QColor(170, 200, 255), 7.f);
                }
            }
        });
    }

    // ---- ossos e restos no duto + um respiro quebrado ----
    {
        MeshBuilder mb;
        bonesPile(mb, V3(-95.f, -165.f, -270.f), 0.4f, 93);
        skull(mb, V3(-94.5f, -165.f, -268.5f), 20.f, 0.5f);
        mb.cylinder(V3(-36.f, -170.f, -263.f), V3(-36.f, -170.f, -263.f) + V3(0, 0.05f, 0), 0.01f, 3, QColor(0, 0, 0), MAT_COAL);
        w.addStatic(std::move(mb));
    }

    // ---- POIs ----
    w.addPoi(V3(-46.f, -184.4f, -260.2f), PoiCategory::Secret, Q("3.3 · Tubo D e o beco de magma colapsado"),
        Q("Tubo D sai da cúpula para o leste: (−53, −186, −258) → (−38, −186, −262) e termina num beco de magma colapsado (rocha negra e vermelha, brilho fraco entre as pedras).\n\n"
          "Perceba: um fiapo de ar quente subindo no fundo do beco; poeira dançando no ar. Percepção CD 15 para notar a corrente de ar e ver que ela vem de uma chaminé vertical estreita (1,9 m de raio) logo acima do desmoronamento.\n"
          "Mesmo com o desmoronamento, o beco é largo o bastante para guardar uma mochila ou prender uma montaria. É o desvio perfeito para contornar a fortaleza: os cães não farejam o ar quente."), kSess, kGrp);
    w.addPoi(V3(-36.f, -178.f, -263.f), PoiCategory::Secret, Q("3.3 · Chaminé vertical (16 m, escalável)"),
        Q("Poço natural de (−36, −186, −263) até (−36, −170, −263): 16 m de rocha vulcânica áspera, de 3,8 m de diâmetro, com apoios fartos.\n\n"
          "Atletismo CD 12 para escalar (CD 10 com corda; cada falha: queda de até 16 m, 5d6). Subir leva ~4 rodadas. Calor residual de 19 °C no fundo; ar frio e seco no topo.\n"
          "No alto: câmara de 6 m e a boca do duto de respiro. Dica: quem sobe sem luz precisa de visão no escuro; a tocha pode apagar com a corrente de ar (nada a ver com o poço)."), kSess, kGrp);
    w.addPoi(V3(-60.f, -163.2f, -257.f), PoiCategory::Secret, Q("3.3 · Duto de respiro (sobre a cúpula)"),
        Q("Duto de ~1,3–1,7 m de diâmetro (só se rasteja: Médios andam a meio deslocamento; Pequenos cabem em pé encurvados) que corre para oeste POR CIMA da cúpula: (−36, −170, −263) → (−55, −164, −258) → (−75, −163, −255) → (−95, −165, −275). Obsidiana negra com brilho azul-pálido vindo dos Olhos do Respiro.\n\n"
          "Perceba: o eco da voz de Kaelen ('Sim, Espírito…') e latidos abafados. Barulho: Furtividade CD 12 para não ecoar (armadura pesada: desvantagem). Sem luz, a chama da tocha atrai os cães por reflexo na obsidiana.\n"
          "É também o desvio que contorna a fortaleza: leva ao Tubo C sem passar pelo acampamento."), kSess, kGrp);
    const char* oe[3] = {"Olho do Respiro #1 (oeste da mesa)", "Olho do Respiro #2 (sobre o braseiro)", "Olho do Respiro #3 (sobre as jaulas)"};
    const char* ot[3] = {
        "Buraco vertical de ~1,1 m de diâmetro e ~6 m de comprimento entre o duto e o teto da cúpula (em (−62, ·, −257)). Dá para ver o pátio e o portão; é o melhor ponto para observar a patrulha dos cães.",
        "Buraco vertical (−75, ·, −255) logo acima do braseiro de chama fria; luz azul sobe como um poço de gelo. Vê-se o pódio de Kaelen e as 4 jaulas. Quem cai por aqui cai ~20 m (6d6) no centro do acampamento, a 2 m do braseiro.",
        "Buraco vertical (−86, ·, −252) sobre o lado oeste, a ~2 m das jaulas. Dá para descer uma corda e alcançar uma das jaulas (Atletismo CD 12) ou cair sobre as jaulas de ferro (a 17 m). Ideal para um resgate em silêncio."};
    for (int i = 0; i < 3; ++i)
        w.addPoi(V3(ex[i], -163.0f, ez[i]), PoiCategory::Secret, QString::fromUtf8(oe[i]),
            QString::fromUtf8(ot[i]) + Q("\n\nRegras: Percepção CD 12 para ouvir Kaelen; ouvir os delírios dele revela que ele conversa com 'o Espírito' e menciona a Máquina do Frio. Dar uma olhada exige luz apagada (a luz do acampamento é azul e vem de baixo — a tocha denuncia). Cuidado com o buraco: Destreza CD 10 ao rastejar sobre ele ou cai."), kSess, kGrp);
    w.addPoi(V3(-92.f, -176.f, -283.f), PoiCategory::Secret, Q("3.3 · Segunda chaminé até o Tubo C"),
        Q("Do fim do duto de respiro (−95, −165, −275) desce uma segunda chaminé (raio 1,6 m, 23 m de profundidade) até um pequeno túnel que desemboca no Tubo C em (−85, −190, −288): o grupo reaparece ATRÁS de Kaelen, sem ter passado pela paliçada.\n\n"
          "Atletismo CD 12 (descida: CD 10). Queda: 8d6. Uma corda amarrada no topo dá vantagem. Em baixo, os rastros de arrasto de Kaelen cruzam o tubo (indicando que ele usa o Tubo C para levar os sequestrados à Máquina do Frio)."), kSess, kGrp);
}
