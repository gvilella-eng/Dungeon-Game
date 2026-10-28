// CAMADA 3 — 3.4 A Passagem dos Fungos da Memoria: Tubo B, fungos bioluminescentes, esporos, Miconidas,
// zona de alucinacao (a rocha vira o interior vulcanico de um dragao) e o Bosque Miconida.
#include "Layer3_Parts.h"
#include "Prefabs.h"
#include <memory>

#define Q(s) QStringLiteral(s)
using namespace L23;
using namespace L3;

namespace {

void myconid(MeshBuilder& mb, const Vec3& pos, float yaw, float s, bool sovereign)
{
    mb.push(); mb.translate(pos); mb.rotateY(yaw); mb.scale(s);
    QColor flesh(196, 186, 156), fleshD(150, 140, 118), cap = sovereign ? QColor(120, 255, 200) : QColor(70, 220, 190);
    for (int k = -1; k <= 1; k += 2) {
        mb.cylinder(V3(k * 0.15f, 0, 0), V3(k * 0.13f, 0.58f, 0), 0.11f, 0.09f, 7, fleshD, MAT_FLESH);
        mb.ellipsoid(V3(k * 0.15f, 0.04f, -0.05f), V3(0.13f, 0.05f, 0.17f), fleshD, MAT_FLESH, 6);
    }
    mb.ellipsoid(V3(0, 0.98f, 0), V3(0.36f, 0.5f, 0.32f), flesh, MAT_FLESH, 9);
    for (int k = -1; k <= 1; k += 2) {
        mb.capsule(V3(k * 0.33f, 1.3f, 0), V3(k * 0.52f, 0.9f, -0.22f), 0.075f, flesh, MAT_FLESH, 6);
        mb.sphere(V3(k * 0.52f, 0.88f, -0.24f), 0.085f, fleshD, MAT_FLESH, 6);
    }
    mb.ellipsoid(V3(0, 1.62f, -0.02f), V3(0.2f, 0.2f, 0.19f), flesh, MAT_FLESH, 8);                  // cabeca
    for (int k = -1; k <= 1; k += 2) mb.sphere(V3(k * 0.085f, 1.64f, -0.19f), 0.03f, QColor(15, 25, 20), MAT_COAL, 6);
    mb.box(V3(0, 1.53f, -0.2f), V3(0.05f, 0.012f, 0.01f), QColor(60, 30, 30), MAT_FLESH);
    mb.cylinder(V3(0, 1.72f, 0), V3(0, 1.8f, 0), 0.5f, 0.5f, 10, QColor(30, 110, 100), MAT_FUNGUS);   // lamelas
    mb.ellipsoid(V3(0, 1.9f, 0), V3(0.52f, 0.28f, 0.52f), cap, MAT_FUNGUS, 9);                         // chapeu
    for (int i = 0; i < 6; ++i) {
        float a = i * 1.047f + 0.4f;
        mb.sphere(V3(std::cos(a) * 0.3f, 2.08f - std::fabs(std::sin(a * 1.7f)) * 0.06f, std::sin(a) * 0.3f), 0.045f, QColor(200, 255, 235), MAT_EMISSIVE, 5);
    }
    if (sovereign) {
        mb.ellipsoid(V3(0.0f, 2.22f, 0.f), V3(0.3f, 0.2f, 0.3f), QColor(160, 255, 220), MAT_FUNGUS, 8);
        mb.ellipsoid(V3(0.0f, 2.4f, 0.f), V3(0.16f, 0.13f, 0.16f), QColor(210, 255, 240), MAT_FUNGUS, 7);
        for (int i = 0; i < 5; ++i) {                                                                  // tendrilhos luminosos
            float a = i * 1.2566f;
            std::vector<Vec3> p = {V3(std::cos(a) * 0.3f, 1.9f, std::sin(a) * 0.3f), V3(std::cos(a) * 0.7f, 1.5f, std::sin(a) * 0.7f), V3(std::cos(a) * 0.85f, 0.9f, std::sin(a) * 0.85f)};
            mb.tube(p, std::vector<float>{0.03f, 0.025f, 0.012f}, 5, QColor(130, 255, 215), MAT_FUNGUS);
        }
    }
    mb.pop();
}

} // namespace

void L3::buildFungus(World& w)
{
    const QString kSess = Q("Sessão 3 · Nível 7→8");
    const QString kGrp = Q("Camada 3 · As Veias Vítreas");
    using namespace Prefabs;
    Rnd rg(808);
    const Vec3 GC(-145.f, -190.f, -265.f);          // centro do bosque (chao)

    // ---- fungos por toda parte: raios a partir do eixo do Tubo B e do bosque ----
    {
        MeshBuilder mb;
        const QColor caps[5] = {QColor(80, 255, 200), QColor(60, 220, 255), QColor(120, 255, 140), QColor(90, 255, 230), QColor(160, 150, 255)};
        auto rotTo = [&](const Vec3& n) { return QQuaternion::rotationTo(Vec3(0, 1, 0), n.normalized()); };
        auto put = [&](const Vec3& origin, float reach, int idx, float hMin, float hMax) {
            Vec3 d(rg.f(-1.f, 1.f), rg.f(-1.f, 0.7f), rg.f(-1.f, 1.f));
            if (d.lengthSquared() < 0.05f) return;
            d.normalize();
            RayHit h = w.raycast(origin, d, reach, false);
            if (!h.hit) return;
            mb.push(); mb.translate(h.pos); mb.rotate(rotTo(h.normal));
            float s = rg.f(hMin, hMax);
            if (rg.f() < 0.18f) mb.ellipsoid(V3(0, 0.02f, 0), V3(rg.f(0.12f, 0.36f), 0.03f, rg.f(0.1f, 0.3f)), QColor(40, 200 + rg.i(0, 40), 150), MAT_FUNGUS, 7);   // esteira de fungo
            else mushroom(mb, V3(0, -0.02f, 0), s, caps[idx % 5], true);
            mb.pop();
        };
        // Tubo B: polilinha de pisos
        const Vec3 B[4] = {V3(-95.f, -186.f, -255.f), V3(-115.f, -188.f, -250.f), V3(-132.f, -189.f, -258.f), V3(-141.f, -190.f, -263.f)};
        for (int i = 0; i < 230; ++i) {
            int sgm = rg.i(0, 2); float f = rg.f();
            Vec3 o = B[sgm] + (B[sgm + 1] - B[sgm]) * f + V3(0, 1.8f, 0);
            put(o, 6.f, i, 0.25f, 0.85f);
        }
        for (int i = 0; i < 300; ++i) {                      // bosque
            Vec3 o = GC + V3(rg.f(-6.f, 6.f), rg.f(0.8f, 4.5f), rg.f(-6.f, 6.f));
            put(o, 8.f, i, 0.3f, 1.2f);
        }
        // grandes cogumelos-arvore no chao do bosque (longe da entrada, a leste)
        {
            const float ga[6] = {60.f, 115.f, 170.f, 225.f, 280.f, 322.f};
            for (int i = 0; i < 6; ++i) {
                float a = rad(ga[i]), r = rg.f(6.2f, 8.2f);
                Vec3 p(GC.x() + r * std::cos(a), -190.f, GC.z() + r * std::sin(a));
                mushroom(mb, p, rg.f(1.8f, 3.0f), caps[i % 5], true);
            }
        }
        w.addStatic(std::move(mb));
    }

    // ---- Miconidas (isolados e pacificos) ----
    struct MP { Vec3 p; float yaw; float s; };
    const MP mc[] = {{V3(-141.f, -190.f, -259.f), 120.f, 1.0f}, {V3(-150.f, -190.f, -259.5f), 20.f, 0.9f}, {V3(-152.f, -190.f, -267.f), -80.f, 1.05f},
                     {V3(-148.f, -190.f, -272.f), 10.f, 0.95f}, {V3(-140.f, -190.f, -270.f), 70.f, 1.1f}, {V3(-137.5f, -190.f, -265.5f), 90.f, 0.85f},
                     {V3(-146.f, -190.f, -258.f), 180.f, 0.9f}};
    {
        MeshBuilder mb;
        for (auto& m : mc) myconid(mb, m.p, m.yaw, m.s, false);
        myconid(mb, V3(-146.f, -190.f, -268.5f), 0.f, 1.5f, true);
        w.addStatic(std::move(mb));
    }

    // ---- Esporo do Sonho Antigo (interativo) ----
    DynHandle pod;
    {
        MeshBuilder pm;
        pm.ellipsoid(V3(0, 0.15f, 0), V3(0.35f, 0.15f, 0.35f), QColor(60, 70, 66), MAT_LIMESTONE, 7);
        pm.sphere(V3(0, 0.45f, 0), 0.2f, QColor(160, 255, 235), MAT_FUNGUS, 10);
        pm.sphere(V3(0, 0.45f, 0), 0.11f, QColor(255, 255, 255), MAT_EMISSIVE, 8);
        for (int i = 0; i < 6; ++i) {
            float a = i * 1.047f;
            pm.sphere(V3(std::cos(a) * 0.28f, 0.45f + std::sin(a * 2.f) * 0.12f, std::sin(a) * 0.28f), 0.05f, QColor(120, 255, 200), MAT_EMISSIVE, 5);
        }
        pod = w.addDynamic(Q("esporo_sonho"), std::move(pm), V3(GC.x(), -190.f, GC.z()));
        w.addUpdater([pod](float, float t) { pod->pos.setY(-190.f + 0.05f * std::sin(t * 1.5f)); });
    }
    Light* podLight = w.addLight(V3(GC.x(), -189.2f, GC.z()), V3(0.4f, 1.0f, 0.8f), 4.f, 9.f, LightAnim::Pulse, Q("esporo do sonho"));
    podLight->animFreq = 0.4f;
    w.addInteractable(Q("esporo_sonho"), V3(GC.x(), -189.5f, GC.z()), 1.0f, Q("Coletar o Esporo do Sonho Antigo"), [&w, pod, podLight]() {
        if (!pod->visible) return;
        pod->visible = false; podLight->enabled = false;
        w.state.addItem(Q("Esporo do Sonho Antigo"));
        w.state.toast(Q("O esporo pulsa como um coração pequeno. Cheira a fumaça e a cinzas de dragão."), QColor(150, 255, 220), 5.f);
        w.state.flash(QColor(120, 255, 200), 0.3f);
    });

    // ---- luzes e esporos ----
    {
        const float lx[4] = {-102.f, -114.f, -124.f, -134.f}, ly[4] = {-186.f, -187.f, -187.5f, -188.f}, lz[4] = {-253.f, -250.f, -253.f, -260.f};
        for (int i = 0; i < 4; ++i)
            w.addLight(V3(lx[i], ly[i] + 1.4f, lz[i]), V3(0.2f, 1.0f, 0.7f), 3.2f, 13.f, LightAnim::Pulse, Q("fungos do tubo B"))->animFreq = 0.35f + 0.1f * i;
        for (int i = 0; i < 6; ++i) {
            float a = i * 1.047f;
            w.addLight(GC + V3(std::cos(a) * 5.5f, 1.8f, std::sin(a) * 5.5f), i % 2 ? V3(0.2f, 0.9f, 1.f) : V3(0.3f, 1.0f, 0.6f), 3.4f, 11.f, LightAnim::Pulse, Q("fungos do bosque"))->animFreq = 0.3f + 0.07f * i;
        }
        w.addLight(GC + V3(0, 3.f, 0), V3(0.3f, 1.0f, 0.8f), 3.f, 16.f, LightAnim::Pulse, Q("esporos"))->animFreq = 0.2f;
        for (int i = 0; i < 4; ++i)
            w.addEmitter(ParticlePreset::Spores, V3(-100.f - i * 11.f, -186.f - i * 0.8f, -253.f + (i % 2 ? 1.f : -2.f) - i * 1.5f), V3(3.2f, 1.8f, 2.6f), 16.f, Q("esporos do tubo B"));
        for (int i = 0; i < 3; ++i)
            w.addEmitter(ParticlePreset::Spores, GC + V3(rg.f(-3.f, 3.f), 2.f, rg.f(-3.f, 3.f)), V3(4.f, 2.f, 4.f), 28.f, Q("esporos do bosque"));
    }

    // ---- zonas ----
    {
        ZoneEffects fb;
        fb.areaName = Q("3.4 · A Passagem dos Fungos da Memória"); fb.subtitle = Q("Tubo B · fungos bioluminescentes"); fb.temperature = 9.f;
        fb.fogColor = V3(0.008f, 0.03f, 0.024f); fb.fogDensity = 0.024f;
        fb.ambientColor = V3(0.02f, 0.07f, 0.06f); fb.ambientGround = V3(0.01f, 0.04f, 0.035f);
        w.addZone(Q("tubo_b"), ZoneShape::makeBox(V3(-142.f, -192.f, -266.f), V3(-94.f, -182.f, -244.f)), fb, 4);
        ZoneEffects fh; fh.hallucination = 0.35f;
        w.addZone(Q("esporos_leves"), ZoneShape::makeSphere(V3(-130.f, -188.f, -257.f), 12.f), fh, 5);
        ZoneEffects fg;
        fg.areaName = Q("3.4 · Bosque Micônida"); fg.subtitle = Q("Esporos: a rocha vira o interior de um dragão"); fg.hallucination = 0.9f; fg.temperature = 11.f;
        fg.fogColor = V3(0.01f, 0.05f, 0.04f); fg.fogDensity = 0.03f;
        fg.ambientColor = V3(0.03f, 0.1f, 0.08f); fg.ambientGround = V3(0.015f, 0.05f, 0.04f);
        w.addZone(Q("bosque_miconida"), ZoneShape::makeSphere(V3(GC.x(), -187.5f, GC.z()), 11.5f), fg, 6);
    }

    // ---- POIs ----
    w.addPoi(V3(-112.f, -186.2f, -250.5f), PoiCategory::Area, Q("3.4 · Tubo B (Passagem dos Fungos da Memória)"),
        Q("Tubo B para oeste: (−97, −186, −255) → (−115, −188, −250) → (−132, −189, −258) → Bosque Micônida (raio 10 em (−145, −190, −265)). Mesmo tubo de obsidiana, agora coberto por fungos bioluminescentes ciano-esverdeados que pulsam em ondas lentas; nuvens de esporos flutuando na luz.\n\n"
          "Perceba: cheiro doce e úmido (cogumelo, terra), calor suave (9 °C) e um zumbido musical quase imperceptível. A tocha pode ser apagada: a luz dos fungos basta para andar.\n"
          "Percepção CD 12: os fungos formam um 'caminho' leve até o bosque. O tubo é largo o bastante para fugir dos cães de Kaelen (que não gostam dos esporos)."), kSess, kGrp);
    w.addPoi(V3(-130.f, -187.5f, -257.f), PoiCategory::Trap, Q("3.4 · Nuvens de esporos (alucinação)"),
        Q("Respirar os esporos faz os jogadores enxergarem a caverna como era há milênios: paredes de carne, veias de fogo pulsando — o INTERIOR VULCÂNICO E PULSANTE de um dragão titânico. Entrega a lore sem NPCs (no jogo, é o efeito de alucinação do motor: a rocha vira carne pulsante).\n\n"
          "Regras: Constituição CD 13 ao entrar no Bosque ou fica Envenenado 1 h, com visões (desvantagem em testes de ataque/Sabedoria; vê coisas que não estão lá). Sucesso: só um fio de visões leves por 1 min. Máscara ou pano molhado: vantagem.\n"
          "Narração sugerida: 'Uma muralha de carne vermelha respira ao redor; a luz é o calor de uma fornalha; ouvem um batimento lento, enorme, vindo de baixo. Entendem que o Coração do Dragão ainda bate, e que algo — a Máquina — o está drenando.'"), kSess, kGrp);
    w.addPoi(V3(GC.x(), -187.5f, GC.z() + 1.5f), PoiCategory::Area, Q("3.4 · O Bosque Micônida"),
        Q("Câmara de raio 10 em (−145, −190, −265): chão forrado de fungos, 'cogumelos-árvore' de 2–3 m, lamelas luminosas e nuvens de esporos. 6–8 Micônidas pacíficos e isolados vivem aqui, colhendo calor do Coração que sobe das rochas.\n\n"
          "Descanso: relativamente seguro (os Micônidas só afastam quem os ameaça), mas a alucinação atrapalha o descanso longo (Sabedoria CD 12 ou nenhum benefício).\n"
          "O Bosque é o último respiro antes do domínio psíquico do Alhoon. Os fungos são uma colônia única, conectada por rede micelial: 'lembram' tudo o que passa por aqui — inclusive Kaelen (que jamais entrou)."), kSess, kGrp);
    w.addPoi(V3(-146.f, -188.5f, -268.5f), PoiCategory::Creature, Q("3.4 · O Soberano Micônida"),
        Q("Micônida maior (escala 1,5) com coroa luminosa e tendrilhos dourado-esverdeados. Fala por Rapport de Esporos: mostra a MEMÓRIA do Dragão (calor, vôo, a queda, o ressurgimento do Coração) em vez de palavras.\n\n"
          "Micônida Soberano (MM, CR 2 para Micônida Soberano de 5ª edição: 'Myconid Sovereign'): Rapport de Esporos (Sab CD 12: aliados veem a memória por 10 min; podem receber uma pista sobre a Máquina do Frio), Esporos Alucinógenos (CD 12 Con), Esporos Calmantes (para acalmar uma criatura hostil). Ele pede em troca: 'não queimem a Montanha'.\n"
          "Pode ser usado como informação do Mestre: revela que o Coração do Dragão ainda bate (o dragão está ressuscitando), e que algo gelado devora o calor. Hostil só se atacado; os outros Micônidas protegem."), kSess, kGrp);
    for (int i = 0; i < 3; ++i)
        w.addPoi(mc[i * 2 % 7].p + V3(0, 2.4f, 0), PoiCategory::Creature, Q("3.4 · Micônida pacífico %1").arg(i + 1),
            Q("Micônida adulto (MM, CR 1/2): humanoide-fungo isolado, de pele pálida e chapéu ciano luminoso, ~1,7 m. Não ataca; encara o grupo, emite um fio de esporos calmantes e volta a 'pastar' calor do chão.\n\nRegras: Empatia com Plantas/Natureza CD 10 permite uma conversa de Rapport; um Micônida ferido libera esporos (Con CD 11 ou Envenenado)."), kSess, kGrp);
    w.addPoi(V3(GC.x(), -189.f, GC.z()), PoiCategory::Treasure, Q("3.4 · Esporo do Sonho Antigo"),
        Q("Casulo de fungo translúcido do tamanho de um punho, pulsando em ciano-branco, cultivado no centro do Bosque pelos Micônidas ao longo de séculos. Contém uma visão extra a critério do Mestre (o que o Dragão viu antes de morrer, uma pista sobre o ponto fraco do Alhoon, a localização de um segredo na Camada 4...).\n\n"
          "Mecânica sugerida: usar (ação, descanso curto) concede uma visão de 1 min (inspiração) e um uso de Orientação gratuito; consumir o esporo cura 2d6 PV. Quebrar o esporo sem cuidado: nuvem de esporos (Con CD 13). Local: (−145, −190, −265). [E] para coletar."), kSess, kGrp);
    w.addPoi(V3(-136.f, -187.f, -262.f), PoiCategory::Lore, Q("3.4 · A Memória do Dragão (lore)"),
        Q("Se o grupo usar o Rapport de Esporos ou ficar alucinado, narre: o Dragão de Fogo (um colosso rubro com asas de fornalha) mergulhou na montanha para morrer; seu Coração, entretanto, não parou — 'incuba'. Há 1.000 anos os Micônidas o escutam.\n"
          "Fatos revelados: o calor da vila vem do Coração; algo o drena (a visão mostra um disco negro girando no alto da caverna: o Dissipador do Vazio); a 'voz' que Kaelen ouve não é o Dragão — é uma mente fria (olhos brancos, tentáculos). Isso prepara a Camada 5."), kSess, kGrp);
}
