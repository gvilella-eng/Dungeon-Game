// CAMADA 3 — 3.1 Tubo A (props) e 3.2 O Entroncamento de Kaelen: palicada, jaulas, posto de comando,
// Kaelen, hell hounds corrompidos, braseiro de chama fria, bau com o Anel de Aquecimento.
#include "Layer3_Parts.h"
#include "Prefabs.h"
#include <memory>

#define Q(s) QStringLiteral(s)
using namespace L23;
using namespace L3;

namespace {

struct CampState {
    DynHandle kaelen = nullptr, hound[2] = {nullptr, nullptr};
    Light* kLight = nullptr; Light* hLight[2] = {nullptr, nullptr};
    DynHandle flame = nullptr;
    float kYaw = 0, hYaw[2] = {0, 0};
    float kBase = 0, hBase[2] = {0, 0};
    bool alert = false;
    float t = 0.f; int line = 0;
    DynHandle chestShut = nullptr, chestOpen = nullptr;
    bool chestOpened = false;
};

float turnToward(float cur, float target, float maxStep)
{
    float d = std::fmod(target - cur + 540.f, 360.f) - 180.f;
    return cur + clampf(d, -maxStep, maxStep);
}

// folha de portao aberta (rustica)
void gateLeaf(MeshBuilder& mb, const Vec3& hinge, float yaw)
{
    mb.push(); mb.translate(hinge); mb.rotateY(yaw);
    for (int i = 0; i < 6; ++i) {
        float x = 0.2f + i * 0.34f;
        mb.cylinder(V3(x, 0.f, 0), V3(x + 0.02f, 3.2f + (i % 3) * 0.35f, 0), 0.15f, 0.12f, 6, QColor(88 + i * 3, 70, 52), MAT_ROTTEN_WOOD);
    }
    mb.box(V3(1.05f, 1.0f, 0.2f), V3(1.05f, 0.07f, 0.04f), QColor(60, 50, 40), MAT_ROTTEN_WOOD);
    mb.box(V3(1.05f, 2.3f, 0.2f), V3(1.05f, 0.07f, 0.04f), QColor(60, 50, 40), MAT_ROTTEN_WOOD);
    mb.pop();
}

} // namespace

void L3::buildKaelenCamp(World& w)
{
    const QString kSess = Q("Sessão 3 · Nível 7→8");
    const QString kGrp = Q("Camada 3 · As Veias Vítreas");
    const float fy = kDomeFloor;
    Rnd rg(333);
    using namespace Prefabs;
    auto polar = [&](float r, float aDeg, float y = 0.f) { float a = rad(aDeg); return Vec3(DC.x() + r * std::cos(a), y, DC.z() + r * std::sin(a)); };
    const float gateS = 75.f, gateN = 262.f;                    // angulos dos portoes (graus)
    auto inGate = [&](float aDeg, float half) {
        auto dd = [](float a, float b) { return std::fabs(std::fmod(a - b + 540.f, 360.f) - 180.f); };
        return dd(aDeg, gateS) < half || dd(aDeg, gateN) < half;
    };

    // ====================================================== 3.1 Tubo A: marcas de arrasto, correntes, sapato, alarme
    {
        MeshBuilder mb;
        const Vec3 pts[4] = {V3(-30.f, -178.f, -186.5f), V3(-48.f, -180.f, -196.f), V3(-62.f, -182.f, -220.f), V3(-69.f, -185.2f, -233.f)};
        auto onPath = [&](float s, float lat) {          // s em [0,3)
            int i = std::min(2, (int)s); float f = s - i;
            Vec3 p = pts[i] + (pts[i + 1] - pts[i]) * f;
            Vec3 d = (pts[i + 1] - pts[i]); d.setY(0); d.normalize();
            Vec3 n(-d.z(), 0, d.x());
            p += n * lat;
            p.setY(groundY(w, p.x(), p.y(), p.z()));
            return p;
        };
        for (int i = 0; i < 46; ++i) {                    // dois sulcos de arrasto
            float s = 0.25f + i * 0.058f;
            for (int k = -1; k <= 1; k += 2) {
                Vec3 a = onPath(s, 0.28f * k + rg.f(-0.04f, 0.04f)), b = onPath(s + 0.03f, 0.28f * k + rg.f(-0.04f, 0.04f));
                mb.push(); mb.translate((a + b) * 0.5f + V3(0, 0.016f, 0)); mb.rotateY(yawToward(a, b));
                mb.box(V3(0, 0, 0), V3(0.05f, 0.012f, 0.55f), QColor(12, 11, 14), MAT_COAL);
                mb.pop();
            }
        }
        for (int i = 0; i < 8; ++i) {                      // pegadas de botas (do captor)
            Vec3 p = onPath(0.6f + i * 0.3f, (i % 2 ? 0.9f : -0.9f));
            mb.push(); mb.translate(p + V3(0, 0.012f, 0)); mb.rotateY(yawToward(p, onPath(0.9f + i * 0.3f, 0.f)));
            mb.box(V3(0, 0, 0), V3(0.13f, 0.01f, 0.32f), QColor(10, 10, 12), MAT_COAL);
            mb.pop();
        }
        for (int i = 0; i < 3; ++i) {                      // correntes cortadas
            Vec3 a = onPath(0.9f + i * 0.25f, 1.0f - i * 0.8f), b = a + V3(rg.f(-1.2f, 1.2f), 0, rg.f(-1.2f, 1.2f));
            b.setY(groundY(w, b.x(), a.y(), b.z()));
            chain(mb, a + V3(0, 0.03f, 0), b + V3(0, 0.03f, 0), 0.04f);
        }
        Vec3 sh = onPath(0.62f, -0.5f);                    // sapato infantil
        mb.push(); mb.translate(sh); mb.rotateY(40.f);
        mb.box(V3(0, 0.035f, 0), V3(0.045f, 0.035f, 0.1f), QColor(96, 66, 44), MAT_HIDE);
        mb.box(V3(0, 0.008f, 0), V3(0.048f, 0.008f, 0.105f), QColor(40, 30, 24), MAT_HIDE);
        mb.box(V3(0, 0.07f, 0.04f), V3(0.04f, 0.015f, 0.04f), QColor(70, 46, 32), MAT_HIDE);
        mb.pop();
        {   // fio de alarme com latas (a ~8 m do portao)
            Vec3 c = onPath(2.62f, 0.f); Vec3 axisD = (pts[3] - pts[2]); axisD.setY(0); axisD.normalize();
            Vec3 n(-axisD.z(), 0, axisD.x());
            Vec3 o = c + V3(0, 0.45f, 0);
            RayHit hl = w.raycast(o, n, 8.f, false), hr = w.raycast(o, -n, 8.f, false);
            Vec3 pl = hl.hit ? hl.pos : o + n * 3.5f, pr = hr.hit ? hr.pos : o - n * 3.5f;
            std::vector<Vec3> wire = MeshBuilder::catenary(pl, pr, 0.05f, 10);
            mb.tube(wire, 0.008f, 4, QColor(110, 105, 100), MAT_IRON);
            for (int i = 1; i < 6; ++i) {
                Vec3 p = pl + (pr - pl) * (i / 6.f) + V3(0, -0.06f, 0);
                mb.cylinder(p + V3(0, -0.1f, 0), p + V3(0, 0.02f, 0), 0.04f, 7, QColor(120, 112, 100), MAT_RUSTY_IRON);
            }
            w.addPoi(c + V3(0, 1.2f, 0), PoiCategory::Trap, Q("3.1 · Fio de alarme de Kaelen"),
                Q("Arame esticado a ~0,45 m do chão entre as paredes do Tubo A, com latas de ferro penduradas — a ~8 m do portão do entroncamento.\n\n"
                  "Percepção CD 13 (a passiva 13 nota o brilho do arame na luz da tocha). Se alguém tropeçar: o chocalho ecoa pelo tubo vítreo e os hell hounds latem no canil — Kaelen sabe que alguém chega e quantos. Desarmar: Ladrões CD 10 ou passar por cima (Acrobacia CD 8).\n"
                  "Efeito: surpresa anulada para o grupo; Kaelen manda os cães para o portão e fica no pódio. Mostra o método dele — um caçador, não um sacerdote."), kSess, kGrp);
        }
        w.addStatic(std::move(mb));
        w.addPoi(sh + V3(0, 0.9f, 0), PoiCategory::Lore, Q("3.1 · Sapato infantil e marcas de arrasto"),
            Q("Dois sulcos paralelos no chão de cinza petrificada, intercalados com pegadas de botas grandes (um homem de ~1,90 m, passo longo). Um sapatinho de couro de criança jaz de lado, com o cadarço rompido.\n\n"
              "Sobrevivência (rastreamento) CD 11: as marcas têm 3–5 dias; quem foi arrastado resistiu no início (sulcos fundos) e depois parou de lutar. Duas correntes foram cortadas com ferramenta de lenhador (talhos limpos de machado).\n"
              "História: o sapato é de um garoto de ~9 anos — o dono está em uma das jaulas (Elin)."), kSess, kGrp);
    }

    // ====================================================== 3.2 PALICADA
    {
        MeshBuilder mb;
        const int N = 132;
        std::vector<Vec3> tops(N), bases(N); std::vector<bool> present(N, false);
        for (int i = 0; i < N; ++i) {
            float a = i * 360.f / N + rg.f(-0.4f, 0.4f);
            if (inGate(a, 8.f)) continue;
            float r = 15.f + rg.f(-0.18f, 0.18f);
            Vec3 b = polar(r, a, fy);
            float h = rg.f(3.5f, 5.6f);
            QColor c(86 + rg.i(0, 24), 70 + rg.i(0, 18), 54 + rg.i(0, 12));
            mb.cylinder(b + V3(0, -0.1f, 0), b + V3(rg.f(-0.04f, 0.04f), h, rg.f(-0.04f, 0.04f)), 0.19f, 0.15f, 6, c, MAT_ROTTEN_WOOD);
            mb.cone(b + V3(0, h - 0.02f, 0), b + V3(0, h + 0.55f, 0), 0.15f, 6, shadeC(c, 0.8f), MAT_ROTTEN_WOOD);
            tops[i] = b + V3(0, h * 0.9f, 0); bases[i] = b; present[i] = true;
        }
        for (int i = 0; i < N; ++i) {            // travessas amarradas e correntes
            int j = (i + 1) % N;
            if (!present[i] || !present[j]) continue;
            mb.cylinder(bases[i] + V3(0, 1.15f, 0.f), bases[j] + V3(0, 1.15f, 0.f), 0.06f, 5, QColor(128, 104, 70), MAT_ROPE);
            mb.cylinder(bases[i] + V3(0, 2.5f, 0.f), bases[j] + V3(0, 2.5f, 0.f), 0.06f, 5, QColor(128, 104, 70), MAT_ROPE);
            if (i % 7 == 0) chain(mb, tops[i] + V3(0, -0.4f, 0), tops[j] + V3(0, -0.4f, 0), 0.5f);
        }
        for (int i = 0; i < 12; ++i) {           // costelas gigantes de feras antigas formando arcos
            float a = i * 30.f + 15.f;
            if (inGate(a, 15.f)) continue;
            giantRib(mb, polar(15.4f, a, fy), polar(15.4f, a + 11.f, fy), 0.24f, 4.6f + rg.f(0.f, 1.5f), 30 + i);
        }
        for (int g = 0; g < 2; ++g) {            // portao sul (voltado para o Tubo A) e portao norte (traseiro)
            float ga = g == 0 ? gateS : gateN;
            Vec3 c = polar(15.f, ga, fy);
            float outYaw = yawToward(DC, c);
            Vec3 tn(-std::sin(rad(ga)), 0, std::cos(rad(ga)));
            Vec3 hingeA = c - tn * 2.1f, hingeB = c + tn * 2.1f;
            gateLeaf(mb, hingeA, outYaw + 108.f);
            gateLeaf(mb, hingeB, outYaw + 72.f);
            mb.cylinder(hingeA, hingeA + V3(0, 6.2f, 0), 0.26f, 0.22f, 7, QColor(70, 56, 42), MAT_ROTTEN_WOOD);
            mb.cylinder(hingeB, hingeB + V3(0, 6.2f, 0), 0.26f, 0.22f, 7, QColor(70, 56, 42), MAT_ROTTEN_WOOD);
            mb.cylinder(hingeA + V3(0, 6.0f, 0), hingeB + V3(0, 6.0f, 0), 0.22f, 7, QColor(74, 60, 46), MAT_ROTTEN_WOOD);
            for (int k = 0; k < 3; ++k) skull(mb, hingeA + (hingeB - hingeA) * (0.25f + 0.25f * k) + V3(0, 5.45f, 0), outYaw + 180.f, 1.4f);
            chain(mb, hingeA + V3(0, 5.9f, 0), hingeB + V3(0, 5.9f, 0), 0.9f);
            for (int k = 0; k < 2; ++k)
                totemOfBones(mb, polar(17.f + k * 1.2f, ga + (k ? 10.f : -9.f), fy), 2.2f, 40 + g * 3 + k);
        }
        w.addStatic(std::move(mb));
        for (int i = 0; i < 60; ++i) {           // colisores da paliçada (blocos de 6 graus), exceto nos portoes
            float a = i * 6.f + 3.f;
            if (inGate(a, 7.5f)) continue;
            w.addBoxCollider(Q("palicada_%1").arg(i), polar(15.f, a, fy + 2.f), V3(0.9f, 2.f, 0.28f), QQuaternion::fromAxisAndAngle(0, 1, 0, -a - 90.f));
        }
    }

    // ====================================================== JAULAS PENDURADAS (6; 4 com sequestrados)
    {
        MeshBuilder mb;
        const float ang[6] = {25.f, 100.f, 150.f, 305.f, 340.f, 55.f}, rr[6] = {8.f, 7.f, 9.f, 8.f, 7.f, 6.5f};
        const bool full[6] = {true, true, true, true, false, false};
        for (int i = 0; i < 6; ++i) {
            Vec3 p = polar(rr[i], ang[i], 0.f);
            float attachY = domeCeilingY(p.x(), p.z()) - 0.3f;
            float botY = fy + 2.5f, size = 1.8f;
            hangingCage(mb, V3(p.x(), attachY, p.z()), size, attachY - (botY + size), full[i]);
        }
        w.addStatic(std::move(mb));
        const char* nm[4] = {"Marta, a padeira (41 anos)", "Tomás, aprendiz de ferreiro (19 anos)", "Elin, o menino (9 anos)", "Hob, o lenhador velho (62 anos)"};
        const char* dur[4] = {"durabilidade estimada 11 dias ± 2", "durabilidade estimada 17 dias (o mais resistente ao frio)", "durabilidade estimada 6 dias", "durabilidade estimada 4 dias"};
        const char* extra[4] = {
            "Reza em voz baixa o tempo todo. Foi levada quando saiu para recolher lenha. Sabe onde Kaelen guarda a chave das jaulas (no cinto, em argola de ferro).",
            "Ferido no ombro; tenta quebrar a barra com o pé em silêncio. Se libertado ajuda na luta (porrete improvisado). Sabe que Kaelen 'ouve algo' à noite.",
            "O sapatinho do Tubo A é dele. Não fala; olha fixamente o braseiro azul. Se levado de volta à vila, é peça de empatia para a mesa.",
            "Conhece Kaelen há 20 anos: 'ele cortava lenha comigo, não era assim'. Diz que Kaelen conversa com alguém quando está sozinho. Perto do fim: Medicina CD 13 ou uma poção para não morrer."};
        for (int i = 0; i < 4; ++i) {
            Vec3 p = polar(rr[i], ang[i], 0.f);
            w.addPoi(V3(p.x(), fy + 1.2f, p.z()), PoiCategory::Creature, Q("3.2 · Jaula %1 — %2").arg(i + 1).arg(QString::fromUtf8(nm[i])),
                Q("Aldeão sequestrado, pendurado a ~2,5 m do chão (frio, fome, olhar vazio). Na Caderneta de Kaelen: ") + QString::fromUtf8(dur[i]) + Q(".\n\n") + QString::fromUtf8(extra[i]) +
                Q("\n\nJaula: ferro enferrujado, trinco com cadeado (Ladrões CD 15 / Força CD 18 para entortar). Descer uma jaula da corrente exige Atletismo CD 12 e faz barulho de correntes: o grupo decide entre salvar ou flanquear. Quem sobe nas jaulas fica à vista do braseiro azul."), kSess, kGrp);
        }
        for (int i = 4; i < 6; ++i) {
            Vec3 p = polar(rr[i], ang[i], 0.f);
            w.addPoi(V3(p.x(), fy + 1.2f, p.z()), PoiCategory::Area, Q("3.2 · Jaula vazia %1").arg(i - 3),
                Q("Jaula de ferro aberta, com ossos pequenos e um pano rasgado da vila no fundo. Mostra que o 'ritual' de Kaelen acontece há semanas: as duas últimas vítimas já foram 'entregues' à Máquina do Frio. Percepção CD 13: marcas de dentes na grade (os hell hounds)."), kSess, kGrp);
        }
    }

    // ====================================================== BRASEIRO DE CHAMA FRIA (centro)
    {
        const Vec3 c = V3(DC.x(), fy, DC.z());
        MeshBuilder mb;
        brazier(mb, c, 2.1f, false);
        for (int i = 0; i < 12; ++i) {
            float a = rg.f(0, 6.28f), d = rg.f(0.f, 0.5f);
            mb.box(c + V3(std::cos(a) * d, 1.78f, std::sin(a) * d), V3(0.08f, 0.06f, 0.08f), QColor(70, 140, 240), MAT_EMISSIVE);
        }
        for (int i = 0; i < 6; ++i)
            mb.ellipsoid(c + V3(std::cos(i * 1.05f) * 1.6f, 0.18f, std::sin(i * 1.05f) * 1.6f), V3(0.35f, 0.2f, 0.3f), QColor(40, 40, 46), MAT_OBSIDIAN, 6);
        w.addStatic(std::move(mb));
        MeshBuilder fm;
        for (int i = 0; i < 9; ++i) {                                  // linguas de chama fria
            float a = i * 0.7f, rb = i == 0 ? 0.f : 0.3f;
            float h = i == 0 ? 1.7f : 0.7f + 0.5f * std::fabs(std::sin(i * 1.9f));
            Vec3 b0(std::cos(a) * rb, 0.f, std::sin(a) * rb);
            Vec3 tip(std::cos(a) * rb * 0.35f + 0.08f * std::sin(i * 3.1f), h, std::sin(a) * rb * 0.35f);
            QColor c = i == 0 ? QColor(190, 230, 255) : (i % 2 ? QColor(70, 140, 255) : QColor(110, 180, 255));
            fm.cone(b0, tip, i == 0 ? 0.2f : 0.15f, 7, c, MAT_EMISSIVE);
        }
        auto fs = std::make_shared<DynHandle>(w.addDynamic(Q("chama_fria"), std::move(fm), c + V3(0, 1.78f, 0)));
        w.addUpdater([fs](float, float t) {
            float f = 1.f + 0.12f * std::sin(t * 7.f) + 0.07f * std::sin(t * 13.1f);
            (*fs)->scl = Vec3(1.f - 0.05f * std::sin(t * 9.f), f, 1.f - 0.05f * std::sin(t * 11.f));
            (*fs)->setYaw(t * 25.f);
        });
        w.addCylinderCollider(Q("braseiro_frio"), c + V3(0, 0.9f, 0), 0.8f, 0.9f);
        Light* l = w.addLight(c + V3(0, 2.2f, 0), V3(0.35f, 0.65f, 1.0f), 11.f, 32.f, LightAnim::Pulse, Q("chama fria"));
        l->animFreq = 0.8f; l->animDepth = 0.22f;
        w.addLight(c + V3(0, 7.f, 0), V3(0.25f, 0.45f, 0.9f), 3.5f, 26.f, LightAnim::Pulse, Q("reflexo na cupula"))->animFreq = 0.35f;
        EmitterDesc ed; ed.preset = ParticlePreset::Sparks; ed.name = Q("brasas azuis"); ParticleEmitter::applyPreset(ed);
        ed.pos = c + V3(0, 2.0f, 0); ed.halfExtent = V3(0.35f, 0.1f, 0.35f); ed.rate = 14.f; ed.velocity = V3(0, 1.3f, 0); ed.velJitter = 0.4f; ed.gravity = V3(0, 0.15f, 0);
        ed.lifeMin = 1.5f; ed.lifeMax = 3.5f; ed.sizeMin = 0.03f; ed.sizeMax = 0.07f;
        ed.colorStart = QColor(130, 200, 255, 255); ed.colorEnd = QColor(60, 90, 255, 0);
        w.addEmitter(ed);
        w.addPoi(c + V3(0, 2.6f, 0), PoiCategory::Area, Q("3.2 · Braseiro de chama fria"),
            Q("Bacia de bronze com fogo azul-pálido que NÃO aquece: ilumina o entroncamento inteiro com luz de gelo. É o foco 'ritual' de Kaelen — ele acredita que a chama fria é o sinal da voz da Montanha.\n\n"
              "Mecânica: o fogo azul não dá calor (6 °C constantes) e queima necroticamente 1d6 por rodada a quem encostar. Detectar Magia: aura de Necromancia (resquício do Alhoon); Dissipar Magia (CD 14) apaga por 1 h. Sem ele a sala cai na escuridão (só tochas) e Kaelen ganha vantagem em Percepção contra tochas.\n"
              "Narrativa: as cabeças das jaulas olham sempre para ele."), kSess, kGrp);
    }

    // ====================================================== POSTO DE COMANDO + CADERNETA
    const Vec3 post = V3(-80.f, fy, -262.f);
    {
        MeshBuilder mb;
        float yawP = yawToward(post, polar(15.f, gateS, fy));
        altar(mb, post, yawP, V3(4.4f, 0.42f, 3.4f), MAT_OBSIDIAN);
        Vec3 tp = post + V3(0, 0.42f, 0);
        table(mb, tp, yawP, V3(2.4f, 0.85f, 1.05f), true);
        float ty = tp.y() + 0.85f;
        mb.push(); mb.translate(V3(tp.x(), ty + 0.006f, tp.z())); mb.rotateY(yawP);      // mapa grande + alfinetes
        mb.box(V3(-0.1f, 0, 0), V3(0.55f, 0.004f, 0.4f), QColor(214, 196, 150), MAT_PAPER);
        for (int i = 0; i < 6; ++i) mb.box(V3(-0.5f + i * 0.18f, 0.008f, rg.f(-0.3f, 0.3f)), V3(0.15f, 0.003f, 0.01f), QColor(120, 30, 30), MAT_PAPER);
        for (int i = 0; i < 4; ++i) mb.sphere(V3(rg.f(-0.5f, 0.3f), 0.03f, rg.f(-0.3f, 0.3f)), 0.018f, QColor(90, 160, 255), MAT_EMISSIVE, 5);
        mb.push(); mb.translate(0.62f, 0.012f, 0.12f); mb.rotateY(-12.f);                // a Caderneta (aberta)
        mb.box(V3(0, 0.01f, 0), V3(0.16f, 0.012f, 0.11f), QColor(40, 40, 52), MAT_CLOTH);
        mb.box(V3(-0.075f, 0.026f, 0), V3(0.072f, 0.006f, 0.1f), QColor(226, 218, 190), MAT_PAPER);
        mb.box(V3(0.075f, 0.026f, 0), V3(0.072f, 0.006f, 0.1f), QColor(226, 218, 190), MAT_PAPER);
        for (int i = 0; i < 6; ++i) mb.box(V3(-0.075f + (i % 2) * 0.15f, 0.033f, -0.07f + (i / 2) * 0.05f), V3(0.05f, 0.001f, 0.004f), QColor(40, 30, 28), MAT_COAL);
        mb.pop();
        mb.pop();
        Vec3 noteP(tp.x() + 0.55f, ty + 0.1f, tp.z() + 0.05f);
        mb.cylinder(V3(tp.x() - 0.9f, ty, tp.z() + 0.1f), V3(tp.x() - 0.9f, ty + 0.18f, tp.z() + 0.1f), 0.04f, 6, QColor(200, 190, 160), MAT_BONE);
        mb.sphere(V3(tp.x() - 0.9f, ty + 0.21f, tp.z() + 0.1f), 0.022f, QColor(255, 190, 90), MAT_EMISSIVE, 5);
        crate(mb, post + V3(-3.1f, 0, 0.6f), 20.f, 0.9f); barrel(mb, post + V3(-3.5f, 0, -1.2f), 0.f, 1.1f);
        sack(mb, post + V3(2.7f, 0, -1.5f), 15.f, 1.0f);
        w.addStatic(std::move(mb));
        w.addLight(V3(tp.x() - 0.9f, ty + 0.5f, tp.z() + 0.1f), V3(1.0f, 0.7f, 0.35f), 2.4f, 7.f, LightAnim::TorchFlicker, Q("vela do posto"));
        w.addBoxCollider(Q("mesa_posto"), V3(tp.x(), ty - 0.45f, tp.z()), V3(1.2f, 0.45f, 0.55f), QQuaternion::fromAxisAndAngle(0, 1, 0, yawP));
        auto pg = std::make_shared<int>(0);
        w.addInteractable(Q("caderneta_kaelen"), noteP, 1.0f, Q("Ler a Caderneta de Kaelen"), [&w, pg]() {
            static const char* L[] = {
                "Caderneta de Kaelen — pág. 14. \"Sujeito 1: Marta, 41 a. Durabilidade 11 dias ±2. Rendimento térmico: 0,8 unidade/dia. Reza em voz alta; irrelevante.\"",
                "\"Sujeito 2: Tomás, 19 a. Durabilidade 17 dias. Maior resistência ao frio — prioridade para a Máquina do Frio. Sujeito 3: Elin, 9 a. Durabilidade 6 dias.\"",
                "\"Sujeito 4: Hob, 62 a. Durabilidade 4 dias. Rendimento insuficiente. A voz diz que serve: o fogo exige combustível.\"",
                "\"A Montanha fala todas as noites. Ela chama de Máquina do Frio o que eu não compreendo. Eu carrego o peso para que a vila não precise congelar.\"",
                "\"Cálculo: 50 vidas por mês × custo. A matemática não mente. Por que, então, o menino não para de chorar? — ver pág. 15 (sem anotação)\""};
            int n = (int)(sizeof(L) / sizeof(L[0]));
            w.state.toast(QString::fromUtf8(L[*pg % n]), QColor(235, 220, 180), 8.f);
            ++*pg;
            w.state.setFlag(Q("leu_caderneta"));
        });
        w.addPoi(noteP + V3(0, 0.7f, 0), PoiCategory::Lore, Q("3.2 · Caderneta de Kaelen (lore)"),
            Q("Encadernada em couro escuro, sem uma única oração: anotações frias e matemáticas sobre a 'durabilidade' de cada sequestrado e referências à 'Máquina do Frio'.\n\n"
              "Conteúdo (cada [E] mostra uma página): Marta 11 dias; Tomás 17 dias; Elin 6 dias; Hob 4 dias; tabelas de 'rendimento térmico por pessoa'; um cálculo '50 vidas/mês'. Na margem, em letra trêmula: 'a voz diz que o fogo exige combustível'.\n"
              "Revelação: Kaelen acredita que a voz é o Espírito da Montanha / o Dragão — na verdade é o Alhoon dominando sua mente. Investigação CD 14 liga o vocabulário ('durabilidade', 'rendimento', 'custo de operação') ao diário de calibração do Alhoon (Camada 5).\n"
              "Ganho: o grupo aprende o nome 'Máquina do Frio' e que há um quinto sujeito sem nome (a página 15 está em branco)."), kSess, kGrp);
        w.addPoi(post + V3(0.f, 1.3f, 0.2f), PoiCategory::Area, Q("3.2 · Posto de comando de Kaelen"),
            Q("Pódio de obsidiana negra com uma mesa de campanha: mapas do caminho (Tubos A, B, C e D marcados com alfinetes azuis), a Caderneta de Kaelen, uma vela e restos de comida. Cheira a fumaça de lenha e pele molhada.\n\n"
              "Mapas: mostram a escalada do poço, o corredor térmico ('gelo mágico, não passar') e uma marca 'MÁQUINA' ao norte, abaixo da Fenda. Investigação CD 12 para decifrar a rota até o labirinto.\n"
              "O pódio tem 0,4 m de altura (cobertura leve para Kaelen); a mesa dá meia cobertura. Um prisioneiro olha de uma jaula a 6 m."), kSess, kGrp);
    }

    // ====================================================== OUTROS ADERECOS DO ACAMPAMENTO
    {
        MeshBuilder mb;
        hideTent(mb, V3(-83.5f, fy, -247.f), 25.f, 1.9f, 1.7f, QColor(96, 78, 66));
        hideTent(mb, V3(-65.5f, fy, -246.f), -30.f, 1.7f, 1.6f, QColor(84, 72, 64));
        for (int r = 0; r < 4; ++r)                      // lenha empilhada
            for (int c = 0; c < 5 - r; ++c)
                mb.cylinder(V3(-87.f + c * 0.3f + r * 0.15f, fy + 0.15f + r * 0.28f, -263.f), V3(-87.f + c * 0.3f + r * 0.15f, fy + 0.15f + r * 0.28f, -261.f), 0.14f, 7, QColor(110 - r * 5, 82, 56), MAT_WOOD);
        mb.cylinder(V3(-87.f, fy, -254.f), V3(-87.f, fy + 0.55f, -254.f), 0.38f, 0.34f, 10, QColor(104, 76, 52), MAT_WOOD);   // cepo + machado
        mb.cylinder(V3(-87.f, fy + 0.5f, -254.f), V3(-87.f, fy + 1.1f, -254.1f), 0.03f, 6, QColor(90, 62, 40), MAT_WOOD);
        mb.box(V3(-87.f, fy + 1.0f, -254.18f), V3(0.2f, 0.12f, 0.025f), QColor(96, 96, 104), MAT_IRON);
        for (int i = 0; i < 9; ++i) {
            Vec3 p = polar(rg.f(3.f, 13.f), rg.f(0.f, 360.f), fy);
            if (std::hypot(p.x() - DC.x(), p.z() - DC.z()) < 2.4f) continue;
            if (rg.f() < 0.6f) bonesPile(mb, p, rg.f(0.4f, 0.9f), 50 + i); else skull(mb, p, rg.f(0, 360), 1.f);
        }
        const Vec3 ken = V3(-66.f, fy, -266.f);          // canil
        for (int i = 0; i < 6; ++i) {
            Vec3 a = ken + V3(-2.5f + i * 1.0f, 0, 2.4f);
            mb.cylinder(a, a + V3(0, 1.1f, 0), 0.07f, 5, QColor(80, 62, 48), MAT_ROTTEN_WOOD);
        }
        mb.cylinder(ken + V3(-2.5f, 0.9f, 2.4f), ken + V3(2.5f, 0.9f, 2.4f), 0.04f, 5, QColor(110, 90, 66), MAT_ROPE);
        mb.cylinder(ken + V3(-2.5f, 0.5f, 2.4f), ken + V3(2.5f, 0.5f, 2.4f), 0.04f, 5, QColor(110, 90, 66), MAT_ROPE);
        mb.cylinder(ken + V3(1.9f, 0.01f, -0.4f), ken + V3(1.9f, 0.14f, -0.4f), 0.28f, 8, QColor(70, 60, 50), MAT_IRON);
        mb.cylinder(ken + V3(2.3f, 0.01f, 0.9f), ken + V3(2.3f, 0.1f, 0.9f), 0.22f, 8, QColor(70, 60, 50), MAT_IRON);
        bonesPile(mb, ken + V3(-0.8f, 0, -1.2f), 0.8f, 71); bonesPile(mb, ken + V3(0.8f, 0, 1.0f), 0.6f, 72);
        for (int i = 0; i < 2; ++i) {
            Vec3 stake = ken + V3(i ? 1.5f : -1.6f, 0, -1.9f);
            mb.cylinder(stake, stake + V3(0, 0.5f, 0), 0.08f, 6, QColor(60, 48, 40), MAT_IRON);
        }
        w.addStatic(std::move(mb));
    }

    // ====================================================== BAU DE KAELEN (Anel de Aquecimento)
    auto cs = std::make_shared<CampState>();
    {
        const Vec3 cp = V3(-84.f, fy, -258.f);
        float yawC = yawToward(cp, DC);
        MeshBuilder shut; Prefabs::chest(shut, V3(0, 0, 0), 0.f, false, 1.6f);
        MeshBuilder opn; Prefabs::chest(opn, V3(0, 0, 0), 0.f, true, 1.6f);
        opn.torus(V3(0, 0.52f, 0), V3(0, 1, 0), 0.07f, 0.014f, QColor(255, 190, 70), MAT_GOLD, 14, 6);
        opn.sphere(V3(0, 0.58f, 0.f), 0.022f, QColor(255, 110, 30), MAT_EMISSIVE, 6);
        cs->chestShut = w.addDynamic(Q("bau_kaelen_fechado"), std::move(shut), cp, QQuaternion::fromAxisAndAngle(0, 1, 0, yawC));
        cs->chestOpen = w.addDynamic(Q("bau_kaelen_aberto"), std::move(opn), cp, QQuaternion::fromAxisAndAngle(0, 1, 0, yawC));
        cs->chestOpen->visible = false;
        w.addBoxCollider(Q("bau_kaelen"), cp + V3(0, 0.3f, 0), V3(0.8f, 0.3f, 0.5f), QQuaternion::fromAxisAndAngle(0, 1, 0, yawC));
        Light* gl = w.addLight(cp + V3(0, 1.1f, 0), V3(1.0f, 0.45f, 0.12f), 3.2f, 7.f, LightAnim::Pulse, Q("anel"));
        gl->animFreq = 0.5f; gl->enabled = false;
        w.addInteractable(Q("bau_kaelen"), cp + V3(0, 0.6f, 0), 1.1f, Q("Abrir o Baú de Kaelen"), [&w, cs, gl]() {
            if (cs->chestOpened) return;
            cs->chestOpened = true;
            cs->chestShut->visible = false; cs->chestOpen->visible = true; gl->enabled = true;
            w.state.addItem(Q("Anel de Aquecimento"));
            w.state.toast(Q("Um calor suave sobe do seu dedo: o frio recua. (Anel de Aquecimento — Ring of Warmth)"), QColor(255, 190, 110), 5.f);
            w.state.flash(QColor(255, 150, 70), 0.3f);
        });
        w.addPoi(cp + V3(0, 1.2f, 0), PoiCategory::Treasure, Q("3.2 · Baú de Kaelen — Anel de Aquecimento"),
            Q("Baú de lenhador reforçado com ferro, trancado (Ladrões CD 15 / Força CD 17 / a chave está no cinto de Kaelen). Dentro: o Anel de Aquecimento (Ring of Warmth), o item mais cobiçado da campanha.\n\n"
              "Efeito (Ring of Warmth): resistência a dano de frio e o portador ignora os efeitos do frio extremo (−20 °C, −15 °C): sem exaustão por frio. Detectar Magia: Abjuração.\n"
              "Também há: 80 po em moedas de cobre da vila, um odre de cerveja, uma luva infantil e uma carta do padre (Kaelen o conhecia? lore opcional).\n"
              "Armadilha opcional: agulha envenenada (Percepção CD 14; Con CD 13 ou 2d6 veneno). Local: (−84, −186, −258)."), kSess, kGrp);
    }

    // ====================================================== KAELEN + HELL HOUNDS (dinamicos)
    {
        HumanoidParams hp; hp.height = 1.98f; hp.build = 1.28f; hp.skin = QColor(168, 164, 176); hp.cloth = QColor(84, 68, 56); hp.cloth2 = QColor(44, 40, 42);
        hp.cape = true; hp.axe = true; hp.glowingEyes = true; hp.eyeColor = QColor(130, 205, 255); hp.hair = QColor(40, 36, 38);
        MeshBuilder km; Prefabs::humanoid(km, V3(0, 0, 0), 0.f, hp);
        Vec3 kp = post + V3(-0.9f, 0.42f, -1.7f);
        cs->kBase = yawToward(kp, polar(15.f, gateS, fy));
        cs->kYaw = cs->kBase;
        cs->kaelen = w.addDynamic(Q("kaelen"), std::move(km), kp, QQuaternion::fromAxisAndAngle(0, 1, 0, cs->kBase));
        cs->kLight = w.addLight(kp + V3(0, 1.7f, 0.f), V3(0.5f, 0.8f, 1.0f), 1.8f, 6.f, LightAnim::Pulse, Q("aura necrotica de Kaelen"));
        cs->kLight->animFreq = 0.6f;
        const Vec3 hp0[2] = {V3(-66.6f, fy, -266.f), V3(-64.6f, fy, -264.4f)};
        for (int i = 0; i < 2; ++i) {
            MeshBuilder hm; Prefabs::wolf(hm, V3(0, 0, 0), 0.f, true, i ? Pose::Crouch : Pose::Stand, 1.25f);
            cs->hBase[i] = yawToward(hp0[i], polar(15.f, gateS, fy)) + (i ? 20.f : -20.f);
            cs->hYaw[i] = cs->hBase[i];
            cs->hound[i] = w.addDynamic(Q("hellhound_%1").arg(i + 1), std::move(hm), hp0[i], QQuaternion::fromAxisAndAngle(0, 1, 0, cs->hBase[i]));
            cs->hLight[i] = w.addLight(hp0[i] + V3(0, 1.1f, 0), V3(0.3f, 0.6f, 1.0f), 1.2f, 4.5f, LightAnim::Pulse, Q("olhos do cao %1").arg(i + 1));
            cs->hLight[i]->animFreq = 0.9f;
        }
        MeshBuilder cm;
        chain(cm, hp0[0] + V3(0, 0.7f, 0), V3(-65.6f, fy + 0.4f, -267.9f), 0.15f);
        chain(cm, hp0[1] + V3(0, 0.7f, 0), V3(-63.5f, fy + 0.4f, -267.9f), 0.15f);
        w.addStatic(std::move(cm));
        w.registerEvent(Q("kaelen_alert"), Q("Kaelen percebe o grupo"), Qt::Key_Y, [&w, cs]() {
            cs->alert = !cs->alert; cs->t = 0.f; cs->line = 0;
            if (cs->alert) {
                w.state.toast(Q("Kaelen: \"O fogo exige combustível!\""), QColor(140, 205, 255), 4.f);
                w.state.screenShake(0.04f, 0.5f); w.state.flash(QColor(80, 140, 255), 0.25f);
            } else w.state.toast(Q("Kaelen volta a vigiar o portão."), QColor(200, 210, 230), 3.f);
        });
        w.addUpdater([&w, cs](float dt, float t) {
            Vec3 pp = w.playerPos();
            auto upd = [&](DynHandle d, float& yaw, float base, bool bounce) {
                float target = cs->alert ? yawToward(d->pos, pp) : base;
                yaw = turnToward(yaw, target, (cs->alert ? 260.f : 70.f) * dt);
                d->setYaw(yaw);
                if (bounce) d->scl = cs->alert ? Vec3(1.f, 1.f + 0.015f * std::sin(t * 30.f), 1.f) : Vec3(1, 1, 1);
            };
            upd(cs->kaelen, cs->kYaw, cs->kBase, false);
            for (int i = 0; i < 2; ++i) upd(cs->hound[i], cs->hYaw[i], cs->hBase[i], true);
            if (cs->alert) {
                cs->t += dt;
                cs->kLight->intensity = 4.8f + 1.4f * std::sin(cs->t * 9.f);
                for (int i = 0; i < 2; ++i) cs->hLight[i]->intensity = 3.2f + 1.0f * std::sin(cs->t * 12.f + i);
                if (cs->line == 0 && cs->t > 3.8f) { cs->line = 1; w.state.toast(Q("Kaelen: \"Eu carrego o peso para que a vila não precise congelar!\""), QColor(140, 205, 255), 4.5f); }
            } else { cs->kLight->intensity = 1.8f; for (int i = 0; i < 2; ++i) cs->hLight[i]->intensity = 1.2f; }
        });
        w.addPoi(kp + V3(0, 2.4f, 0), PoiCategory::Creature, Q("3.2 · Kaelen (mid-boss)"),
            Q("Humano alto (~1,98 m), ex-lenhador e guia da vila. Machado de lenhador, capa de pele, aura de frio necrótico (luz azul-pálida). Os olhos brilham em azul quando fala com a 'voz'.\n\n"
              "Stat block sugerido: Warlock/Blackguard de ~nível 8 (CR 8–9; PV ~110): machado +1 (1d12+5 cortante + 2d6 necrótico), Aura de Frio (3 m: Con CD 14 ou −3 m de deslocamento), Raio de Gelo Sombrio (3d8 de frio/necrótico), Grito do Espírito (1/dia: Sab CD 15 ou assustado).\n"
              "Frases: \"O fogo exige combustível!\", \"Eu carrego o peso para que a vila não precise congelar!\", \"A Montanha me pede mais um.\" — acredita que é a voz do Espírito da Montanha/Dragão (na verdade, o Alhoon).\n"
              "Tática: abre com os cães para flanquear; usa a chama fria como foco; se os prisioneiros forem ameaçados, usa um como escudo. Dá para negociar (Persuasão CD 20) mostrando o sapato de Elin ou a carta do padre.\n"
              "Tecla [Y]: Kaelen e os cães se viram para o grupo, olhos acendem e ele fala. Tesouro: Baú e Caderneta."), kSess, kGrp);
        w.addPoi(hp0[0] + V3(0.5f, 1.8f, 0.4f), PoiCategory::Creature, Q("3.2 · Canil — 2 Hell Hounds corrompidos"),
            Q("Dois lobos da montanha distorcidos pela magia do Alhoon: pelo escuro, rachaduras de brilho azul, olhos de brasa azul. Presos por correntes a estacas (alcance 3 m); latem se algo passa no Tubo A (fio de alarme).\n\n"
              "Hell Hound (MM, CR 3 — reflavor: sopro de Gelo Sombrio no lugar do fogo; imunes a frio, vulneráveis a fogo). PV 45 cada. Mordida 2d6+3 + 1d6 gelo; Sopro (recarga 5–6): cone de 4,5 m, Des CD 12, 6d6 frio.\n"
              "Tática: flanqueiam os mais fracos, vão para o portão se Kaelen chamar; se soltos, atacam quem estiver sob as jaulas. Adestrar Animais CD 16 acalma um cão por 1 h (são vítimas da corrupção). Os olhos acendem quando [Y] é pressionado."), kSess, kGrp);
    }

    // ====================================================== POIs adicionais
    {
        Vec3 g = polar(15.f, gateS, fy);
        w.addPoi(g + V3(0, 1.4f, 0), PoiCategory::Area, Q("3.2 · Portão da paliçada (voltado para o Tubo A)"),
            Q("Entrada de 4 m na paliçada de madeira podre (anel de raio 15 m, ~5 m de altura), reforçada com costelas de feras antigas (ossos gigantes arqueados) e correntes; crânios pendurados no lintel. Duas folhas apodrecidas escoradas abertas.\n\n"
              "Regras: a paliçada dá cobertura 3/4; escalar (Atletismo CD 12) leva a um topo pontiagudo. Dá para contornar por fora, no corredor de 7 m entre a paliçada e a parede da cúpula (os Tubos B, C e D saem de lá) — Furtividade CD 14 sob a luz azul.\n"
              "Alarme: os cães latem quando o fio do Tubo A é tocado."), kSess, kGrp);
        Vec3 gn = polar(15.f, gateN, fy);
        w.addPoi(gn + V3(0, 1.4f, 0), PoiCategory::Area, Q("3.2 · Portão traseiro da paliçada (rumo ao Tubo C)"),
            Q("Passagem de 4 m na paliçada, voltada para o norte: a rota principal sai por aqui em direção ao Tubo C e à Fenda. Uma folha tem corrente quebrada; marcas de arrasto indicam por onde os sequestrados são levados à Máquina do Frio."), kSess, kGrp);
        w.addPoi(V3(DC.x(), fy + 12.f, DC.z()), PoiCategory::Area, Q("3.2 · O Entroncamento de Kaelen (visão geral)"),
            Q("Cúpula onde quatro tubos se encontram. Centro (−75, −186, −255), raio horizontal 22 m, chão y −186, teto y −166 (20 m): uma 'nave' de obsidiana negra espelhada, cheia de estalactites vítreas. Chão de cinza petrificada. 6 °C.\n\n"
              "Ao entrar pela paliçada: braseiro de chama fria no centro, 6 jaulas penduradas entre os raios 6 e 9 (4 com aldeões), posto de comando a noroeste, canil a leste, baú ao lado do posto, lenha e tendas.\n"
              "Saídas: Tubo A (sul, chegada), Tubo B (oeste, Fungos), Tubo C (norte, Fenda), Tubo D (leste, secreto: chaminé e dutos de respiro por cima da cúpula)."), kSess, kGrp);
    }

    w.addLight(V3(DC.x(), fy + 17.f, DC.z()), V3(0.2f, 0.38f, 0.8f), 2.6f, 30.f, LightAnim::Pulse, Q("brilho frio no teto"))->animFreq = 0.15f;
    w.addLight(V3(DC.x() - 10.f, fy + 3.f, DC.z() + 4.f), V3(0.25f, 0.45f, 0.85f), 3.0f, 14.f, LightAnim::Pulse, Q("reflexo frio"))->animFreq = 0.25f;
    w.addLight(V3(DC.x() + 9.f, fy + 3.f, DC.z() - 6.f), V3(0.25f, 0.45f, 0.85f), 3.0f, 14.f, LightAnim::Pulse, Q("reflexo frio"))->animFreq = 0.3f;
    EmitterDesc dd; dd.preset = ParticlePreset::Dust; dd.name = Q("cinza no ar"); ParticleEmitter::applyPreset(dd);
    dd.pos = V3(DC.x(), fy + 6.f, DC.z()); dd.halfExtent = V3(18.f, 6.f, 18.f); dd.rate = 12.f; dd.colorStart = QColor(150, 175, 215, 70);
    w.addEmitter(dd);
}
