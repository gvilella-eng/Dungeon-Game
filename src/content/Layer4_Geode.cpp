// CAMADA 4 — O Geodo Primordial (Sessao 5, Nivel 8)
// Elipsoide de ametista do tamanho de uma pequena cidade, sob um ceu de cristal.
// Balcao, Escadaria de Cristal, Selo dos Prismas (jogavel), Zona de Gravidade Distorcida, Campo de Geiseres,
// Mare de Magma, Camara do Ar Rarefeito e a Porta de Adamante com os 3 selos de exaustao.
#include "Dungeon.h"
#include "world/World.h"
#include "Prefabs.h"
#include "app/CommandLine.h"
#include <memory>

#define Q(s) QStringLiteral(s)

namespace {

const QString kGrp = QStringLiteral("Camada 4 · O Geodo Primordial");
const QString kSess = QStringLiteral("Sessão 5 · Nível 8");
const QString kArea = QStringLiteral("Camada 4 · O Geodo Primordial");

const Vec3 kC(20, -240, -370);          // centro (ajustado: ver notas do relatorio)
const Vec3 kRad(80, 34, 60);
const float kFloor = -255.f;
const float kBeamY = -253.5f;

struct Rng {
    uint32_t s;
    explicit Rng(uint32_t seed) : s(seed * 2654435761u + 12345u) {}
    float f() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (s & 0xFFFFFF) / 16777216.f; }
    float r(float a, float b) { return a + (b - a) * f(); }
};

QColor crystalColor(Rng& r, int alpha = 215)
{
    float k = r.f();
    if (k < 0.40f) return QColor(150 + (int)r.r(-20, 20), 70 + (int)r.r(-10, 20), 235, alpha);     // ametista
    if (k < 0.62f) return QColor(190, 100 + (int)r.r(-20, 30), 245, alpha);                         // violeta claro
    if (k < 0.82f) return QColor(80, 190 + (int)r.r(-20, 30), 250, alpha);                          // ciano
    if (k < 0.92f) return QColor(235, 130, 235, alpha);                                              // rosa
    return QColor(235, 235, 255, alpha);                                                             // quartzo
}

float roofY(float x, float z)
{
    float a = (x - kC.x()) / kRad.x(), c = (z - kC.z()) / kRad.z();
    float t = 1.f - a * a - c * c;
    return t > 0.f ? kC.y() + kRad.y() * std::sqrt(t) : kC.y();
}

// cristal torre: corpo hexagonal + ponta; up = direcao do crescimento.
void crystalTower(MeshBuilder& mb, const Vec3& base, const Vec3& up, float height, float radius, const QColor& col, int seed)
{
    Rng r((uint32_t)seed);
    Vec3 u = up.normalized();
    Vec3 body = base + u * (height * 0.8f);
    mb.cylinder(base - u * 0.5f, body, radius, radius * 0.86f, 6, col, MAT_CRYSTAL, true, false);
    QColor tip = QColor(std::min(255, col.red() + 30), std::min(255, col.green() + 30), std::min(255, col.blue() + 20), col.alpha());
    mb.cylinder(body, base + u * height, radius * 0.86f, 0.f, 6, tip, MAT_CRYSTAL, false, false);
    Vec3 side = std::fabs(u.y()) > 0.9f ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
    Vec3 t1 = Vec3::crossProduct(u, side).normalized(), t2 = Vec3::crossProduct(u, t1).normalized();
    int n = 2 + (int)(r.f() * 3.f);
    for (int i = 0; i < n; ++i) {
        float a = r.r(0, 6.283f);
        Vec3 dir = (u * r.r(0.55f, 0.85f) + (t1 * std::cos(a) + t2 * std::sin(a)) * r.r(0.35f, 0.7f)).normalized();
        Vec3 b2 = base + (t1 * std::cos(a) + t2 * std::sin(a)) * radius * 0.7f + u * (height * r.r(0.0f, 0.25f));
        float L = height * r.r(0.25f, 0.5f), rr = radius * r.r(0.3f, 0.5f);
        QColor c2 = crystalColor(r);
        mb.cylinder(b2, b2 + dir * L * 0.75f, rr, rr * 0.85f, 6, c2, MAT_CRYSTAL, true, false);
        mb.cylinder(b2 + dir * L * 0.75f, b2 + dir * L, rr * 0.85f, 0.f, 6, c2, MAT_CRYSTAL, false, false);
    }
}

QQuaternion alongZ(const Vec3& dir)
{
    Vec3 z = dir.normalized(), up(0, 1, 0);
    Vec3 x = Vec3::crossProduct(up, z);
    if (x.lengthSquared() < 1e-6f) x = Vec3(1, 0, 0);
    x.normalize();
    Vec3 y = Vec3::crossProduct(z, x);
    return QQuaternion::fromAxes(x, y, z);
}

// laje (cristal caido / rampa): a-b = linha central do TOPO; colisor + malha
void slab(World& w, MeshBuilder& mb, const QString& name, const Vec3& a, const Vec3& b, float width, float thick, const QColor& col)
{
    Vec3 d = b - a; float len = d.length();
    QQuaternion q = alongZ(d);
    Vec3 yAxis = q.rotatedVector(Vec3(0, 1, 0));
    Vec3 center = (a + b) * 0.5f - yAxis * (thick * 0.5f);
    mb.push(); mb.translate(center); mb.rotate(q);
    mb.box(Vec3(0, 0, 0), Vec3(width * 0.5f, thick * 0.5f, len * 0.5f), col, MAT_CRYSTAL);
    QColor edge(std::min(255, col.red() + 40), std::min(255, col.green() + 40), 255, col.alpha());
    mb.box(Vec3(width * 0.5f, thick * 0.35f, 0), Vec3(0.18f, thick * 0.35f, len * 0.5f), edge, MAT_CRYSTAL);
    mb.box(Vec3(-width * 0.5f, thick * 0.35f, 0), Vec3(0.18f, thick * 0.35f, len * 0.5f), edge, MAT_CRYSTAL);
    mb.pop();
    w.addBoxCollider(name, center, Vec3(width * 0.5f + 0.1f, thick * 0.5f, len * 0.5f), q, false);
}

// ponto na borda do topo de uma torre (mesma altura), r metros do centro, na direcao de 'toward'
Vec3 edgeToward(const Vec3& top, const Vec3& toward, float r)
{
    Vec3 h = toward - top; h.setY(0); h.normalize();
    return top + h * r;
}

float yawTo(const Vec3& from, const Vec3& to) { return std::atan2(-(to.x() - from.x()), -(to.z() - from.z())) * 57.2957795f; }

// ------------------------------------------------------------------ Selo dos Prismas
const Vec3 kSrc(-20, kBeamY, -340);
const Vec3 kPrism[4] = {Vec3(5, kBeamY, -340), Vec3(5, kBeamY, -365), Vec3(35, kBeamY, -365), Vec3(35, kBeamY, -405)};
const Vec3 kDecoy[2] = {Vec3(5, kBeamY, -320), Vec3(60, kBeamY, -365)};
const int kSolution[4] = {0, 2, 0, 2};          // N, L, N, L
const int kInitial[4] = {4, 6, 2, 4};           // S (vai a isca), O, L (vai a isca), S
const char* kDirName[8] = {"Norte", "Nordeste", "Leste", "Sudeste", "Sul", "Sudoeste", "Oeste", "Noroeste"};

Vec3 dirVec(int k) { float a = k * 0.785398f; return Vec3(std::sin(a), 0, -std::cos(a)); }

struct Seg { Vec3 a, b; };
struct Trace { std::vector<Seg> segs; bool hitIce = false; int endKind = 0; /*0 parede,1 isca,2 gelo*/ int endObj = -1; Vec3 end; };

bool raySphere(const Vec3& o, const Vec3& d, const Vec3& c, float r, float& t)
{
    Vec3 oc = o - c; float b = Vec3::dotProduct(oc, d), cc = Vec3::dotProduct(oc, oc) - r * r;
    float disc = b * b - cc; if (disc < 0.f) return false;
    float s = -b - std::sqrt(disc); if (s < 0.f) s = -b + std::sqrt(disc);
    if (s < 0.f) return false;
    t = s;
    return true;
}

Trace traceBeam(const World& w, const int dirs[4])
{
    Trace tr;
    Vec3 o = kSrc + Vec3(1, 0, 0) * 0.9f, d(1, 0, 0);
    Vec3 segStart = kSrc;
    int skip = -1;
    for (int it = 0; it < 9; ++it) {
        RayHit h = w.raycast(o, d, 220.f, true);
        float tw = h.hit ? h.t : 220.f;
        float best = tw; int kind = 0, obj = -1;
        for (int j = 0; j < 4; ++j) { float t; if (j != skip && raySphere(o, d, kPrism[j], 0.55f, t) && t < best) { best = t; kind = 3; obj = j; } }
        for (int j = 0; j < 2; ++j) { float t; if (raySphere(o, d, kDecoy[j], 0.55f, t) && t < best) { best = t; kind = 1; obj = j; } }
        if (kind == 3) {
            Vec3 c = kPrism[obj];
            tr.segs.push_back({segStart, c});
            segStart = c; o = c + dirVec(dirs[obj]) * 0.6f; d = dirVec(dirs[obj]); skip = obj;
            continue;
        }
        Vec3 e = kind == 1 ? kDecoy[obj] : (h.hit ? h.pos : o + d * 220.f);
        tr.segs.push_back({segStart, e}); tr.end = e;
        tr.endKind = kind;
        if (kind == 0 && h.hit && h.collider == QLatin1String("muralha_gelo")) { tr.hitIce = true; tr.endKind = 2; }
        tr.endObj = obj;
        return tr;
    }
    tr.end = segStart;
    return tr;
}

} // namespace

void buildLayer4Geode(World& w)
{
    // ======================================================== GEOMETRIA
    w.carveChamber(kC, kRad, kFloor, MAT_GEODE, 0.55f, 2.f, Q("geodo"));

    // balcao: boca do corredor (L10 -> balcao), plataforma saliente (laje de rocha) sobre o vazio
    w.carve(Prim::box(Vec3(-27, -210.6f, -383.5f), Vec3(14, 3.4f, 6.5f), QQuaternion(), 1.5f).floorAt(-214.f).noise(0.25f).smooth(1.5f).mat(MAT_GEODE).named(Q("balcao")));
    w.carve(Prim::ellipsoid(Vec3(-26, -207.f, -383.5f), Vec3(20, 9, 14)).floorAt(-214.f).noise(0.3f).smooth(2.f).mat(MAT_GEODE).named(Q("balcao_galeria")));
    w.fill(Prim::box(Vec3(-24, -216.5f, -383.5f), Vec3(7.5f, 2.5f, 6.8f), QQuaternion(), 0.6f).noise(0.2f).smooth(0.8f).mat(MAT_GEODE).named(Q("balcao_laje")));

    // passagem leste (atras da Muralha de Gelo) ate a Porta de Adamante
    w.carve(Prim::box(Vec3(88, -249.5f, -406.5f), Vec3(13.5f, 5.6f, 4.3f), QQuaternion(), 0.8f).floorAt(-255.f).noise(0.2f).smooth(1.2f).mat(MAT_GEODE).named(Q("passagem_leste")));

    // Fenda da Chama (fissura no chao) -> fonte do feixe
    w.carve(Prim::box(Vec3(-20, -258.5f, -340), Vec3(1.1f, 3.8f, 4.2f), QQuaternion(), 0.5f).noise(0.25f).smooth(0.6f).mat(MAT_HOT_BASALT).named(Q("fenda_chama")));

    // Mare de magma: leito do rio (z -410..-402), tubo de lava a oeste e sumidouro a leste
    w.carve(Prim::box(Vec3(-25, -258.5f, -406.5f), Vec3(27.5f, 3.5f, 4.3f), QQuaternion(), 0.4f).noise(0.2f).smooth(0.8f).mat(MAT_HOT_BASALT).named(Q("leito_rio")));
    w.carve(Prim::capsule(Vec3(-66, -250.5f, -406.5f), Vec3(-48, -250.5f, -406.5f), 4.4f, 4.6f).floorAt(-255.f).noise(0.3f).smooth(1.f).mat(MAT_HOT_BASALT).named(Q("tubo_lava")));
    w.carve(Prim::cylinder(Vec3(3.5f, 0, -406.5f), 3.4f, -270.f, -255.f).noise(0.25f).smooth(0.6f).mat(MAT_HOT_BASALT).named(Q("sumidouro")));

    // Camara do Ar Rarefeito: tunel ao norte do rio + camara (raio ~9)
    {
        std::vector<Vec3> pts = {Vec3(-26, -255, -409.5f), Vec3(-27, -255, -418), Vec3(-29, -255, -427)};
        for (auto& p : pts) p.setY(p.y() + 2.7f * 0.6f);
        TunnelOpts o; o.floorFrac = 0.6f; o.material = MAT_GEODE; o.noise = 0.3f; o.k = 1.5f; o.tag = Q("tunel_ar");
        w.carveTunnel(pts, 2.7f, o);
        w.carveChamber(Vec3(-30, -250.2f, -435), Vec3(9.5f, 5.8f, 9.5f), -255.f, MAT_GEODE, 0.4f, 2.f, Q("camara_ar"));
    }

    // Ninho de Cristal: bolsoes no teto sobre a Zona de Gravidade
    w.carve(Prim::sphere(Vec3(73.5f, -226.5f, -334.8f), 5.2f).noise(0.3f).smooth(1.5f).mat(MAT_GEODE).named(Q("ninho_a")));
    w.carve(Prim::sphere(Vec3(72, -222.f, -335), 3.4f).noise(0.2f).smooth(1.2f).mat(MAT_GEODE).named(Q("ninho_b")));

    // ======================================================== ZONAS
    {
        ZoneEffects fx;
        fx.areaName = kArea; fx.subtitle = kSess + Q(" · 25 °C · ametista e chama");
        fx.temperature = 25.f; fx.fogColor = Vec3(0.045f, 0.026f, 0.085f); fx.fogDensity = 0.0042f;
        fx.ambientColor = Vec3(0.12f, 0.08f, 0.23f); fx.ambientGround = Vec3(0.05f, 0.035f, 0.10f); fx.ambientStrength = 1.f;
        w.addZone(Q("geodo"), ZoneShape::makeBox(Vec3(-62, -262, -432), Vec3(102, -204, -308)), fx, 1);

        ZoneEffects b; b.areaName = Q("4.1 O Balcão do Geodo"); b.subtitle = kSess; b.temperature = 21.f;
        w.addZone(Q("balcao"), ZoneShape::makeBox(Vec3(-45, -219, -392), Vec3(-15, -205, -375)), b, 2);

        ZoneEffects st; st.areaName = Q("4.2 A Escadaria de Cristal"); st.subtitle = kSess;
        w.addZone(Q("escadaria"), ZoneShape::makeBox(Vec3(-24, -256, -412), Vec3(14, -214, -345)), st, 2);

        ZoneEffects sp; sp.areaName = Q("4.3 O Selo dos Prismas"); sp.subtitle = kSess + Q(" · quebra-cabeça de luz");
        w.addZone(Q("selo_prismas"), ZoneShape::makeBox(Vec3(-24, -258, -412), Vec3(68, -244, -330)), sp, 2);

        ZoneEffects g; g.gravity = Vec3(0, 9.81f, 0); g.areaName = Q("4.4 Zona de Gravidade Distorcida"); g.subtitle = kSess + Q(" · a gravidade aponta para o teto");
        g.fogColor = Vec3(0.06f, 0.025f, 0.10f); g.ambientColor = Vec3(0.22f, 0.12f, 0.34f);
        w.addZone(Q("gravidade"), ZoneShape::makeBox(Vec3(45, -245.5f, -360), Vec3(80, -212, -320)), g, 5);
        w.addTrigger(Q("gravidade_entrada"), ZoneShape::makeBox(Vec3(45, -245.5f, -360), Vec3(80, -212, -320)), [&w]() {
            w.state.toast(Q("Náusea: Sabedoria CD 12 ou fica atordoado 1 rodada. A gravidade agora aponta para o teto!"), QColor(240, 190, 255), 5.f);
        }, nullptr, false);

        ZoneEffects gz; gz.areaName = Q("4.5 O Campo de Gêiseres"); gz.subtitle = kSess + Q(" · vapor superaquecido"); gz.temperature = 36.f;
        gz.fogColor = Vec3(0.09f, 0.07f, 0.12f); gz.fogDensity = 0.007f;
        w.addZone(Q("geiseres"), ZoneShape::makeBox(Vec3(8, -256, -346), Vec3(52, -238, -314)), gz, 3);

        ZoneEffects mz; mz.areaName = Q("4.6 A Maré de Magma"); mz.subtitle = kSess + Q(" · o rio sobe e desce"); mz.temperature = 38.f;
        mz.fogColor = Vec3(0.09f, 0.03f, 0.02f); mz.fogDensity = 0.008f; mz.ambientColor = Vec3(0.20f, 0.09f, 0.10f);
        w.addZone(Q("mare"), ZoneShape::makeBox(Vec3(-68, -262, -415), Vec3(6, -246, -397)), mz, 3);

        ZoneEffects az; az.areaName = Q("4.7 A Câmara do Ar Rarefeito"); az.subtitle = kSess + Q(" · a chama encolhe"); az.torchScale = 0.32f; az.temperature = 17.f;
        az.fogColor = Vec3(0.05f, 0.03f, 0.04f); az.fogDensity = 0.012f; az.ambientColor = Vec3(0.09f, 0.06f, 0.10f);
        w.addZone(Q("ar_rarefeito"), ZoneShape::makeBox(Vec3(-41, -258, -446), Vec3(-14, -242, -411)), az, 4);

        ZoneEffects dz; dz.areaName = Q("4.8 A Porta de Adamante"); dz.subtitle = kSess + Q(" · selos de exaustão"); dz.temperature = 3.f;
        dz.fogColor = Vec3(0.04f, 0.07f, 0.10f); dz.fogDensity = 0.009f; dz.ambientColor = Vec3(0.07f, 0.12f, 0.17f);
        w.addZone(Q("porta"), ZoneShape::makeBox(Vec3(66, -258, -412), Vec3(101.5f, -243, -401)), dz, 4);
    }

    // ======================================================== CRISTAIS (props): paredes, chao, torres, teto
    {
        MeshBuilder cm;
        Rng rg(7);
        auto excluded = [](const Vec3& p) {
            if (p.x() < -14 && p.y() > -224 && std::fabs(p.z() + 383.5f) < 13) return true;                       // balcao
            if (p.x() > 64 && std::fabs(p.z() + 406.5f) < 9.5f && p.y() < -240) return true;                       // passagem leste
            if (p.x() < -34 && std::fabs(p.z() + 406.5f) < 8 && p.y() < -245) return true;                         // tubo de lava
            if (std::fabs(p.x() + 27) < 8 && p.z() < -407 && p.y() < -245) return true;                             // tunel do ar
            if (p.x() > 62 && p.x() < 82 && p.z() > -342 && p.z() < -326 && p.y() > -232) return true;              // ninho
            return false;
        };
        int placed = 0, tries = 0;
        std::vector<Vec3> glowPts;
        while (placed < 480 && tries < 4000) {
            ++tries;
            float u = rg.r(0.f, 6.2832f), v = rg.r(-0.55f, 0.98f);
            float cv = std::sqrt(1.f - v * v);
            Vec3 d(std::cos(u) * cv, v, std::sin(u) * cv);
            RayHit h = w.raycast(kC, d, 160.f, false);
            if (!h.hit) continue;
            Vec3 p = h.pos;
            if (excluded(p)) continue;
            bool floorHit = h.normal.y() > 0.85f && p.y() < -253.5f;
            if (floorHit && (p - Vec3(kC.x(), p.y(), kC.z())).length() < 40.f) continue;   // chao so perto das bordas
            float big = rg.f() < 0.08f ? 2.2f : 1.f;
            float rad = rg.r(0.55f, 1.5f) * big * (floorHit ? 0.7f : 1.f);
            Prefabs::crystalCluster(cm, p - h.normal * 0.2f, h.normal, rad, 4 + (int)(rg.f() * 4), placed * 13 + 5, crystalColor(rg, 255), true);
            if (rg.f() < 0.035f) glowPts.push_back(p + h.normal * 2.f);
            ++placed;
        }
        // torres de cristal (chao) com colisor, nas areas de passagem
        struct Tw { float x, z, h, r; };
        const Tw towers[] = {{-38, -352, 30, 3.4f}, {-30, -392, 36, 3.8f}, {50, -380, 22, 3.2f}, {60, -352, 26, 4.f}, {-5, -330, 24, 3.2f}, {20, -418, 30, 4.f}};
        int ti = 0;
        for (const Tw& t : towers) {
            QColor c = crystalColor(rg, 200);
            crystalTower(cm, Vec3(t.x, kFloor, t.z), Vec3(0.06f * (ti % 3 - 1), 1, 0.05f), t.h, t.r, c, 100 + ti);
            w.addCylinderCollider(Q("torre_%1").arg(ti), Vec3(t.x, kFloor + t.h * 0.4f, t.z), t.r * 0.82f, t.h * 0.4f);
            if (ti < 5) glowPts.push_back(Vec3(t.x, kFloor + t.h * 0.55f, t.z));
            ++ti;
        }
        // torres de borda (decorativas): ao longo da elipse do chao
        for (int i = 0; i < 26; ++i) {
            float a = (i + rg.r(0.1f, 0.9f)) * 6.2832f / 26.f;
            float sx = 0.93f + rg.r(-0.03f, 0.03f);
            Vec3 p(kC.x() + 72.f * sx * std::cos(a), kFloor, kC.z() + 53.f * sx * std::sin(a));
            if (excluded(Vec3(p.x(), -250, p.z())) || (p.x() < -10 && p.x() > -50 && std::fabs(p.z() + 406.f) < 14)) continue;
            float h = rg.r(10.f, 32.f);
            crystalTower(cm, p, Vec3(rg.r(-0.15f, 0.15f) - (p.x() - kC.x()) * 0.004f, 1, rg.r(-0.15f, 0.15f) - (p.z() - kC.z()) * 0.004f), h, rg.r(1.6f, 3.4f), crystalColor(rg, 200), 200 + i);
        }
        // cristais pendurados (teto)
        for (int i = 0; i < 28; ++i) {
            float x = kC.x() + rg.r(-62.f, 62.f), z = kC.z() + rg.r(-44.f, 44.f);
            float ry = roofY(x, z);
            if (ry < -225.f) continue;
            if (x > 42 && x < 82 && z > -362 && z < -318) continue;           // mesas invertidas
            if (x > -30 && x < 18 && z > -415 && z < -348) continue;           // escadaria
            if (x < -12 && z > -392 && z < -375) continue;                     // balcao
            if (std::fabs(z + 406.f) < 6.f && x > 30) continue;               // passagem leste
            crystalTower(cm, Vec3(x, ry + 1.2f, z), Vec3(0, -1, 0), rg.r(7.f, 22.f), rg.r(1.1f, 3.f), crystalColor(rg, 200), 300 + i);
        }
        w.addStatic(std::move(cm));
        // brilhos internos: luzes pulsantes violeta/ciano espalhadas
        int li = 0;
        for (const Vec3& g : glowPts) {
            if (li >= 14) break;
            bool cy = (li % 3 == 1);
            Light* l = w.addLight(g, cy ? Vec3(0.3f, 0.75f, 1.f) : Vec3(0.62f, 0.32f, 1.f), 7.f, 30.f, LightAnim::Pulse, Q("cristal"));
            l->animFreq = 0.12f + 0.03f * (li % 5); l->animDepth = 0.35f;
            ++li;
        }
        const Vec3 sky[] = {Vec3(20, -218, -370), Vec3(-25, -222, -340), Vec3(60, -222, -395), Vec3(60, -224, -340), Vec3(-30, -226, -400), Vec3(20, -224, -330), Vec3(20, -224, -410)};
        for (int i = 0; i < 7; ++i) {
            Light* l = w.addLight(sky[i], (i & 1) ? Vec3(0.35f, 0.55f, 1.f) : Vec3(0.7f, 0.35f, 1.f), 8.f, 55.f, LightAnim::Pulse, Q("ceu de cristal"));
            l->animFreq = 0.09f + 0.02f * i; l->animDepth = 0.3f;
        }
        w.addEmitter(ParticlePreset::Dust, Vec3(20, -235, -370), Vec3(60, 16, 45), 60.f, Q("poeira do geodo"))->d.activeRange = 140.f;
        w.addEmitter(ParticlePreset::Spores, Vec3(20, -240, -370), Vec3(60, 14, 45), 40.f, Q("brilho"))->d.activeRange = 140.f;
    }

    // ======================================================== BALCAO + ESCADARIA DE CRISTAL
    const Vec3 B0(-19.f, -214.f, -384.f);
    const Vec3 P1(-9.f, -224.f, -407.f), P2(8.f, -234.5f, -384.f), P3(-15.f, -245.f, -368.f), E3(1.5f, kFloor, -353.f);
    {
        MeshBuilder sm;
        const float PR = 2.5f;
        const Vec3 tops[3] = {P1, P2, P3};
        for (int i = 0; i < 3; ++i) {
            const Vec3& t = tops[i];
            float h = t.y() - kFloor;
            QColor c = (i == 1) ? QColor(110, 190, 250, 215) : QColor(165, 90, 240, 215);
            sm.cylinder(Vec3(t.x(), kFloor - 0.5f, t.z()), t, PR, PR * 0.96f, 6, c, MAT_CRYSTAL, true, false);
            sm.cylinder(Vec3(t.x(), t.y() - 0.02f, t.z()), Vec3(t.x(), t.y() + 0.18f, t.z()), PR * 0.9f, PR * 0.84f, 6, QColor(210, 190, 255, 230), MAT_CRYSTAL, true, false);
            w.addCylinderCollider(Q("torre_escada_%1").arg(i), Vec3(t.x(), kFloor + h * 0.5f - 0.1f, t.z()), PR * 0.86f, h * 0.5f + 0.1f);
            for (int k = 0; k < 6; ++k) {
                float a = k * 1.047f + i;
                Vec3 b = Vec3(t.x() + std::cos(a) * (PR + 0.5f), kFloor + 1.f + (k % 3) * h * 0.25f, t.z() + std::sin(a) * (PR + 0.5f));
                Vec3 dir(std::cos(a) * 0.5f, 1.f, std::sin(a) * 0.5f);
                sm.cylinder(b, b + dir.normalized() * (3.f + k * 0.4f), 0.5f, 0.f, 6, QColor(190, 150, 255, 220), MAT_CRYSTAL, true, false);
            }
        }
        const float in = PR - 0.5f;
        slab(w, sm, Q("escada_1"), B0, edgeToward(P1, B0, in), 3.4f, 0.8f, QColor(150, 100, 240, 220));
        slab(w, sm, Q("escada_2"), edgeToward(P1, P2, in), edgeToward(P2, P1, in), 3.4f, 0.8f, QColor(120, 170, 250, 220));
        slab(w, sm, Q("escada_3"), edgeToward(P2, P3, in), edgeToward(P3, P2, in), 3.4f, 0.8f, QColor(165, 100, 245, 220));
        slab(w, sm, Q("escada_4"), edgeToward(P3, E3, in), E3, 3.4f, 0.8f, QColor(130, 160, 250, 220));
        w.addStatic(std::move(sm));
    }

    // ======================================================== SELO DOS PRISMAS
    struct Puzzle {
        int dir[4] = {kInitial[0], kInitial[1], kInitial[2], kInitial[3]};
        DynHandle prism[4] = {nullptr, nullptr, nullptr, nullptr};
        DynHandle ice = nullptr; Collider* iceCol = nullptr;
        std::vector<Beam*> beams; EmitterHandle sparks = nullptr; Light* endLight = nullptr;
        bool melted = false; float melt = -1.f; float acc = 0.f;
        bool solved = false;
        Trace last;
    };
    auto pz = std::make_shared<Puzzle>();
    EmitterHandle steamIce = nullptr;
    {
        MeshBuilder fm;
        fm.box(Vec3(-20, -258.9f, -340), Vec3(0.8f, 0.1f, 3.6f), QColor(255, 120, 30), MAT_LAVA);
        for (int k = 0; k < 3; ++k) {
            float a = k * 2.094f + 0.5f;
            Vec3 b(-20 + std::cos(a) * 1.9f, kFloor, -340 + std::sin(a) * 1.9f);
            fm.cylinder(b, Vec3(-20 + std::cos(a) * 0.9f, kBeamY - 0.6f, -340 + std::sin(a) * 0.9f), 0.28f, 0.2f, 6, QColor(70, 56, 80), MAT_OBSIDIAN);
        }
        fm.cone(Vec3(-20, kBeamY, -340), Vec3(-20, kBeamY + 1.5f, -340), 0.45f, 6, QColor(240, 235, 255, 200), MAT_CRYSTAL);
        fm.cone(Vec3(-20, kBeamY, -340), Vec3(-20, kBeamY - 1.3f, -340), 0.45f, 6, QColor(240, 235, 255, 200), MAT_CRYSTAL);
        fm.sphere(Vec3(-20, kBeamY, -340), 0.25f, QColor(255, 230, 160), MAT_EMISSIVE, 7);
        for (int j = 0; j < 4; ++j) {
            fm.cylinder(Vec3(kPrism[j].x(), kFloor, kPrism[j].z()), Vec3(kPrism[j].x(), kBeamY - 0.75f, kPrism[j].z()), 0.62f, 0.5f, 8, QColor(58, 56, 66), MAT_OBSIDIAN);
            fm.torus(Vec3(kPrism[j].x(), kBeamY - 0.75f, kPrism[j].z()), Vec3(0, 1, 0), 0.52f, 0.05f, QColor(180, 130, 255), MAT_EMISSIVE, 16, 4);
            w.addCylinderCollider(Q("pedestal_p%1").arg(j + 1), Vec3(kPrism[j].x(), kFloor + 0.4f, kPrism[j].z()), 0.6f, 0.4f);
        }
        for (int j = 0; j < 2; ++j) {
            fm.cylinder(Vec3(kDecoy[j].x(), kFloor, kDecoy[j].z()), Vec3(kDecoy[j].x(), kBeamY - 0.75f, kDecoy[j].z()), 0.62f, 0.5f, 8, QColor(58, 56, 66), MAT_OBSIDIAN);
            fm.torus(Vec3(kDecoy[j].x(), kBeamY - 0.75f, kDecoy[j].z()), Vec3(0, 1, 0), 0.52f, 0.05f, QColor(255, 140, 110), MAT_EMISSIVE, 16, 4);
            fm.cone(Vec3(kDecoy[j].x(), kBeamY, kDecoy[j].z()), Vec3(kDecoy[j].x(), kBeamY + 0.5f, kDecoy[j].z()), 0.36f, 6, QColor(200, 160, 255, 150), MAT_CRYSTAL);
            fm.cone(Vec3(kDecoy[j].x(), kBeamY, kDecoy[j].z()), Vec3(kDecoy[j].x(), kBeamY - 0.5f, kDecoy[j].z()), 0.36f, 6, QColor(200, 160, 255, 150), MAT_CRYSTAL);
            w.addCylinderCollider(Q("pedestal_d%1").arg(j + 1), Vec3(kDecoy[j].x(), kFloor + 0.4f, kDecoy[j].z()), 0.6f, 0.4f);
        }
        w.addStatic(std::move(fm));

        w.addLight(Vec3(-20, -256.2f, -340), Vec3(1.f, 0.45f, 0.12f), 9.f, 24.f, LightAnim::TorchFlicker, Q("fenda"));
        w.addEmitter(ParticlePreset::Embers, Vec3(-20, -255.2f, -340), Vec3(0.8f, 0.1f, 3.4f), 46.f, Q("brasas da fenda"));
        w.addEmitter(ParticlePreset::Sparks, Vec3(-20, -255.f, -340), Vec3(0.5f, 0.1f, 3.f), 8.f, Q("fagulhas da fenda"));

        for (int j = 0; j < 4; ++j) {
            MeshBuilder pm;
            pm.cone(Vec3(0, 0, 0), Vec3(0, 0.5f, 0), 0.38f, 6, QColor(205, 160, 255, 190), MAT_CRYSTAL);
            pm.cone(Vec3(0, 0, 0), Vec3(0, -0.5f, 0), 0.38f, 6, QColor(205, 160, 255, 190), MAT_CRYSTAL);
            pm.cone(Vec3(0, 0, -0.3f), Vec3(0, 0, -0.78f), 0.1f, 5, QColor(255, 230, 150), MAT_EMISSIVE);
            pz->prism[j] = w.addDynamic(Q("prisma_%1").arg(j + 1), std::move(pm), kPrism[j]);
        }
        for (int i = 0; i < 10; ++i) {
            Beam* b = w.addBeam(Q("feixe_%1").arg(i), kSrc, kSrc, QColor(255, 205, 100), 0.14f);
            b->visible = false; pz->beams.push_back(b);
        }
        pz->sparks = w.addEmitter(ParticlePreset::Sparks, kSrc, Vec3(0.1f, 0.1f, 0.1f), 30.f, Q("fim do feixe"));
        pz->endLight = w.addLight(kSrc, Vec3(1.f, 0.8f, 0.4f), 3.f, 8.f, LightAnim::None, Q("luz do feixe"));

        MeshBuilder im;
        im.boxMinMax(Vec3(-1.0f, -5.4f, -4.3f), Vec3(1.0f, 5.4f, 4.3f), QColor(160, 210, 245, 130), MAT_CLEAR_ICE);
        im.boxMinMax(Vec3(-0.7f, -4.8f, -3.8f), Vec3(0.7f, 4.8f, 3.8f), QColor(210, 240, 255, 90), MAT_CLEAR_ICE);
        for (int k = 0; k < 7; ++k) im.cone(Vec3(-0.9f + 0.3f * (k % 2), -5.4f + k * 1.5f, -3.6f + k * 1.2f), Vec3(-1.2f, -5.4f + k * 1.5f + 0.9f, -3.6f + k * 1.2f), 0.3f, 5, QColor(220, 245, 255, 150), MAT_CLEAR_ICE);
        pz->ice = w.addDynamic(Q("muralha_gelo"), std::move(im), Vec3(80, -249.6f, -406.5f));
        pz->iceCol = w.addBoxCollider(Q("muralha_gelo"), Vec3(), Vec3(1.0f, 5.4f, 4.4f), QQuaternion(), true);
        w.attachCollider(pz->iceCol, pz->ice);
        w.addLight(Vec3(78.5f, -252.f, -406.5f), Vec3(0.5f, 0.8f, 1.f), 4.f, 14.f, LightAnim::Pulse, Q("gelo"))->animFreq = 0.2f;
        steamIce = w.addEmitter(ParticlePreset::Steam, Vec3(80, -252.f, -406.5f), Vec3(1.2f, 2.5f, 4.f), 0.f, Q("vapor derretimento"));
        steamIce->setEnabled(false);
    }

    for (int j = 0; j < 4; ++j) {
        Interactable* it = w.addInteractable(Q("prisma_%1").arg(j + 1), kPrism[j], 0.95f, Q("Girar o prisma (45°)"), nullptr);
        it->promptFn = [pz, j]() { return Q("Girar o prisma %1 (45°) — aponta para %2").arg(j + 1).arg(QString::fromUtf8(kDirName[pz->dir[j]])); };
        it->callback = [&w, pz, j]() {
            pz->dir[j] = (pz->dir[j] + 1) % 8;
            pz->prism[j]->setYaw(-pz->dir[j] * 45.f);
            w.state.toast(Q("Prisma %1 gira: agora aponta para %2 (%3°)").arg(j + 1).arg(QString::fromUtf8(kDirName[pz->dir[j]])).arg(pz->dir[j] * 45), QColor(225, 200, 255), 2.f);
        };
    }
    for (int j = 0; j < 2; ++j) {
        w.addInteractable(Q("isca_%1").arg(j + 1), kDecoy[j], 0.95f, Q("Examinar o cristal"), [&w]() {
            w.state.toast(Q("Este cristal está fundido ao pedestal: não gira. O feixe que o atinge morre aqui (isca)."), QColor(255, 180, 160), 4.f);
        });
    }
    for (int j = 0; j < 4; ++j) pz->prism[j]->setYaw(-pz->dir[j] * 45.f);

    w.addUpdater([&w, pz, steamIce](float dt, float t) {
        pz->acc += dt;
        for (int j = 0; j < 4; ++j) pz->prism[j]->pos = kPrism[j] + Vec3(0, 0.05f * std::sin(t * 1.3f + j), 0);
        if (pz->acc >= 0.06f) {
            pz->acc = 0.f;
            pz->last = traceBeam(w, pz->dir);
            const Trace& tr = pz->last;
            for (size_t i = 0; i < pz->beams.size(); ++i) {
                Beam* b = pz->beams[i];
                if (i < tr.segs.size()) { b->a = tr.segs[i].a; b->b = tr.segs[i].b; b->visible = true; b->color = pz->solved ? QColor(255, 235, 150) : QColor(255, 205, 100); b->intensity = pz->solved ? 2.4f : 1.6f; }
                else b->visible = false;
            }
            pz->sparks->setPosition(tr.end); pz->sparks->setEnabled(true);
            pz->endLight->pos = tr.end - Vec3(0.5f, 0, 0);
            pz->endLight->color = tr.endKind == 1 ? Vec3(1.f, 0.4f, 0.3f) : Vec3(1.f, 0.8f, 0.4f);
            if (tr.hitIce && !pz->melted) {
                pz->melted = true; pz->melt = 0.f; pz->solved = true;
                w.state.toast(Q("O feixe atinge a Muralha de Gelo Místico: ela começa a derreter!"), QColor(255, 230, 150), 5.f);
                w.state.screenShake(0.03f, 1.2f);
                steamIce->d.rate = 70.f; steamIce->setEnabled(true);
            }
        }
        if (pz->melt >= 0.f && pz->melt < 4.f) {
            pz->melt += dt;
            float s = std::max(0.02f, 1.f - pz->melt / 4.f);
            pz->ice->scl = Vec3(1, s, 1); pz->ice->pos.setY(-255.f + 5.4f * s - 0.1f);
            if (pz->melt >= 4.f) {
                pz->ice->visible = false; pz->iceCol->enabled = false; steamIce->setEnabled(false);
                w.state.toast(Q("A Muralha derreteu. O caminho leste para a Porta de Adamante está aberto."), QColor(190, 230, 255), 5.f);
            }
        }
    });
    w.registerEvent(Q("prismas_resolver"), Q("Resolver os prismas instantaneamente (4.3)"), Qt::Key_J, [&w, pz]() {
        for (int j = 0; j < 4; ++j) { pz->dir[j] = kSolution[j]; pz->prism[j]->setYaw(-pz->dir[j] * 45.f); }
        w.state.toast(Q("Mestre: os prismas se alinham sozinhos."), QColor(225, 200, 255), 3.f);
    });
    w.registerEvent(Q("prismas_reiniciar"), Q("Recongelar a Muralha e reembaralhar os prismas (4.3)"), 0, [&w, pz]() {
        for (int j = 0; j < 4; ++j) { pz->dir[j] = kInitial[j]; pz->prism[j]->setYaw(-pz->dir[j] * 45.f); }
        pz->melted = false; pz->melt = -1.f; pz->solved = false; pz->ice->visible = true; pz->ice->scl = Vec3(1, 1, 1); pz->ice->pos.setY(-249.6f); pz->iceCol->enabled = true;
        w.state.toast(Q("A Muralha de Gelo se refaz; os prismas voltam à posição inicial."), QColor(170, 220, 255), 3.f);
    });

    // ======================================================== ZONA DE GRAVIDADE: planalto, rampa, mesas invertidas, Ninho
    {
        MeshBuilder gm;
        const Vec3 plat(58, kFloor, -343);
        gm.cylinder(Vec3(plat.x(), kFloor - 0.5f, plat.z()), Vec3(plat.x(), -247.f, plat.z()), 6.5f, 6.0f, 6, QColor(150, 80, 235, 215), MAT_CRYSTAL, true, false);
        gm.cylinder(Vec3(plat.x(), -247.02f, plat.z()), Vec3(plat.x(), -246.85f, plat.z()), 5.6f, 5.4f, 6, QColor(205, 185, 255, 230), MAT_CRYSTAL, true, false);
        w.addCylinderCollider(Q("planalto"), Vec3(plat.x(), (kFloor - 0.5f - 247.f) * 0.5f, plat.z()), 5.4f, (-247.f - (kFloor - 0.5f)) * 0.5f);
        for (int k = 0; k < 5; ++k) {
            float a = k * 1.26f + 0.3f;
            Vec3 b(plat.x() + std::cos(a) * 4.6f, -246.9f, plat.z() + std::sin(a) * 4.6f);
            Prefabs::crystalCluster(gm, b, Vec3(0, 1, 0), 0.7f, 5, 40 + k, QColor(120, 190, 255), true);
        }
        Vec3 rs(30, kFloor, -356), re(plat.x() - 3.3f, -247.f, plat.z() - 1.9f);
        slab(w, gm, Q("rampa_planalto"), rs, re, 3.4f, 0.8f, QColor(140, 170, 250, 220));
        // mesas invertidas (estalagmites penduradas do teto): base em -238, degraus de 2,4 m rumo ao teto
        struct Ms { float x, z, r, bottom; };
        const Ms mesas[] = {{58, -343, 5.5f, -238.f}, {62.4f, -340.2f, 1.6f, -235.6f}, {65.2f, -338.6f, 1.6f, -233.2f}, {67.8f, -337.4f, 1.6f, -230.8f},
                            {70.2f, -337.2f, 1.6f, -228.4f}, {72.f, -337.f, 1.6f, -226.f}, {73.2f, -334.f, 1.3f, -223.6f}, {71.f, -335.6f, 1.3f, -221.2f}};
        int mi = 0;
        for (const Ms& m : mesas) {
            float top = roofY(m.x, m.z) + 1.5f;
            if (mi >= 6) top = -217.f;
            QColor c = (mi % 2) ? QColor(110, 190, 250, 215) : QColor(170, 95, 240, 215);
            gm.cylinder(Vec3(m.x, m.bottom, m.z), Vec3(m.x, top, m.z), m.r, m.r * 1.05f, 6, c, MAT_CRYSTAL, true, false);
            gm.cylinder(Vec3(m.x, m.bottom + 0.15f, m.z), Vec3(m.x, m.bottom - 0.02f, m.z), m.r * 0.9f, m.r * 0.95f, 6, QColor(215, 195, 255, 235), MAT_CRYSTAL, true, false);
            w.addCylinderCollider(Q("mesa_%1").arg(mi), Vec3(m.x, (m.bottom + top) * 0.5f, m.z), m.r * 0.86f, (top - m.bottom) * 0.5f);
            ++mi;
        }
        // Ninho de Cristal: cristais de brasa e a Escama (de cabeca para baixo)
        const Vec3 nest(71.f, -221.2f, -335.6f);
        for (int k = 0; k < 6; ++k) {
            float a = k * 1.047f;
            Vec3 b(72 + std::cos(a) * 3.0f, -219.2f, -335 + std::sin(a) * 3.0f);
            Prefabs::crystalCluster(gm, b, Vec3(0, -1, 0), 0.6f, 4, 70 + k, QColor(255, 140, 90), true);
        }
        gm.ellipsoid(nest + Vec3(0, -0.12f, 0), Vec3(0.34f, 0.07f, 0.26f), QColor(120, 40, 28), MAT_HOT_BASALT, 8);
        gm.ellipsoid(nest + Vec3(0, -0.18f, 0), Vec3(0.26f, 0.04f, 0.19f), QColor(255, 110, 40), MAT_EMISSIVE, 8);
        w.addStatic(std::move(gm));
        w.addLight(Vec3(72, -224.f, -335), Vec3(1.f, 0.5f, 0.2f), 4.f, 12.f, LightAnim::Pulse, Q("escama"))->animFreq = 0.3f;
        w.addLight(Vec3(60, -240.f, -343), Vec3(0.7f, 0.4f, 1.f), 6.f, 22.f, LightAnim::Pulse, Q("planalto"))->animFreq = 0.2f;
        w.addInteractable(Q("escama_dragao"), Vec3(71.f, -222.6f, -335.6f), 1.6f, Q("Pegar a Escama do Dragão Ancestral"), [&w]() {
            if (w.state.hasItem(Q("Escama do Dragão Ancestral"))) { w.state.toast(Q("A escama já está com você; ainda morna."), QColor(255, 200, 150), 3.f); return; }
            w.state.addItem(Q("Escama do Dragão Ancestral"));
            w.state.toast(Q("Escama do Dragão Ancestral: morna ao toque. Resistência a dano de fogo enquanto estiver atunada."), QColor(255, 190, 120), 6.f);
        });
    }

    // ======================================================== CAMPO DE GEISERES
    {
        struct Gey { Vec3 pos; int order = 0; EmitterHandle em = nullptr; Light* l = nullptr; Zone* z = nullptr; };
        auto geys = std::make_shared<std::vector<Gey>>();
        MeshBuilder gm;
        const int ord[3][3] = {{0, 1, 2}, {5, 4, 3}, {6, 7, 8}};
        const float xs[3] = {15, 30, 45}, zs[3] = {-322, -329, -336};
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col) {
                Gey g; g.pos = Vec3(xs[col], kFloor, zs[row]); g.order = ord[row][col];
                gm.torus(Vec3(g.pos.x(), kFloor + 0.05f, g.pos.z()), Vec3(0, 1, 0), 0.85f, 0.22f, QColor(70, 56, 50), MAT_OBSIDIAN, 14, 5);
                gm.cylinder(Vec3(g.pos.x(), kFloor - 0.05f, g.pos.z()), Vec3(g.pos.x(), kFloor + 0.04f, g.pos.z()), 0.68f, 0.68f, 12, QColor(255, 120, 40), MAT_LAVA);
                EmitterDesc d; d.preset = ParticlePreset::Steam; ParticleEmitter::applyPreset(d);
                d.pos = g.pos + Vec3(0, 0.3f, 0); d.halfExtent = Vec3(0.35f, 0.05f, 0.35f); d.rate = 0.f;
                d.velocity = Vec3(0, 8.5f, 0); d.velJitter = 1.2f; d.lifeMin = 1.1f; d.lifeMax = 1.8f; d.sizeMin = 0.7f; d.sizeMax = 1.5f; d.turbulence = 0.7f;
                d.colorStart = QColor(235, 230, 240, 120); d.colorEnd = QColor(235, 230, 240, 0); d.activeRange = 130.f;
                g.em = w.addEmitter(d);
                g.l = w.addLight(g.pos + Vec3(0, 1.f, 0), Vec3(1.f, 0.55f, 0.25f), 0.f, 12.f, LightAnim::None, Q("geiser"));
                ZoneEffects hz; hz.hazard = true; hz.hazardText = Q("Gêiser de vapor superaquecido: Constituição CD 14 ou 4d6 de dano de fogo!");
                g.z = w.addZone(Q("hazard_geiser_%1").arg(g.order), ZoneShape::makeBox(g.pos + Vec3(-1.5f, -0.5f, -1.5f), g.pos + Vec3(1.5f, 13.f, 1.5f)), hz, 7);
                g.z->enabled = false;
                geys->push_back(g);
            }
        w.addStatic(std::move(gm));
        w.addUpdater([geys](float, float t) {
            const float cyc = 12.f, slot = cyc / 9.f;
            float c = std::fmod(t, cyc);
            for (Gey& g : *geys) {
                float st = g.order * slot;
                float k = std::fmod(c - st + cyc, cyc);
                bool firing = k < 2.0f;
                float tele = std::fmod(st - c + cyc, cyc);
                bool telegraph = !firing && tele < 0.8f;
                g.z->enabled = firing && k > 0.25f;
                g.em->d.rate = firing ? 95.f : (telegraph ? 10.f : 0.f);
                g.em->setEnabled(firing || telegraph);
                g.l->intensity = firing ? 6.f : (telegraph ? 1.5f : 0.f);
            }
        });
    }

    // ======================================================== MARE DE MAGMA
    {
        MeshBuilder lm; lm.setFlow(1.f, 0.f);
        lm.boxMinMax(Vec3(-27.5f, -1.f, -4.f), Vec3(27.5f, 0.f, 4.f), QColor(215, 70, 12), MAT_LAVA);
        DynHandle lava = w.addDynamic(Q("mare_lava"), std::move(lm), Vec3(-25, -255.25f, -406.5f));
        MeshBuilder tm; tm.setFlow(1.f, 0.f);
        tm.boxMinMax(Vec3(-66.f, -255.9f, -410.5f), Vec3(-52.5f, -255.3f, -402.5f), QColor(210, 65, 10), MAT_LAVA);
        tm.boxMinMax(Vec3(1.8f, -266.f, -409.f), Vec3(5.2f, -265.4f, -404.f), QColor(255, 90, 15), MAT_LAVA);
        w.addStatic(std::move(tm));
        MeshBuilder sm;
        const float sz[3] = {-403.4f, -406.f, -408.6f};
        for (int i = 0; i < 3; ++i) {
            sm.cylinder(Vec3(-25, -258.6f, sz[i]), Vec3(-25, -254.9f, sz[i]), 1.1f, 0.95f, 7, QColor(52, 40, 40), MAT_OBSIDIAN, true, false);
            w.addCylinderCollider(Q("pedra_travessia_%1").arg(i), Vec3(-25, -256.7f, sz[i]), 0.95f, 1.8f);
        }
        w.addStatic(std::move(sm));
        ZoneEffects hz; hz.hazard = true; hz.hazardText = Q("Magma: 10d10 de dano de fogo!"); hz.temperature = 70.f;
        Zone* hazard = w.addZone(Q("hazard_magma"), ZoneShape::makeBox(Vec3(-66, -268, -410.3f), Vec3(3, -254.5f, -402.7f)), hz, 8);
        Light* ll = w.addLight(Vec3(-25, -253.5f, -406.5f), Vec3(1.f, 0.4f, 0.1f), 8.f, 26.f, LightAnim::Pulse, Q("rio de magma"));
        ll->animFreq = 0.4f; ll->animDepth = 0.2f;
        w.addLight(Vec3(-52, -253.f, -406.5f), Vec3(1.f, 0.4f, 0.1f), 6.f, 18.f, LightAnim::Pulse, Q("tubo de lava"));
        w.addEmitter(ParticlePreset::Embers, Vec3(-25, -254.5f, -406.5f), Vec3(26.f, 0.2f, 3.5f), 60.f, Q("brasas do rio"));
        w.addEmitter(ParticlePreset::Steam, Vec3(3.5f, -255.f, -406.5f), Vec3(1.5f, 0.1f, 1.5f), 14.f, Q("vapor do sumidouro"));
        w.addUpdater([lava, hazard, ll](float, float t) {
            float level = -255.25f + 0.75f * std::sin(t * 6.2832f / 40.f);
            lava->pos.setY(level);
            hazard->shape.box = Aabb(Vec3(-66, -268, -410.3f), Vec3(3, level + 0.85f, -402.7f));
            ll->pos.setY(level + 1.2f);
        });
    }

    // ======================================================== CAMARA DO AR RAREFEITO
    {
        MeshBuilder am;
        am.cylinder(Vec3(-30, -255, -435), Vec3(-30, -254.2f, -435), 0.7f, 0.55f, 8, QColor(62, 52, 58), MAT_OBSIDIAN);
        am.sphere(Vec3(-30, -253.95f, -435), 0.22f, QColor(255, 90, 30), MAT_EMISSIVE, 8);
        am.cone(Vec3(-30, -253.95f, -435), Vec3(-30, -253.45f, -435), 0.16f, 6, QColor(255, 160, 60), MAT_EMISSIVE);
        Rng r(99);
        for (int i = 0; i < 12; ++i) {
            float a = r.r(0, 6.28f), d = r.r(2.5f, 7.5f);
            am.ellipsoid(Vec3(-30 + std::cos(a) * d, -254.6f, -435 + std::sin(a) * d), Vec3(r.r(0.3f, 0.9f), r.r(0.3f, 0.7f), r.r(0.3f, 0.9f)), QColor(95, 75, 105), MAT_OBSIDIAN, 6);
        }
        for (int i = 0; i < 6; ++i) {
            float a = r.r(0, 6.28f), d = r.r(5.f, 8.f);
            Prefabs::crystalCluster(am, Vec3(-30 + std::cos(a) * d, -255.f, -435 + std::sin(a) * d), Vec3(0, 1, 0), 0.8f, 5, 500 + i, QColor(170, 110, 240), true);
        }
        w.addStatic(std::move(am));
        w.addLight(Vec3(-30, -253.3f, -435), Vec3(1.f, 0.4f, 0.12f), 3.f, 8.f, LightAnim::TorchFlicker, Q("pedra de fogo"));
        w.addLight(Vec3(-30, -248, -435), Vec3(0.6f, 0.35f, 0.9f), 3.f, 18.f, LightAnim::Pulse, Q("camara ar"));
        w.addInteractable(Q("pedra_fogo"), Vec3(-30, -253.8f, -435), 1.5f, Q("Pegar a Pedra de Fogo Elemental"), [&w]() {
            if (w.state.hasItem(Q("Pedra de Fogo Elemental"))) { w.state.toast(Q("A pedra já está com você."), QColor(255, 200, 150), 3.f); return; }
            w.state.addItem(Q("Pedra de Fogo Elemental"));
            w.state.toast(Q("Pedra de Fogo Elemental: granada de Bola de Fogo (3º nível). Aqui, a área seria dobrada: não use!"), QColor(255, 190, 120), 6.f);
        });
    }

    // ======================================================== PORTA DE ADAMANTE + SELOS DE EXAUSTAO
    {
        struct Door {
            DynHandle door = nullptr; Collider* col = nullptr;
            DynHandle gear[3] = {nullptr, nullptr, nullptr}, hot[3] = {nullptr, nullptr, nullptr}, frost[3] = {nullptr, nullptr, nullptr};
            bool done[3] = {false, false, false};
            int count = 0; int paid = 0;
            float sink = -1.f;
            Light* gl[3] = {nullptr, nullptr, nullptr};
        };
        auto dr = std::make_shared<Door>();
        const Vec3 doorC(98, -250.f, -406.5f);
        MeshBuilder dm;
        dm.box(Vec3(0, 0, 0), Vec3(0.6f, 5.0f, 4.2f), QColor(34, 58, 50), MAT_ADAMANTINE);
        dm.box(Vec3(-0.62f, 0, 0), Vec3(0.08f, 4.8f, 4.0f), QColor(26, 44, 40), MAT_ADAMANTINE);
        for (int k = -3; k <= 3; ++k) dm.box(Vec3(-0.7f, 0, k * 1.2f), Vec3(0.06f, 4.8f, 0.07f), QColor(60, 110, 90), MAT_ADAMANTINE);
        for (int k = -4; k <= 4; ++k) dm.box(Vec3(-0.7f, k * 1.1f, 0), Vec3(0.06f, 0.07f, 3.9f), QColor(60, 110, 90), MAT_ADAMANTINE);
        for (int i = 0; i < 18; ++i) dm.sphere(Vec3(-0.72f, -4.4f + (i / 3) * 1.7f, -3.5f + (i % 3) * 3.5f), 0.1f, QColor(80, 140, 120), MAT_ADAMANTINE, 5);
        dm.box(Vec3(-0.72f, 4.3f, 0), Vec3(0.04f, 0.18f, 3.2f), QColor(120, 255, 200), MAT_EMISSIVE);
        dr->door = w.addDynamic(Q("porta_adamante"), std::move(dm), doorC);
        dr->col = w.addBoxCollider(Q("porta_adamante"), Vec3(), Vec3(0.62f, 5.0f, 4.4f), QQuaternion(), true);
        w.attachCollider(dr->col, dr->door);
        const Vec3 gp[3] = {Vec3(97.2f, -250.2f, -409.3f), Vec3(97.2f, -252.8f, -406.5f), Vec3(97.2f, -250.2f, -403.7f)};
        for (int i = 0; i < 3; ++i) {
            MeshBuilder gm; Prefabs::gear(gm, Vec3(0, 0, 0), Vec3(1, 0, 0), 1.15f, 14, 0.3f, MAT_IRON);
            gm.cylinder(Vec3(0.1f, 0, 0), Vec3(-0.35f, 0, 0), 0.22f, 0.22f, 8, QColor(80, 80, 90), MAT_IRON);
            dr->gear[i] = w.addDynamic(Q("engrenagem_selo_%1").arg(i + 1), std::move(gm), gp[i]);
            MeshBuilder hm; Prefabs::gear(hm, Vec3(0, 0, 0), Vec3(1, 0, 0), 1.15f, 14, 0.3f, MAT_EMISSIVE);
            dr->hot[i] = w.addDynamic(Q("engrenagem_quente_%1").arg(i + 1), std::move(hm), gp[i] + Vec3(-0.03f, 0, 0));
            dr->hot[i]->visible = false;
            MeshBuilder fm;
            fm.cylinder(Vec3(-0.2f, 0, 0), Vec3(0.15f, 0, 0), 1.3f, 1.3f, 14, QColor(200, 235, 255, 150), MAT_CLEAR_ICE);
            for (int k = 0; k < 10; ++k) {
                float a = k * 0.628f;
                fm.cone(Vec3(-0.25f, std::cos(a) * 1.2f, std::sin(a) * 1.2f), Vec3(-0.55f, std::cos(a) * 1.3f, std::sin(a) * 1.3f), 0.14f, 5, QColor(225, 245, 255, 190), MAT_CLEAR_ICE);
            }
            dr->frost[i] = w.addDynamic(Q("geada_%1").arg(i + 1), std::move(fm), gp[i]);
            dr->gl[i] = w.addLight(gp[i] + Vec3(-1.f, 0, 0), Vec3(1.f, 0.25f, 0.1f), 0.f, 9.f, LightAnim::Pulse, Q("engrenagem quente"));
            Interactable* it = w.addInteractable(Q("engrenagem_selo_%1").arg(i + 1), gp[i] + Vec3(-0.6f, 0, 0), 1.3f, Q("Transferir calor vital (1 Dado de Vida ou 1 nível de Exaustão)"), nullptr);
            it->promptFn = [dr, i]() -> QString {
                if (dr->done[i]) return Q("Engrenagem girada");
                bool hd = (dr->paid % 2 == 0);
                return Q("Transferir calor vital — custo desta engrenagem: %1").arg(hd ? Q("1 Dado de Vida") : Q("1 nível de Exaustão"));
            };
            it->callback = [&w, dr, i]() {
                if (dr->done[i]) return;
                bool hd = (dr->paid % 2 == 0);
                dr->done[i] = true; ++dr->count; ++dr->paid;
                if (hd) { w.state.hitDiceSpent++; w.state.toast(Q("Você transfere calor vital: -1 Dado de Vida. A engrenagem esquenta, avermelha e gira! (%1/3)").arg(dr->count), QColor(255, 170, 120), 5.f); }
                else { w.state.addExhaustion(1); w.state.toast(Q("O calor foge do seu corpo: +1 nível de Exaustão. A engrenagem esquenta, avermelha e gira! (%1/3)").arg(dr->count), QColor(255, 140, 110), 5.f); }
                dr->frost[i]->visible = false; dr->gear[i]->visible = false; dr->hot[i]->visible = true;
                dr->gl[i]->intensity = 6.f;
                w.state.screenShake(0.03f, 0.6f);
                if (dr->count == 3) {
                    dr->sink = 0.f;
                    w.state.toast(Q("As três engrenagens giram juntas: com um rangido de metal, a Porta de Adamante afunda no chão."), QColor(210, 255, 230), 6.f);
                }
            };
        }
        w.addUpdater([dr, doorC, gp](float dt, float t) {
            for (int i = 0; i < 3; ++i) if (dr->done[i]) dr->hot[i]->setRotation(QQuaternion::fromAxisAndAngle(1, 0, 0, t * 70.f * (i == 1 ? -1.f : 1.f)));
            if (dr->sink >= 0.f && dr->sink < 6.f) {
                dr->sink += dt;
                float e = std::min(1.f, dr->sink / 6.f); e = e * e * (3 - 2 * e);
                dr->door->pos = doorC + Vec3(0, -10.6f * e, 0);
                for (int i = 0; i < 3; ++i) { dr->hot[i]->pos = gp[i] + Vec3(-0.03f, -10.6f * e, 0); dr->gl[i]->pos = gp[i] + Vec3(-1.f, -10.6f * e, 0); }
                if (dr->sink >= 6.f) { dr->col->enabled = false; for (int i = 0; i < 3; ++i) dr->hot[i]->visible = false; }
            }
        });
        w.registerEvent(Q("porta_abrir"), Q("Girar os 3 selos / abrir a Porta de Adamante (4.8)"), Qt::Key_X, [&w, dr]() {
            if (dr->sink >= 0.f) return;
            for (int i = 0; i < 3; ++i) {
                dr->done[i] = true; dr->frost[i]->visible = false; dr->gear[i]->visible = false; dr->hot[i]->visible = true; dr->gl[i]->intensity = 6.f;
            }
            dr->count = 3; dr->sink = 0.f;
            w.state.toast(Q("Mestre: os três selos giram. A Porta de Adamante afunda no chão."), QColor(210, 255, 230), 5.f);
        });
        w.addLight(Vec3(94.5f, -249.f, -406.5f), Vec3(0.45f, 0.8f, 1.f), 5.f, 18.f, LightAnim::Pulse, Q("porta"))->animFreq = 0.15f;
        w.addEmitter(ParticlePreset::Steam, Vec3(95.5f, -254.5f, -406.5f), Vec3(1.2f, 0.1f, 4.f), 6.f, Q("frio da porta"));
        MeshBuilder pm;
        Rng r(55);
        for (int i = 0; i < 10; ++i) {
            Vec3 b(94 + r.r(0, 3), -255, -410 + r.r(0, 7));
            pm.cone(b, b + Vec3(0, r.r(0.4f, 1.3f), 0), r.r(0.15f, 0.35f), 5, QColor(210, 240, 255, 180), MAT_CLEAR_ICE);
        }
        w.addStatic(std::move(pm));
    }

    // ======================================================== POIs
    auto poi = [&](const Vec3& p, PoiCategory c, const QString& t, const QString& txt) { w.addPoi(p, c, t, txt, kSess, kArea); };
    poi(Vec3(20, -250.f, -370), PoiCategory::Area, Q("4 · O Geodo Primordial (visão geral)"),
        Q("Câmara do tamanho de uma pequena cidade: dentro de um geodo gigante. No programa: elipsoide de centro (20,-240,-370) e raios (80,34,60) — ajustado em relação à bíblia (centro -232, raio vertical 26) para que o chão plano em y -255 tenha ~140 × 105 m e comporte todas as áreas; teto em ~-206. Paredes de ametista com centenas de cristais, torres de cristal de 10–36 m e cristais pendurados.\n\n"
          "LUZ: o geodo brilha por dentro (pulsos lentos violeta e ciano); a Fenda da Chama e o rio de magma dão luz quente. Temperatura ~25 °C, ar limpo (visibilidade >100 m): a escala é a emoção. A tocha é quase desnecessária: o Mestre pode narrar que o brilho 'não aquece nem queima, mas pesa nos olhos'.\n\n"
          "ARMADILHAS = SISTEMA IMUNOLÓGICO do Coração do Dragão: anomalias elementais moldadas pelo subconsciente da fera. Nenhuma criatura aqui é 'inteligente'; os perigos são o próprio ambiente (gravidade, vapor, magma, ar rarefeito, gelo).\n\n"
          "ÁREAS: 4.1 Balcão · 4.2 Escadaria · 4.3 Selo dos Prismas · 4.4 Gravidade Distorcida · 4.5 Gêiseres · 4.6 Maré de Magma · 4.7 Ar Rarefeito · 4.8 Porta de Adamante.\nAtalhos do Mestre: J = resolver os prismas · X = girar os 3 selos."));
    poi(Vec3(-34, -212.6f, -383.5f), PoiCategory::Area, Q("4.1 O Balcão do Geodo"),
        Q("O corredor do labirinto desemboca em uma laje de rocha saliente na parede oeste, 41 m acima do chão. A PRIMEIRA VISTA do geodo deve ser de tirar o fôlego: deixe os jogadores chegarem à borda antes de descrever.\n\n"
          "Narração sugerida: 'O corredor liso termina no vazio. Abaixo de vocês, quarenta metros de ar e uma floresta de torres de cristal que brilham como brasas frias. No teto, estalactites de ametista do tamanho de catedrais. E no centro do chão, uma fenda de fogo.'\n\n"
          "Queda da laje: 13d6 (41 m, máx. 20d6). Vertigem: Atletismo/Acrobacia CD 10 para não escorregar ao se debruçar. Ponto de descanso curto razoável (corredor atrás)."));
    poi(Vec3(-9, -222.6f, -407), PoiCategory::Area, Q("4.2 A Escadaria de Cristal"),
        Q("Zigue-zague de cristais gigantes caídos apoiados em topos de torres de cristal: balcão → P1 (-9,-224,-407) → P2 (8,-234.5,-384) → P3 (-15,-245,-368) → chão (1.5,-255,-353). Cada rampa tem 3,4 m de largura e ≈25–28° (posições levemente deslocadas da bíblia para manter a inclinação suave com os raios reais das torres).\n\n"
          "Não há corrimão: nos topos das torres (platôs hexagonais de ~5 m) o vazio é de 10–30 m. Bom lugar para um combate vertical (criaturas voadoras, mephits de Camada 3, empurrões). Correr nos topos: Acrobacia CD 10."));
    poi(Vec3(-20, -254.4f, -337), PoiCategory::Puzzle, Q("4.3 A Fenda da Chama (fonte do feixe)"),
        Q("Fissura de ~2 × 8 m no chão em (-20,-255,-340): lava, brasas e fagulhas ao fundo. Acima dela, 3 pilares de obsidiana seguram um CRISTAL COLETOR de quartzo que converte o fogo do dragão em um FEIXE DE LUZ dourado, horizontal, a y -253,5, para o leste.\n\n"
          "Perigo: calor intenso (CD 10 Con a cada 10 min a menos de 3 m) — mantenha os jogadores a 3+ m. Descrição: 'o feixe tem a espessura de um braço e não faz som; mas as chamas das tochas se curvam em direção a ele'."));
    poi(kPrism[0] + Vec3(0, 0.9f, 0), PoiCategory::Puzzle, Q("4.3 O Selo dos Prismas — como funciona"),
        Q("Quebra-cabeça de luz totalmente jogável. [E] em um prisma o gira 45° (a ponta dourada mostra para onde ele lança o feixe). O feixe nasce na Fenda (-20,-340), vai para o leste e, a cada prisma atingido, SAI na direção para a qual o prisma aponta (calculado em tempo real, com raycast contra a rocha, colisores e os outros cristais).\n\n"
          "PRISMAS: P1 (5,-340) · P2 (5,-365) · P3 (35,-365) · P4 (35,-405). ISCAS (cristais fundidos que não giram): D1 (5,-320) e D2 (60,-365). ALVO: Muralha de Gelo Místico em (80,-255,-406).\n"
          "SOLUÇÃO: P1 → NORTE · P2 → LESTE · P3 → NORTE · P4 → LESTE. Estado inicial (embaralhado): P1 Sul (vai à isca D1), P2 Oeste, P3 Leste (vai à isca D2), P4 Sul.\n\n"
          "TEMPO: cada tentativa errada consome 10 min de jogo (tochas). Versão Shadowdark: os jogadores podem usar a luz das próprias tochas que estão acabando como fonte alternativa. Dicas graduais: Investigação CD 12 → o feixe só muda quando um prisma é atingido; CD 15 → as pontas douradas indicam a saída; CD 18 → as iscas têm base diferente (sem runa).\nAtalho do Mestre: [J] resolve tudo; a aba Eventos tem o botão de reembaralhar."));
    poi(kPrism[1] + Vec3(0, 0.9f, 0), PoiCategory::Puzzle, Q("4.3 Prisma P2"), Q("(5,-365). Recebe o feixe vindo do norte de P1 (z -340 → -365). Posição correta: LESTE (aponta para P3 em (35,-365)). Se estiver para o Oeste, o feixe bate na parede oeste do geodo."));
    poi(kPrism[2] + Vec3(0, 0.9f, 0), PoiCategory::Puzzle, Q("4.3 Prisma P3"), Q("(35,-365). Posição correta: NORTE (aponta para P4 em (35,-405)). Posição Leste: o feixe vai à ISCA D2 (60,-365) e morre, com um clarão vermelho."));
    poi(kPrism[3] + Vec3(0, 0.9f, 0), PoiCategory::Puzzle, Q("4.3 Prisma P4"), Q("(35,-405). Posição correta: LESTE — o feixe percorre os 45 m até a Muralha de Gelo (80,-406). Quando acerta, o gelo encolhe por ~4 s, solta vapor e some. A Muralha é um colisor 'abrível' (magia de calor forte também a derrete, a critério do Mestre)."));
    poi(kDecoy[0] + Vec3(0, 0.9f, 0), PoiCategory::Trap, Q("4.3 Isca D1 (5,-320)"), Q("Cristal fundido ao pedestal (parece um prisma, mas não gira). Recebe o feixe se P1 apontar para o SUL. O feixe morre com clarão avermelhado. Investigação CD 14 (ou Detectar Magia) mostra que é apenas 'espelho morto'."));
    poi(kDecoy[1] + Vec3(0, 0.9f, 0), PoiCategory::Trap, Q("4.3 Isca D2 (60,-365)"), Q("Isca no caminho natural de P3 apontado para o LESTE: o erro mais comum. O feixe morre aqui."));
    poi(Vec3(78.5f, -252.f, -406.5f), PoiCategory::Puzzle, Q("4.3 A Muralha de Gelo Místico"),
        Q("Lâmina de gelo de 2 × 10,8 × 8,6 m que sela a passagem leste em (80,-255,-406). Gelo mágico (criado pelo Alhoon): imune a dano comum; dano de fogo forte (40 PV, CA 10) também a dissolve, mas chamas comuns só a arranham. O feixe correto a derrete sem custo. Variante do Mestre: a mesma mecânica derrete rios de magma solidificado."));
    poi(Vec3(60, -245.8f, -343), PoiCategory::Trap, Q("4.4 A Zona de Gravidade Distorcida"),
        Q("Região leste: caixa x 45..80, y −245,5..−212, z −360..−320, onde a GRAVIDADE APONTA PARA O TETO. Do chão, uma rampa de cristal (3,4 m) leva ao PLANALTO DE CRISTAL (58,-247,-343), 8 m acima do chão. Pular do planalto faz o personagem 'cair para cima' e pousar nas MESAS INVERTIDAS (estalagmites penduradas do teto) com base em -238.\n\n"
          "Ao entrar: Sabedoria CD 12 ou fica atordoado 1 rodada (náusea). A câmera gira suavemente. Cair para o teto sem mesa: ~22 m = 7d6. Saltar para fora da zona inverte a gravidade de volta (queda de até 39 m: 13d6 — Queda Suave/Levitação/corda ajudam). Ninguém é obrigado a entrar: o Ninho é opcional."));
    poi(Vec3(60, -245.8f, -334), PoiCategory::Area, Q("4.4 Planalto e Mesas Invertidas"),
        Q("Planalto hexagonal de ~11 m de diâmetro, topo em y -247. Sobre ele, a mesa invertida principal (r 5,5; base -238) e uma escada de 7 mesas menores (degraus de 2,4 m 'para baixo', rumo ao teto) até o Ninho. Degraus de 2,4 m não causam dano (<3 m). Andar de cabeça para baixo: o programa gira a câmera e o 'chão' é a face inferior das mesas.\n\nRetorno: só por cima (levitação/queda suave/corda amarrada) — não há degraus 'para cima' de 2,4 m sem Atletismo CD 15 (salto parado de 2,4 m = CD 20)."));
    poi(Vec3(71.f, -222.8f, -335.6f), PoiCategory::Treasure, Q("4.4 O Ninho de Cristal · Escama do Dragão Ancestral"),
        Q("Bolsão no teto (72,-220,-335): cristais cor de brasa crescem 'para baixo' (que é para cima, para quem está invertido). No centro, a ESCAMA DO DRAGÃO ANCESTRAL (amuleto: resistência a dano de fogo; morna ao toque; atuna em descanso curto). [E] pega (item na mochila).\n\n"
          "Lore: é uma escama que caiu do dragão quando ele ainda vivia; quem a segura sente, por um instante, o calor lento de uma batida de coração gigantesca — presságio da Camada 5. Pode ressoar com o Coração: a escama brilha quando o grupo chega à câmara (a critério do Mestre)."));
    poi(Vec3(30, -253.4f, -329), PoiCategory::Trap, Q("4.5 O Campo de Gêiseres"),
        Q("9 respiros de vapor superaquecido numa grade 3×3 (colunas x = 15, 30, 45; linhas z = -322, -329, -336). Disparam em sequência, cada um por 2 s, em um ciclo de 12 s. ORDEM (padrão em 'S'): linha z -322 (x 15 → 30 → 45), linha z -329 (x 45 → 30 → 15), linha z -336 (x 15 → 30 → 45).\n\n"
          "Há um aviso: ~0,8 s antes de disparar, o respiro solta um sopro e brilha. Quem observa 1 ciclo deduz a ordem (Inteligência/Investigação CD 13 para ler o padrão). ATINGIDO: Constituição CD 14 ou 4d6 de dano de fogo (metade se passar). No programa a zona de perigo só existe enquanto o gêiser está ativo, e o jogador volta ao último chão seguro."));
    poi(Vec3(-25, -253.6f, -400), PoiCategory::Trap, Q("4.6 A Maré de Magma"),
        Q("Rio de magma ao longo de z ≈ -406,5, de x -66 (tubo de lava) a x +3 (sumidouro). O nível sobe e desce como MARÉ entre -256,0 e -254,5 com período de 40 s. As PEDRAS DE TRAVESSIA (x -25, z -403,4 / -406 / -408,6; topo em -254,9) ficam expostas na maré baixa e SUBMERSAS na alta (~1/3 do tempo): a zona de perigo de magma sobe junto.\n\n"
          "Cair: 10d10 de fogo (no programa: flash e retorno ao último chão seguro). Dica: Percepção CD 10 mostra que as pedras emergem em ~40 s; Atletismo CD 12 para saltar a lacuna de 0,7 m entre pedras. Solução alternativa (a critério do Mestre): redirecionar o fluxo quebrando/movendo cristais até o sumidouro."));
    poi(Vec3(-28, -253.6f, -420), PoiCategory::Trap, Q("4.7 A Câmara do Ar Rarefeito"),
        Q("Túnel de 17 m ao norte do rio e câmara de raio ~9,5 m em (-30,-253,-435). O ar é tão rarefeito que a CHAMA ENCOLHE (a tocha fica fraca) e qualquer MAGIA DE FOGO explode com o DOBRO DA ÁREA, causando desmoronamentos (rochas caídas já cobrem o chão).\n\nDica: o grupo percebe a falta de ar no túnel (Sobrevivência CD 10); descanso aqui é possível, mas inadvisível."));
    poi(Vec3(-30, -253.3f, -435), PoiCategory::Treasure, Q("4.7 Pedra de Fogo Elemental"),
        Q("Granada de rubi turvo num pedestal: arremessável como Bola de Fogo de 3º nível (8d6, raio 6 m). Aqui, na câmara, a área é DOBRADA (raio 12 m) e provoca desmoronamento (Destreza CD 15 ou 4d6 contundente) — ótima armadilha para um jogador ganancioso. [E] a pega."));
    poi(Vec3(96.5f, -252.f, -406.5f), PoiCategory::Puzzle, Q("4.8 A Porta de Adamante e os 3 Selos de Exaustão"),
        Q("Porta de metal verde-negro (adamante, construção do Alhoon) de 8,6 m de largura e 10 m de altura, em (98,-255,-406,5). Três ENGRENAGENS ARCANAS FRIAS (cobertas de geada) cravadas na face oeste. O mecanismo não exige força, mas CALOR VITAL: para girar cada uma, um personagem transfere voluntariamente calor — gasta 1 DADO DE VIDA ou aceita 1 NÍVEL DE EXAUSTÃO.\n\n"
          "No programa: [E] em cada engrenagem cobra o custo anunciado no prompt (alternando Dado de Vida / Exaustão; o Mestre pode trocar na mesa). A engrenagem avermelha e gira. Com as 3, a porta AFUNDA NO CHÃO em 6 s (colisor 'abrível'). Atalho do Mestre: [X].\n"
          "TEMA: introduz mecanicamente a lógica utilitarista do Alhoon — o calor de poucos paga a passagem de todos. Jogadores atentos percebem que o sacrifício é pequeno agora, e crescerá."));
    poi(Vec3(96.5f, -251.5f, -409.3f), PoiCategory::Trap, Q("4.8 Engrenagens de Exaustão"),
        Q("Três engrenagens de ferro (1,15 m) com geada espessa em z -409,3 / -406,5 / -403,7. Arcana CD 14 reconhece necromancia de transferência térmica. Quem insistir em quebrar a geada à força: 2d6 de dano de frio e o mecanismo trava por 1 h. Cada seleção custa 1 Dado de Vida OU 1 nível de Exaustão (a escolha é do jogador)."));

    // ======================================================== PONTOS DE VISTA
    w.addViewpoint(Q("4.1 O Balcão do Geodo (primeira vista)"), Vec3(-19.5f, -212.35f, -383.5f), -90.f, -13.f, false, kGrp);
    w.addViewpoint(Q("4.1b Panorama do balcão"), Vec3(-22, -212.35f, -388.f), -35.f, -12.f, false, kGrp);
    w.addViewpoint(Q("4.2 Escadaria de Cristal (topo de P1)"), P1 + Vec3(0, 1.65f, 0), yawTo(P1, P2), -14.f, false, kGrp);
    w.addViewpoint(Q("4.2b Escadaria vista do chão"), Vec3(12, -253.35f, -345), yawTo(Vec3(12, 0, -345), Vec3(-6, 0, -380)), 24.f, false, kGrp);
    w.addViewpoint(Q("4.3 O Selo dos Prismas (início)"), Vec3(14, -253.35f, -352), yawTo(Vec3(14, 0, -352), Vec3(5, 0, -340)), -2.f, false, kGrp);
    w.addViewpoint(Q("4.3b Fenda da Chama e feixe"), Vec3(-8, -253.35f, -347), yawTo(Vec3(-8, 0, -347), Vec3(-20, 0, -340)), -2.f, false, kGrp);
    w.addViewpoint(Q("4.3c Prismas P3-P4 e iscas"), Vec3(26, -253.35f, -382), yawTo(Vec3(26, 0, -382), Vec3(35, 0, -366)), 0.f, false, kGrp);
    w.addViewpoint(Q("4.3d Muralha de Gelo Místico"), Vec3(60, -253.35f, -403), yawTo(Vec3(60, 0, -403), Vec3(80, 0, -406.5f)), 0.f, false, kGrp);
    w.addViewpoint(Q("4.4 Zona de Gravidade (planalto)"), Vec3(56, -245.35f, -340), yawTo(Vec3(56, 0, -340), Vec3(66, 0, -338)), 40.f, false, kGrp);
    w.addViewpoint(Q("4.4b Ninho de Cristal"), Vec3(69.5f, -222.9f, -336.f), yawTo(Vec3(69.5f, 0, -336), Vec3(73, 0, -335)), 0.f, false, kGrp);
    w.addViewpoint(Q("4.5 Campo de Gêiseres"), Vec3(30, -253.35f, -345), 180.f, 6.f, false, kGrp);
    w.addViewpoint(Q("4.6 A Maré de Magma"), Vec3(-25, -253.35f, -396), 0.f, -4.f, false, kGrp);
    w.addViewpoint(Q("4.7 Câmara do Ar Rarefeito"), Vec3(-28, -253.35f, -424), 0.f, 0.f, false, kGrp);
    w.addViewpoint(Q("4.8 A Porta de Adamante"), Vec3(88, -253.35f, -406.5f), -90.f, 8.f, false, kGrp);
    w.addViewpoint(Q("4.9 O Geodo inteiro (voo, sul)"), Vec3(40, -236, -322), 22.f, -6.f, true, kGrp);
    w.addViewpoint(Q("4.9b O Geodo inteiro (voo, balcão)"), Vec3(-10, -213.f, -380), -75.f, -14.f, true, kGrp);

    // ======================================================== ROTA
    w.addRoutePoint(Q("Balcão do Geodo"), Vec3(-36, -213.4f, -383));
    w.addRoutePoint(Q("Escadaria · P1"), P1 + Vec3(0, 0.6f, 0));
    w.addRoutePoint(Q("Escadaria · P2"), P2 + Vec3(0, 0.6f, 0));
    w.addRoutePoint(Q("Escadaria · P3"), P3 + Vec3(0, 0.6f, 0));
    w.addRoutePoint(Q("Chão do Geodo"), E3 + Vec3(-1.5f, 0.6f, 0.f));
    w.addRoutePoint(Q("Selo dos Prismas · P2"), Vec3(2.5f, -254.4f, -368));
    w.addRoutePoint(Q("Corredor dos prismas"), Vec3(33, -254.4f, -395));
    w.addRoutePoint(Q("Antes da Muralha de Gelo"), Vec3(62, -254.4f, -403));
    w.addRoutePoint(Q("Passagem leste"), Vec3(78, -254.4f, -406));
    w.addRoutePoint(Q("Porta de Adamante"), Vec3(96, -254.4f, -406.5f));

    // ======================================================== AUTOTESTE DO SELO DOS PRISMAS
    {
        w.refreshColliders();
        Trace s = traceBeam(w, kSolution);
        Trace i = traceBeam(w, kInitial);
        bool okSol = s.hitIce, okIni = !i.hitIce;
        int falsePos = 0;
        for (int j = 0; j < 4; ++j)
            for (int d = 0; d < 8; ++d) {
                if (d == kSolution[j]) continue;
                int dirs[4] = {kSolution[0], kSolution[1], kSolution[2], kSolution[3]};
                dirs[j] = d;
                if (traceBeam(w, dirs).hitIce) { ++falsePos; outLine(QStringLiteral("[Selo dos Prismas]   solução alternativa: prisma %1 -> %2").arg(j + 1).arg(d)); }
            }
        outLine(QStringLiteral("[Selo dos Prismas] autoteste: solução %1 (%2 segmentos) · inicial não resolvido: %3 · configurações com 1 prisma errado que resolvem: %4 → %5")
                    .arg(okSol ? "OK" : "FALHOU").arg((int)s.segs.size()).arg(okIni ? "OK" : "FALHOU").arg(falsePos).arg(okSol && okIni && falsePos == 0 ? "OK" : "ATENÇÃO"));
    }
}
