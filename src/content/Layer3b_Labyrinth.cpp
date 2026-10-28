// CAMADA 3b — O Labirinto Alienigena (Sessao 4, Nivel 8)
// Corredores lisos e curvas antinaturais em rocha ALTERADA, duas Paredes Mutantes, zumbido psiquico,
// 2 Devoradores de Intelecto no teto, 1 Grell no Ninho, Camara da Paranoia (descanso) e saida para o Balcao do Geodo.
#include "Dungeon.h"
#include "world/World.h"
#include "Prefabs.h"
#include <memory>

#define Q(s) QStringLiteral(s)

namespace {

const QString kGrp = QStringLiteral("Camada 3b · O Labirinto Alienígena");
const QString kSess = QStringLiteral("Sessão 4 · Nível 8");
const float kR = 2.15f;      // raio dos corredores (largura util ~3,5 m, altura ~3,4 m)
const float kFF = 0.6f;      // floorFrac

// L0..L10 (y = chao)
const Vec3 L[11] = {
    Vec3(-92, -200, -345), Vec3(-92, -200, -355), Vec3(-108, -201, -355), Vec3(-120, -201, -362),
    Vec3(-108, -201, -370), Vec3(-92, -202, -370), Vec3(-78, -202, -362), Vec3(-92, -202, -384),
    Vec3(-108, -203, -386), Vec3(-78, -203, -386), Vec3(-70, -205, -392)};

// pontos de um corredor curvo (suave, "antinatural"): a -> b com ondulacao lateral senoidal
std::vector<Vec3> curve(const Vec3& a, const Vec3& b, float amp, float waves = 1.f)
{
    Vec3 d = b - a; d.setY(0);
    float len = (b - a).length();
    Vec3 perp = d.lengthSquared() > 1e-4f ? Vec3(-d.z(), 0, d.x()).normalized() : Vec3(1, 0, 0);
    int n = std::max(3, (int)std::ceil(len / 2.2f));
    std::vector<Vec3> pts;
    for (int i = 0; i <= n; ++i) {
        float t = (float)i / n;
        Vec3 p = a + (b - a) * t;
        p.setY(a.y() + (b.y() - a.y()) * (t * t * (3 - 2 * t)));
        float env = std::sin(t * 3.14159f);
        p += perp * (amp * env * std::sin(t * 3.14159f * waves));
        pts.push_back(p);
    }
    return pts;
}
// ponto do meio de um corredor curvo, elevado de dy sobre o chao
Vec3 midOf(const Vec3& a, const Vec3& b, float amp, float waves, float dy, float t = 0.5f)
{
    auto pts = curve(a, b, amp, waves);
    float f = t * (pts.size() - 1);
    int i = std::min((int)f, (int)pts.size() - 2);
    Vec3 p = pts[i] + (pts[i + 1] - pts[i]) * (f - i);
    p.setY(p.y() + dy);
    return p;
}

void corridor(World& w, const std::vector<Vec3>& floorPts, float r = kR)
{
    std::vector<Vec3> pts = floorPts;
    for (auto& p : pts) p.setY(p.y() + r * kFF);
    TunnelOpts o; o.floorFrac = kFF; o.material = MAT_ALTERED; o.noise = 0.06f; o.k = 2.4f; o.tag = Q("labirinto");
    w.carveTunnel(pts, r, o);
}

void junction(World& w, const Vec3& f, float rx = 3.7f, float ry = 2.3f)
{
    w.carveChamber(Vec3(f.x(), f.y() + ry * 0.85f, f.z()), Vec3(rx, ry, rx), f.y(), MAT_ALTERED, 0.05f, 2.2f, Q("no"));
}

// Devorador de Intelecto: cerebro com patas, pes em y=0 (depois virado de cabeca para baixo no teto).
void devourerMesh(MeshBuilder& mb)
{
    QColor brain(214, 132, 150), fold(176, 96, 118);
    mb.ellipsoid(Vec3(0, 0.32f, 0), Vec3(0.30f, 0.20f, 0.38f), brain, MAT_FLESH, 9);
    mb.ellipsoid(Vec3(0.12f, 0.40f, 0.02f), Vec3(0.14f, 0.1f, 0.26f), fold, MAT_FLESH, 7);
    mb.ellipsoid(Vec3(-0.12f, 0.40f, 0.02f), Vec3(0.14f, 0.1f, 0.26f), fold, MAT_FLESH, 7);
    mb.cylinder(Vec3(0, 0.30f, -0.32f), Vec3(0, 0.20f, -0.46f), 0.06f, 0.03f, 6, QColor(120, 40, 50), MAT_FLESH);
    for (int i = 0; i < 4; ++i) {
        float sx = (i & 1) ? 1.f : -1.f, sz = (i & 2) ? 0.5f : -0.5f;
        Vec3 hip(sx * 0.2f, 0.28f, sz * 0.5f);
        Vec3 knee(sx * 0.42f, 0.34f, sz * 0.7f);
        Vec3 foot(sx * 0.5f, 0.0f, sz * 0.8f);
        mb.cylinder(hip, knee, 0.035f, 0.03f, 5, QColor(190, 110, 120), MAT_FLESH);
        mb.cylinder(knee, foot, 0.03f, 0.012f, 5, QColor(70, 30, 35), MAT_BONE);
    }
}

// Grell: cerebro flutuante com bico e 10 tentaculos.
void grellMesh(MeshBuilder& mb)
{
    mb.ellipsoid(Vec3(0, 0, 0), Vec3(0.85f, 0.7f, 0.85f), QColor(120, 90, 160), MAT_FLESH, 12);
    mb.ellipsoid(Vec3(0, 0.25f, 0), Vec3(0.7f, 0.5f, 0.7f), QColor(150, 110, 190), MAT_FLESH, 10);
    mb.cone(Vec3(0, -0.55f, 0.25f), Vec3(0, -0.95f, 0.45f), 0.22f, 6, QColor(40, 35, 45), MAT_BONE);
    for (int i = 0; i < 10; ++i) {
        float a = i * 6.2831853f / 10.f;
        Vec3 base(std::cos(a) * 0.45f, -0.45f, std::sin(a) * 0.45f);
        std::vector<Vec3> pts;
        for (int k = 0; k <= 6; ++k) {
            float t = k / 6.f;
            pts.push_back(base + Vec3(std::cos(a) * 0.35f * t + std::sin(t * 5 + i) * 0.1f, -t * 2.1f, std::sin(a) * 0.35f * t + std::cos(t * 4 + i) * 0.1f));
        }
        mb.tube(pts, std::vector<float>{0.07f, 0.065f, 0.06f, 0.05f, 0.04f, 0.03f, 0.015f}, 6, QColor(105, 80, 140), MAT_FLESH);
        mb.sphere(pts.back(), 0.05f, QColor(220, 220, 200), MAT_BONE, 5);
    }
    mb.sphere(Vec3(0.55f, 0.1f, 0.5f), 0.07f, QColor(250, 240, 120), MAT_EMISSIVE, 5);
    mb.sphere(Vec3(-0.55f, 0.1f, 0.5f), 0.07f, QColor(250, 240, 120), MAT_EMISSIVE, 5);
}

struct Crawler { DynHandle d = nullptr; Vec3 a, b; float s = 0.f; float dir = 1.f; float speed = 0.4f; };

struct Mutant {
    DynHandle wall[2] = {nullptr, nullptr};
    Collider* col[2] = {nullptr, nullptr};
    int active = 0;        // 0 = bloqueia L5-L7 ; 1 = bloqueia L6-L9
    float cd = 3.f;
    int swaps = 0;
};

} // namespace

void buildLayer3bLabyrinth(World& w)
{
    // ======================================================== GEOMETRIA
    junction(w, L[0], 3.4f, 2.2f);
    corridor(w, curve(L[0], L[1], 0.f));
    junction(w, L[1]);
    corridor(w, curve(L[1], L[2], 1.6f, 1.f));          // L1-L2
    junction(w, L[2], 4.f, 2.4f);
    corridor(w, curve(L[2], L[3], -1.4f, 1.f));         // L2-L3 (ninho do Grell)
    corridor(w, curve(L[2], L[4], 1.5f, 1.f));          // L2-L4 (Devorador 1 no teto)
    junction(w, L[4]);
    corridor(w, curve(L[4], L[5], -1.8f, 1.f));         // L4-L5
    junction(w, L[5], 4.f, 2.4f);
    corridor(w, curve(L[1], L[6], 1.2f, 1.f));          // L1-L6
    junction(w, L[6], 4.f, 2.4f);
    corridor(w, curve(L[5], L[6], -1.2f, 1.f));         // L5-L6
    corridor(w, curve(L[5], L[7], 1.4f, 1.f));          // L5-L7 (Parede Mutante A)
    junction(w, L[7], 4.2f, 2.4f);
    corridor(w, curve(L[7], L[8], 1.6f, 1.f));          // L7-L8
    junction(w, L[8]);
    corridor(w, curve(L[7], L[9], -1.6f, 1.f));         // L7-L9 (Devorador 2 no teto)
    junction(w, L[9], 4.f, 2.4f);
    corridor(w, curve(L[6], L[9], 1.8f, 1.5f));         // L6-L9 (Parede Mutante B)
    corridor(w, curve(L[9], L[10], 0.8f, 1.f));         // L9-L10
    junction(w, L[10], 3.6f, 2.3f);
    // saida: L10 -> (-55,-209,-388) -> (-40,-213,-384) (continua na Camada 4: balcao)
    corridor(w, {L[10], Vec3(-62, -207, -390), Vec3(-55, -209, -388), Vec3(-47, -211.2f, -386), Vec3(-40, -213, -384)}, 2.3f);

    // Ninho do Grell (beco em L3)
    const Vec3 nestC(-121, -201, -363.5f);
    w.carveChamber(Vec3(nestC.x(), -196.5f, nestC.z()), Vec3(8.5f, 5.4f, 7.5f), -201.f, MAT_ALTERED, 0.12f, 2.5f, Q("ninho"));
    // Camara da Paranoia: gargalo estreito + beco de ~10 m
    corridor(w, {L[8], Vec3(-110.5f, -203, -388), Vec3(-113, -203, -390.5f)}, 1.35f);
    w.carveChamber(Vec3(-117, -200.8f, -394.5f), Vec3(6.2f, 3.1f, 5.6f), -203.f, MAT_ALTERED, 0.1f, 2.f, Q("paranoia"));

    // ======================================================== ZONAS
    {
        ZoneEffects fx;
        fx.areaName = Q("Camada 3b · O Labirinto Alienígena"); fx.subtitle = kSess + Q(" · 12 °C · zumbido psíquico");
        fx.temperature = 12.f; fx.fogColor = Vec3(0.022f, 0.012f, 0.04f); fx.fogDensity = 0.030f;
        fx.ambientColor = Vec3(0.050f, 0.030f, 0.085f); fx.ambientGround = Vec3(0.022f, 0.014f, 0.040f); fx.ambientStrength = 1.f;
        fx.psychic = 0.55f;
        w.addZone(Q("labirinto"), ZoneShape::makeBox(Vec3(-132, -212, -400), Vec3(-58, -190, -339.5f)), fx, 3);
        ZoneEffects nf = fx; nf.areaName = Q("3b · Ninho do Grell"); nf.subtitle = kSess; nf.psychic = 0.75f;
        nf.fogColor = Vec3(0.03f, 0.014f, 0.05f);
        w.addZone(Q("ninho_grell"), ZoneShape::makeBox(Vec3(-131, -204, -372), Vec3(-111, -190, -354)), nf, 4);
        ZoneEffects pf = fx; pf.areaName = Q("3b · Câmara da Paranoia"); pf.subtitle = kSess + Q(" · descanso"); pf.psychic = 0.30f;
        pf.temperature = 14.f;
        w.addZone(Q("paranoia"), ZoneShape::makeBox(Vec3(-125, -206, -401), Vec3(-108, -196, -386)), pf, 4);
        w.addTrigger(Q("zumbido_entrada"), ZoneShape::makeBox(Vec3(-97, -203, -352), Vec3(-87, -197, -346)), [&w]() {
            w.state.toast(Q("Zumbido psíquico: Sabedoria ou Inteligência CD 14 — falha = 1 nível de Exaustão Psíquica."), QColor(215, 170, 255), 6.f);
        }, nullptr, true);
    }

    // ======================================================== PROPS
    {
        MeshBuilder mb;
        using namespace Prefabs;
        bonesPile(mb, Vec3(-125.5f, -201, -359), 1.6f, 11);
        bonesPile(mb, Vec3(-116.5f, -201, -368), 1.3f, 12);
        bonesPile(mb, Vec3(-127, -201, -367), 1.1f, 13);
        skull(mb, Vec3(-124.2f, -201, -358.4f), 30.f, 1.f);
        skull(mb, Vec3(-117.3f, -201, -367.3f), -60.f, 1.f);
        HumanoidParams cocoon; cocoon.pose = Pose::Lie; cocoon.skin = QColor(180, 175, 170); cocoon.cloth = QColor(150, 150, 140);
        humanoid(mb, Vec3(-124, -201, -368), 80.f, cocoon);
        HumanoidParams c2 = cocoon; c2.cloth = QColor(160, 140, 130);
        humanoid(mb, Vec3(-119.3f, -201, -358), -30.f, c2);
        for (int i = 0; i < 6; ++i) {
            float a = i * 1.1f + 0.3f;
            Vec3 p(nestC.x() + std::cos(a) * 5.8f, -201, nestC.z() + std::sin(a) * 4.8f);
            mb.ellipsoid(p + Vec3(0, 0.5f, 0), Vec3(0.55f, 0.55f, 0.55f), QColor(190, 185, 170), MAT_CLOTH, 7);
        }
        HumanoidParams dead; dead.pose = Pose::Lie; dead.skin = QColor(200, 190, 170); dead.cloth = QColor(90, 80, 70);
        humanoid(mb, Vec3(-117.5f, -203, -394.5f), -70.f, dead);
        mb.box(Vec3(-115.2f, -202.93f, -392.2f), Vec3(0.16f, 0.07f, 0.12f), QColor(120, 70, 50), MAT_PAPER);
        mb.box(Vec3(-115.2f, -202.85f, -392.2f), Vec3(0.14f, 0.03f, 0.1f), QColor(220, 210, 180), MAT_PAPER);
        sack(mb, Vec3(-120.f, -203, -391.f), 20.f, 0.8f);
        for (int i = 0; i < 14; ++i) {   // "equacoes" riscadas na parede
            float a = 0.5f + i * 0.23f;
            if (std::sin(a) > 0.6f) continue;
            Vec3 p(-117 + std::cos(a) * 5.9f, -202.2f + (i % 4) * 0.5f, -394.5f + std::sin(a) * 5.3f);
            mb.box(p, Vec3(0.28f, 0.015f, 0.015f), QColor(150, 110, 220), MAT_EMISSIVE);
        }
        w.addStatic(std::move(mb));
    }
    {
        const Vec3 lp[] = {Vec3(-92, -198.5f, -348), Vec3(-92, -198.5f, -357), Vec3(-108, -199.5f, -357), Vec3(-120, -197.5f, -364),
                           Vec3(-108, -199.5f, -370), Vec3(-92, -200.5f, -372), Vec3(-78, -200.5f, -362), Vec3(-92, -200.5f, -384),
                           Vec3(-108, -201.5f, -386), Vec3(-78, -201.5f, -386), Vec3(-70, -203.5f, -392), Vec3(-62, -205.5f, -390)};
        for (int i = 0; i < 12; ++i) {
            Light* l = w.addLight(lp[i], Vec3(0.55f, 0.28f, 1.0f), 2.4f, 9.f, LightAnim::Pulse, Q("veio"));
            l->animFreq = 0.18f + 0.04f * (i % 5); l->animDepth = 0.45f;
        }
        w.addLight(Vec3(-121, -197.f, -363), Vec3(0.7f, 0.3f, 0.9f), 3.5f, 14.f, LightAnim::Pulse, Q("ninho"))->animFreq = 0.3f;
        w.addLight(Vec3(-116.5f, -201.8f, -393), Vec3(0.4f, 0.8f, 0.55f), 1.8f, 7.f, LightAnim::Pulse, Q("paranoia"))->animFreq = 0.5f;
        w.addEmitter(ParticlePreset::Dust, Vec3(-100, -199, -364), Vec3(22, 1.5f, 18), 8.f, Q("poeira lab"));
        w.addEmitter(ParticlePreset::Spores, Vec3(-121, -198, -363), Vec3(5, 2, 4), 14.f, Q("esporos ninho"));
    }

    // ======================================================== PAREDES MUTANTES
    const Vec3 wallPos[2] = {Vec3(-92.f, -202.f + 1.7f, -377.f), Vec3(-78.f, -202.5f + 1.7f, -374.f)};
    {
        auto mut = std::make_shared<Mutant>();
        for (int i = 0; i < 2; ++i) {
            MeshBuilder wm;
            wm.box(Vec3(0, 0, 0), Vec3(2.45f, 2.0f, 0.32f), QColor(62, 48, 84), MAT_ALTERED);
            wm.box(Vec3(0, 0, 0.33f), Vec3(2.0f, 1.6f, 0.02f), QColor(95, 60, 160), MAT_ALTERED);
            wm.box(Vec3(0, 0, -0.33f), Vec3(2.0f, 1.6f, 0.02f), QColor(95, 60, 160), MAT_ALTERED);
            for (int k = -2; k <= 2; ++k) {
                wm.box(Vec3(k * 0.7f, 0.2f * (k & 1), 0.345f), Vec3(0.02f, 1.1f - 0.12f * std::abs(k), 0.012f), QColor(170, 120, 255), MAT_EMISSIVE);
                wm.box(Vec3(k * 0.7f, 0.2f * (k & 1), -0.345f), Vec3(0.02f, 1.1f - 0.12f * std::abs(k), 0.012f), QColor(170, 120, 255), MAT_EMISSIVE);
            }
            mut->wall[i] = w.addDynamic(Q("parede_mutante_%1").arg(i), std::move(wm), wallPos[i]);
            mut->col[i] = w.addBoxCollider(Q("parede_mutante_%1").arg(i), Vec3(), Vec3(2.45f, 2.0f, 0.34f), QQuaternion(), true);
            w.attachCollider(mut->col[i], mut->wall[i]);
        }
        mut->wall[1]->visible = false; mut->col[1]->enabled = false;   // inicio: A fechada, B aberta
        Vec3 wp0 = wallPos[0], wp1 = wallPos[1];
        w.addUpdater([&w, mut, wp0, wp1](float dt, float) {
            mut->cd -= dt;
            if (mut->cd > 0.f) return;
            const Vec3 pos[2] = {wp0, wp1};
            Vec3 pp = w.playerPos();
            int cur = mut->active, nxt = 1 - cur;
            if ((pp - pos[nxt]).length() < 3.2f || (pp - pos[cur]).length() < 3.2f) return;
            if ((pp - pos[0]).length() > 60.f) return;
            for (int k = 0; k < 2; ++k) {
                const Vec3 c = pos[k];
                const Vec3 smp[5] = {c, c + Vec3(0, 1.5f, 0), c + Vec3(0, -1.3f, 0), c + Vec3(1.8f, 0, 0), c + Vec3(-1.8f, 0, 0)};
                for (const Vec3& s : smp) if (w.isVisibleToPlayer(s, 70.f)) return;
            }
            mut->wall[cur]->visible = false; mut->col[cur]->enabled = false;
            mut->wall[nxt]->visible = true; mut->col[nxt]->enabled = true;
            mut->active = nxt; mut->cd = 5.f; ++mut->swaps;
        });
        w.registerEvent(Q("mutantes_swap"), Q("Trocar as Paredes Mutantes (3b)"), 0, [&w, mut]() {
            int cur = mut->active, nxt = 1 - cur;
            mut->wall[cur]->visible = false; mut->col[cur]->enabled = false;
            mut->wall[nxt]->visible = true; mut->col[nxt]->enabled = true;
            mut->active = nxt; mut->cd = 5.f;
            w.state.toast(Q("As paredes trocaram de lugar."), QColor(200, 160, 255), 2.5f);
        });
    }

    // ======================================================== DEVORADORES DE INTELECTO (teto)
    {
        auto crawlers = std::make_shared<std::vector<Crawler>>();
        auto addCrawler = [&](const QString& name, const Vec3& a, const Vec3& b, float speed, float s0) {
            MeshBuilder mb; devourerMesh(mb);
            Crawler c; c.a = a; c.b = b; c.speed = speed; c.s = s0;
            c.d = w.addDynamic(name, std::move(mb), a, QQuaternion::fromAxisAndAngle(0, 0, 1, 180.f));
            crawlers->push_back(c);
        };
        // teto = chao + kR*(1+kFF) ~ 3,44 m (pontos sobre o eixo do corredor curvo)
        Vec3 d1a = midOf(L[2], L[4], 1.5f, 1.f, 3.38f, 0.12f), d1b = midOf(L[2], L[4], 1.5f, 1.f, 3.38f, 0.88f);
        Vec3 d2a = midOf(L[7], L[9], -1.6f, 1.f, 3.38f, 0.12f), d2b = midOf(L[7], L[9], -1.6f, 1.f, 3.38f, 0.88f);
        addCrawler(Q("devorador_1"), d1a, d1b, 0.45f, 0.3f);
        addCrawler(Q("devorador_2"), d2a, d2b, 0.4f, 0.6f);
        w.addUpdater([crawlers](float dt, float t) {
            for (Crawler& c : *crawlers) {
                float len = (c.b - c.a).length();
                c.s += c.dir * c.speed * dt / len;
                if (c.s > 1.f) { c.s = 1.f; c.dir = -1.f; }
                if (c.s < 0.f) { c.s = 0.f; c.dir = 1.f; }
                Vec3 p = c.a + (c.b - c.a) * c.s;
                Vec3 h = (c.b - c.a) * c.dir; h.setY(0);
                float yaw = std::atan2(-h.x(), -h.z()) * 57.2957795f;
                c.d->pos = p + Vec3(0, 0.03f * std::sin(t * 5.f + c.a.x()), 0);
                c.d->rot = QQuaternion::fromAxisAndAngle(0, 1, 0, yaw) * QQuaternion::fromAxisAndAngle(0, 0, 1, 180.f);
                float s = 1.f + 0.05f * std::sin(t * 9.f + c.a.z());
                c.d->scl = Vec3(1, s, 1);
            }
        });
    }

    // ======================================================== GRELL (flutuando no ninho)
    {
        MeshBuilder mb; grellMesh(mb);
        DynHandle g = w.addDynamic(Q("grell"), std::move(mb), Vec3(nestC.x() + 0.5f, -198.0f, nestC.z() + 0.5f));
        const Vec3 base = g->pos;
        w.addUpdater([g, base](float, float t) {
            g->pos = base + Vec3(std::sin(t * 0.31f) * 0.6f, std::sin(t * 0.8f) * 0.18f, std::cos(t * 0.27f) * 0.5f);
            g->setYaw(std::sin(t * 0.22f) * 55.f);
        });
    }

    // ======================================================== CAMARA DA PARANOIA: pedra rolante + diario
    {
        MeshBuilder bm;
        bm.ellipsoid(Vec3(0, 0, 0), Vec3(1.4f, 1.3f, 1.35f), QColor(104, 96, 110), MAT_ALTERED, 10);
        bm.ellipsoid(Vec3(0.3f, 0.5f, 0.6f), Vec3(0.8f, 0.5f, 0.6f), QColor(124, 114, 128), MAT_ALTERED, 8);
        const Vec3 rest(-119.2f, -203 + 1.2f, -397.8f), closed(-110.9f, -203 + 1.25f, -388.4f);
        DynHandle boulder = w.addDynamic(Q("pedra_paranoia"), std::move(bm), rest);
        Collider* bc = w.addSphereCollider(Q("pedra_paranoia"), Vec3(), 1.3f);
        bc->openable = true; bc->enabled = false;
        w.attachCollider(bc, boulder);
        struct St { bool closed = false; float t = 1.f; };
        auto st = std::make_shared<St>();
        Interactable* it = w.addInteractable(Q("pedra"), rest + Vec3(0, 0.3f, 0), 1.9f, Q("Rolar a pedra para fechar a entrada"), nullptr);
        it->promptFn = [st]() { return st->closed ? Q("Rolar a pedra de volta (abrir a entrada)") : Q("Rolar a pedra para fechar a entrada"); };
        it->callback = [&w, st, it, rest, closed]() {
            if (st->t < 1.f) return;
            st->closed = !st->closed; st->t = 0.f;
            w.state.toast(st->closed ? Q("Você empurra a pedra: ela rola com um estrondo surdo e sela a entrada. O zumbido diminui.")
                                      : Q("Você afasta a pedra. O zumbido psíquico volta a arranhar os ouvidos."), QColor(210, 200, 255), 4.f);
            it->pos = (st->closed ? closed : rest) + Vec3(0, 0.3f, 0);
        };
        w.addUpdater([&w, st, boulder, bc, rest, closed](float dt, float) {
            if (st->t >= 1.f) return;
            st->t = std::min(1.f, st->t + dt / 2.2f);
            Vec3 a = st->closed ? rest : closed, b = st->closed ? closed : rest;
            float e = st->t * st->t * (3 - 2 * st->t);
            boulder->pos = a + (b - a) * e;
            boulder->rot = QQuaternion::fromAxisAndAngle(Vec3(0.6f, 0, -0.8f), st->t * 400.f * (st->closed ? 1.f : -1.f));
            if (st->t >= 1.f) {
                bc->enabled = st->closed;
                if (st->closed) w.state.screenShake(0.03f, 0.5f);
            }
        });
        auto page = std::make_shared<int>(0);
        w.addInteractable(Q("diario_sequestrado"), Vec3(-115.2f, -202.7f, -392.2f), 1.3f, Q("Ler o Diário de um Sequestrado"), [&w, page]() {
            static const QStringList pages = {
                Q("«Dia 3. Kaelen me trancou na jaula, mas a fechadura estava podre. Corri para baixo. Os túneis aqui são... errados. Lisos demais.»"),
                Q("«Dia 5. Os corredores mudam quando não olho. Contei os passos: 23. Contei de novo: 19. A matemática não mente. Eu minto.»"),
                Q("«Dia 7. Uma voz diz que sou útil. Que existe uma vaga. Que só dói no começo. Eu não quero ser útil.»"),
                Q("«Dia ?. Achei uma câmara onde a pedra fecha a porta. Vou dormir. Se alguém ler isto: não confie no Espírito da Montanha. Ele fala como um contador.»")};
            w.state.toast(pages[*page % pages.size()] + Q("  [%1/4]").arg(*page % pages.size() + 1), QColor(235, 225, 190), 8.f);
            ++*page;
        });
    }

    // ======================================================== POIs
    const QString A3b = Q("Camada 3b · O Labirinto Alienígena");
    w.addPoi(Vec3(-92, -198.6f, -348), PoiCategory::Area, Q("3b · Entrada do Labirinto (L0)"),
        Q("A fenda elemental termina aqui e a rocha muda: lisa, polida demais, com veios roxos que pulsam muito devagar. Os corredores têm ~3,5 m de largura e ~3,4 m de altura, curvas suaves que nenhuma água faria.\n\n"
          "NÓS: L0 (-92,-200,-345) · L1 (-92,-200,-355) · L2 (-108,-201,-355) · L3 Ninho do Grell (-120,-201,-362) · L4 (-108,-201,-370) · L5 (-92,-202,-370) · L6 (-78,-202,-362) · L7 (-92,-202,-384) · L8 (-108,-203,-386) → Câmara da Paranoia · L9 (-78,-203,-386) · L10 saída (-70,-205,-392).\n"
          "LIGAÇÕES: L0–L1, L1–L2, L2–L3, L2–L4, L4–L5, L1–L6, L5–L6, L5–L7, L7–L8, L7–L9, L9–L10, L6–L9.\n\n"
          "Temperatura ~12 °C (mais quente que as veias: a rocha 'respira'). Ao entrar: Sabedoria ou Inteligência CD 14, falha = 1 nível de Exaustão Psíquica; repetir a cada descanso. Magia de detecção de pensamentos mostra o labirinto 'ouvindo' o grupo."),
        kSess, A3b);
    w.addPoi(midOf(L[1], L[6], 1.2f, 1.f, 1.5f), PoiCategory::Puzzle, Q("O mapa do labirinto (solução)"),
        Q("Caminho curto: L0 → L1 → L6 → L9 → L10 (cerca de 60 m). Caminho longo (se L6–L9 estiver fechada): L1 → L6 → L5 → L7 → L9 → L10.\n\n"
          "Quem desenha um mapa percebe que as distâncias não fecham: Investigação CD 14 revela que dois corredores 'trocam' quando ninguém olha. Marcar a parede com giz não funciona (a marca some junto com a parede). Uma corda amarrada na entrada é a solução clássica (Percepção CD 12 para notar que a corda fica 'curta')."),
        kSess, A3b);
    w.addPoi(Vec3(-92, -200.6f, -376.2f), PoiCategory::Trap, Q("Parede Mutante A (L5–L7)"),
        Q("Parede de rocha alterada, de lado a lado, com veios roxos. Começa FECHADA (bloqueando L5–L7). O programa a move sozinho quando NENHUM dos dois pontos de parede está no campo de visão do jogador (testa 5 pontos de cada, com oclusão) e o jogador está a mais de 3 m. Intervalo mínimo de 5 s.\n\n"
          "Para o Mestre: no tabuleiro, qualquer jogador que 'vire as costas' ou olhe para dentro de outro corredor permite a troca. Para evitar: alguém fica olhando fixamente (o programa não troca enquanto estiver visível). Atalho do Mestre: aba Eventos > Trocar as Paredes Mutantes.\n"
          "A parede é um colisor 'abrível': passagem/Mover Terra a atravessa."),
        kSess, A3b);
    w.addPoi(Vec3(-78, -201.1f, -372.8f), PoiCategory::Trap, Q("Parede Mutante B (L6–L9)"),
        Q("Começa ABERTA: o atalho L6→L9 está livre. Quando a Parede A some, esta aparece (e vice-versa): sempre exatamente UMA das duas paredes existe.\n\n"
          "Narração sugerida: 'Você pisca. O corredor que você jurava ser reto agora termina em pedra lisa, morna, como pele.' Quem estiver no meio da troca nunca é esmagado: o programa só troca com todos a mais de 3 m."),
        kSess, A3b);
    w.addPoi(midOf(L[4], L[5], -1.8f, 1.f, 1.5f), PoiCategory::Trap, Q("Zumbido psíquico e sussurros"),
        Q("Zona psíquica em todo o labirinto (efeito visual: distorção leve + aberração cromática; mais forte no Ninho). Sussurros periódicos em português: '...a matemática não mente...', '...você é uma variável...'.\n\n"
          "MECÂNICA: ao entrar e a cada descanso, Sabedoria OU Inteligência CD 14 (o jogador escolhe). Falha = 1 nível de Exaustão Psíquica (conta como Exaustão; some com descanso longo FORA da zona). Proteção: Mente Branca / Proteção contra Mal e Bem / Pedra de Silêncio. Os sussurros são o Alhoon testando a rede de conduítes: ele ainda não 'vê' o grupo, apenas escuta pensamentos de superfície."),
        kSess, A3b);
    w.addPoi(midOf(L[2], L[4], 1.5f, 1.f, 2.6f), PoiCategory::Creature, Q("Devorador de Intelecto nº 1 (L2–L4)"),
        Q("Devorador de Intelecto (ND 2, CA 12, 21 PV, Int 7). Rasteja no teto do corredor L2–L4 (~3,4 m de altura), lentamente (0,45 m/s no programa), e cai sobre o último do grupo. Percepção passiva 14 para notar o 'tic-tic' das garras.\n\n"
          "Tática: Devorar Intelecto (CD 12 Int; falha = Int 0 e a criatura toma o corpo, atende a ordens telepáticas do Alhoon). Se o hospedeiro cair a 0 PV, a criatura sai. A vítima age normalmente na mesa; o Mestre fala com ela em segredo."),
        kSess, A3b);
    w.addPoi(midOf(L[7], L[9], -1.6f, 1.f, 2.6f), PoiCategory::Creature, Q("Devorador de Intelecto nº 2 (L7–L9)"),
        Q("Segundo Devorador, no teto do corredor L7–L9, perto da saída. Mesmo bloco de estatísticas. Se o grupo já perdeu alguém para o primeiro, este ataca quem tentar ajudar o hospedeiro.\n\nDica: tochas fracas fazem o teto parecer mais alto do que é; a criatura só aparece quando a luz alcança o teto (alcance forte ~9 m)."),
        kSess, A3b);
    w.addPoi(Vec3(-121, -198.6f, -360), PoiCategory::Creature, Q("Ninho do Grell (L3)"),
        Q("Beco sem saída: câmara de ~17 × 15 m com 9 m de altura, chão forrado de ossos e casulos de seda cinza (vítimas antigas de Kaelen). O Grell (ND 3, CA 12, 55 PV, voo 9 m) paira a ~3 m do chão, cego, e caça por vibração/eletricidade.\n\n"
          "Tática: Tentáculos (ferroada paralisante CD 11 Con por 1 min + 1d10 perfurante), prefere agarrar quem estiver parado. Imune a raio. Tesouro nos casulos: 2 poções de cura, 70 po em moedas de Kaelen, um mapa rasurado do labirinto (vantagem na CD 14 de Investigação do mapa). O casulo mais novo ainda respira: um aldeão (Brenn, 17 anos), delirante."),
        kSess, A3b);
    w.addPoi(Vec3(-116, -201.4f, -392.5f), PoiCategory::Rest, Q("Câmara da Paranoia (descanso da Sessão 4)"),
        Q("Entrada: gargalo de ~2 m a partir de L8 (-108,-203,-386), com um beco de ~10 m atrás. Uma PEDRA arredondada está no fundo do beco. [E] perto dela rola a pedra para fechar a entrada (e de novo para abrir): colisor esférico, bloqueia totalmente. Com a pedra fechada o zumbido psíquico cai muito.\n\n"
          "DESCANSO LONGO POSSÍVEL (pedra fechada). MAS: durante o descanso o Alhoon invade a mente de todos: sonhos de cápsulas de vidro, calor que não aquece, um ser de manto púrpura medindo-os. A voz do 'Espírito da Montanha' oferece alívio em troca de 'um favor': descer sozinho. Peça Sabedoria CD 14 de cada jogador; falha = acorda sem os benefícios do descanso (mantém 1 nível de exaustão) E com 1 pergunta respondida pela voz (pista honesta do Mestre sobre a Máquina do Frio).\n"
          "Se a pedra não for fechada: 1d4 horas depois, um Devorador (ou o Grell, se alertado) aparece."),
        kSess, A3b);
    w.addPoi(Vec3(-115.2f, -202.2f, -392.2f), PoiCategory::Lore, Q("Diário de um Sequestrado"),
        Q("Caderno encharcado de suor ao lado de um esqueleto de aldeão (Torvi, tecelão da vila). [E] folheia 4 páginas.\n\n"
          "Pontos para o Mestre: (1) Confirma que Kaelen é o sequestrador e que a 'oferenda' vai 'para baixo'. (2) Descreve o labirinto mudando e a camada psíquica. (3) Fala em 'uma vaga' e 'uma voz que fala como um contador' → prenúncio do Alhoon e da Vaga (5.3). (4) Mostra que a câmara é um refúgio real.\n"
          "A última frase é uma dica mecânica honesta: a pedra fecha a porta."),
        kSess, A3b);
    w.addPoi(Vec3(-70, -203.4f, -392), PoiCategory::Area, Q("3b · Saída do Labirinto (L10)"),
        Q("O corredor desce suavemente (~12°) por ~30 m até a boca do Balcão do Geodo: (-70,-205,-392) → (-55,-209,-388) → (-40,-213,-384) → balcão (-36,-214,-383). A rocha alterada vai rareando e o ar esquenta (de 12 °C para 25 °C) com cheiro de pedra molhada e metal.\n\nGuarde a revelação: de repente o corredor 'abre' em um céu de cristal (Camada 4)."),
        kSess, A3b);

    // ======================================================== PONTOS DE VISTA
    w.addViewpoint(Q("3b.0 Entrada do Labirinto (L0)"), Vec3(-92, -198.35f, -347), 0.f, -3.f, false, kGrp);
    w.addViewpoint(Q("3b.1 Nó L1 e corredores curvos"), Vec3(-92, -198.35f, -356.5f), 90.f, -2.f, false, kGrp);
    w.addViewpoint(Q("3b.2 Paredes Mutantes (L5-L7)"), Vec3(-92, -200.35f, -372.5f), 0.f, 0.f, false, kGrp);
    w.addViewpoint(Q("3b.3 Corredor L6-L9 (Parede B)"), Vec3(-78, -200.35f, -364.f), 0.f, 0.f, false, kGrp);
    w.addViewpoint(Q("3b.4 Devorador no teto (L2-L4)"), midOf(L[2], L[4], 1.5f, 1.f, 1.65f, 0.2f), 0.f, 12.f, false, kGrp);
    w.addViewpoint(Q("3b.5 Ninho do Grell (L3)"), Vec3(-114, -199.35f, -358.5f) + Vec3(0, 0, 0), 105.f, 4.f, false, kGrp);
    w.addViewpoint(Q("3b.6 Câmara da Paranoia"), Vec3(-110.8f, -201.35f, -388.4f), 52.f, -4.f, false, kGrp);
    w.addViewpoint(Q("3b.7 Saída do Labirinto (L10)"), Vec3(-69, -203.35f, -391.5f), -75.f, -4.f, false, kGrp);
    w.addViewpoint(Q("3b.8 Vista geral do labirinto (voo)"), Vec3(-92, -198.f, -371), 0.f, -25.f, true, kGrp);

    // ======================================================== ROTA
    w.addRoutePoint(Q("Entrada do Labirinto"), Vec3(-92, -199.4f, -345));
    w.addRoutePoint(Q("L1"), Vec3(-92, -199.4f, -355));
    w.addRoutePoint(Q("L6"), Vec3(-78, -201.4f, -362));
    w.addRoutePoint(Q("L9"), Vec3(-78, -202.4f, -386));
    w.addRoutePoint(Q("L10 · saída do Labirinto"), Vec3(-70, -204.4f, -392));
    w.addRoutePoint(Q("Corredor do Balcão"), Vec3(-55, -208.4f, -388));
    w.addRoutePoint(Q("Boca do Balcão"), Vec3(-40, -212.4f, -384));

    // ======================================================== SUSSURROS PSIQUICOS
    {
        struct Wh { float next = 14.f; int idx = 0; };
        auto wh = std::make_shared<Wh>();
        w.addUpdater([&w, wh](float dt, float) {
            if (w.zoneState().psychic < 0.2f) { wh->next = std::max(wh->next, 8.f); return; }
            wh->next -= dt;
            if (wh->next > 0.f) return;
            static const QStringList whispers = {
                Q("...a matemática não mente..."), Q("...você é uma variável..."), Q("...cada passo é um custo..."),
                Q("...o calor é apenas uma perda aceitável..."), Q("...quarenta e dois... quarenta e três..."),
                Q("...ele ainda está olhando..."), Q("...há uma vaga para você..."), Q("...a dor é um número..."),
                Q("...não olhe para trás... conte...")};
            w.state.toast(whispers[(wh->idx * 7 + 3) % whispers.size()], QColor(190, 150, 255), 3.5f);
            ++wh->idx;
            wh->next = 16.f + (float)((wh->idx * 5) % 9);
        });
    }
}
