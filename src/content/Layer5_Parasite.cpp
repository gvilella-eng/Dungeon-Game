// CAMADA 5 — O Parasita de Ferro (covil do Alhoon) (Sessao 6, Nivel 9)
// A caverna E o corpo do dragao: costelas fosseis, cranio, o Coracao pulsante sobre um pedestal, fosso de magma.
// O parasita: andaimes de ferro enferrujado, 36 capsulas de vidro com conduitos humanos, sanguessugas mecanicas,
// o Dissipador do Vazio, baterias arcanas, o Altar do Alhoon, o Arquivo de Calibracao e o evento Colapso.
#include "Dungeon.h"
#include "world/World.h"
#include "Prefabs.h"
#include <memory>

#define Q(s) QStringLiteral(s)

namespace {

const QString kGrp = QStringLiteral("Camada 5 · O Parasita de Ferro");
const QString kSess = QStringLiteral("Sessão 6 · Nível 9");
const QString kArea = QStringLiteral("Camada 5 · O Parasita de Ferro");

const Vec3 HC(100, -285, -505);       // centro do elipsoide (ajustado: ver relatorio)
const Vec3 HR(48, 38, 45);
const float FL = -300.f;              // chao
const Vec3 HEART(100, -287, -505);    // centro do Coracao
const float kPi_ = 3.14159265f;

const float kRingR = 16.f;
const float kRingY[3] = {-300.f, -292.f, -284.f};

Vec3 pol(float r, float deg, float y) { float a = deg * kPi_ / 180.f; return Vec3(HC.x() + r * std::cos(a), y, HC.z() + r * std::sin(a)); }

float wallR(float deg, float y)
{
    float t = 1.f - std::pow((y - HC.y()) / HR.y(), 2.f);
    if (t <= 0.f) return 0.f;
    float a = deg * kPi_ / 180.f, c = std::cos(a) / HR.x(), s = std::sin(a) / HR.z();
    return std::sqrt(t) / std::sqrt(c * c + s * s);
}
float ceilY(float x, float z)
{
    float a = (x - HC.x()) / HR.x(), c = (z - HC.z()) / HR.z();
    float t = 1.f - a * a - c * c;
    return t > 0.f ? HC.y() + HR.y() * std::sqrt(t) : HC.y();
}

float yawTo(const Vec3& from, const Vec3& to) { return std::atan2(-(to.x() - from.x()), -(to.z() - from.z())) * 57.2957795f; }

QQuaternion alongZ(const Vec3& dir)
{
    Vec3 z = dir.normalized(), up(0, 1, 0);
    Vec3 x = Vec3::crossProduct(up, z);
    if (x.lengthSquared() < 1e-6f) x = Vec3(1, 0, 0);
    x.normalize();
    Vec3 y = Vec3::crossProduct(z, x);
    return QQuaternion::fromAxes(x, y, z);
}

// colisor de laje com o TOPO passando por a-b (a, b no topo)
void slabCol(World& w, const QString& name, const Vec3& a, const Vec3& b, float width, float thick)
{
    Vec3 d = b - a; float len = d.length();
    QQuaternion q = alongZ(d);
    Vec3 yAxis = q.rotatedVector(Vec3(0, 1, 0));
    w.addBoxCollider(name, (a + b) * 0.5f - yAxis * (thick * 0.5f), Vec3(width * 0.5f, thick * 0.5f, len * 0.5f), q, false);
}

// curva de Bezier quadratica sobre 3 pontos
std::vector<Vec3> bezier(const Vec3& a, const Vec3& c, const Vec3& b, int n = 10)
{
    std::vector<Vec3> p;
    for (int i = 0; i <= n; ++i) { float t = (float)i / n; p.push_back(a * ((1 - t) * (1 - t)) + c * (2 * (1 - t) * t) + b * (t * t)); }
    return p;
}

void tun(World& w, std::vector<Vec3> floorPts, float r, int mat, float noise = 0.3f)
{
    for (auto& p : floorPts) p.setY(p.y() + r * 0.6f);
    TunnelOpts o; o.floorFrac = 0.6f; o.material = mat; o.noise = noise; o.k = 1.6f;
    w.carveTunnel(floorPts, r, o);
}

struct Rng {
    uint32_t s;
    explicit Rng(uint32_t seed) : s(seed * 2654435761u + 777u) {}
    float f() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (s & 0xFFFFFF) / 16777216.f; }
    float r(float a, float b) { return a + (b - a) * f(); }
};

float heartBeat(float t, float period)
{
    float p = std::fmod(t, period) / period;
    auto g = [](float x, float c, float w) { float d = (x - c) / w; return std::exp(-d * d); };
    return std::min(1.f, g(p, 0.06f, 0.05f) + 0.65f * g(p, 0.30f, 0.06f));
}

} // namespace

void buildLayer5Parasite(World& w)
{
    // ======================================================== GEOMETRIA
    w.carveChamber(HC, HR, FL, MAT_HOT_BASALT, 0.7f, 2.5f, Q("camara_coracao"));
    // fosso de magma (r 22..30)
    w.carve(Prim::annulus(Vec3(HC.x(), -303.5f, HC.z()), 22.f, 30.f, -307.f, -300.f).noise(0.25f).smooth(0.6f).mat(MAT_HOT_BASALT).named(Q("fosso")));
    // pedestal de rocha do Coracao
    w.fill(Prim::cylinder(Vec3(HC.x(), 0, HC.z()), 7.4f, -302.f, -294.6f).noise(0.35f).smooth(1.2f).mat(MAT_HOT_BASALT).named(Q("pedestal")));
    // plataforma do Altar (norte, 4 m acima do chao)
    w.fill(Prim::box(Vec3(100, -298, -541.5f), Vec3(13, 2, 12), QQuaternion(), 0.4f).noise(0.12f).smooth(0.5f).mat(MAT_OBSIDIAN).named(Q("plataforma_altar")));
    // pilares das baterias no fosso (diagonais, r 26)
    for (int k = 0; k < 4; ++k) {
        Vec3 p = pol(26.f, 45.f + 90.f * k, 0);
        w.fill(Prim::cylinder(Vec3(p.x(), 0, p.z()), 2.6f, -307.f, -299.6f).noise(0.2f).smooth(0.5f).mat(MAT_HOT_BASALT).named(Q("pilar_bateria")));
    }
    // a Descida de Ferro: tunel com escadas de grade, do atras da Porta ate a boca da camara
    const std::vector<Vec3> T = {Vec3(100, -255, -406.5f), Vec3(104, -255, -408), Vec3(115, -265, -430), Vec3(118, -278, -455), Vec3(121, -284, -470)};
    tun(w, {T[0], T[1], T[2]}, 2.8f, MAT_OBSIDIAN, 0.3f);
    tun(w, {T[2], T[3], T[4]}, 2.8f, MAT_HOT_BASALT, 0.35f);
    // Arquivo de Calibracao (sala lateral a oeste)
    w.carveChamber(Vec3(46, -296, -500), Vec3(9, 5.2f, 7.6f), FL, MAT_OBSIDIAN, 0.2f, 2.f, Q("arquivo"));
    tun(w, {Vec3(61, FL, -500), Vec3(48, FL, -500)}, 2.5f, MAT_OBSIDIAN, 0.2f);

    // ======================================================== ZONAS
    {
        ZoneEffects fx;
        fx.areaName = kArea; fx.subtitle = kSess + Q(" · 45 °C · o Coração bate");
        fx.temperature = 45.f; fx.fogColor = Vec3(0.085f, 0.022f, 0.012f); fx.fogDensity = 0.0085f;
        fx.ambientColor = Vec3(0.24f, 0.075f, 0.045f); fx.ambientGround = Vec3(0.12f, 0.035f, 0.02f); fx.ambientStrength = 1.f;
        w.addZone(Q("parasita"), ZoneShape::makeBox(Vec3(52, -306, -552), Vec3(150, -246, -458)), fx, 2);

        ZoneEffects tz; tz.areaName = Q("5.1 A Descida de Ferro"); tz.subtitle = kSess + Q(" · 30 °C"); tz.temperature = 30.f;
        tz.fogColor = Vec3(0.05f, 0.025f, 0.02f); tz.fogDensity = 0.018f; tz.ambientColor = Vec3(0.10f, 0.045f, 0.03f); tz.ambientGround = Vec3(0.05f, 0.02f, 0.015f);
        w.addZone(Q("descida"), ZoneShape::makeBox(Vec3(95, -290, -475), Vec3(130, -250, -404)), tz, 3);

        ZoneEffects cz; cz.areaName = Q("5.2 A Câmara do Coração"); cz.subtitle = kSess + Q(" · a caverna é o corpo do dragão");
        w.addZone(Q("camara_coracao"), ZoneShape::makeBox(Vec3(54, -304, -551), Vec3(146, -247, -460)), cz, 3);

        ZoneEffects rz; rz.areaName = Q("5.3 Os Andaimes e as Cápsulas"); rz.subtitle = kSess + Q(" · a engenharia do sofrimento");
        rz.temperature = 48.f;
        w.addZone(Q("andaimes"), ZoneShape::makeCylinder(Vec3(HC.x(), 0, HC.z()), 21.f, -302.f, -276.f), rz, 4);

        ZoneEffects az; az.areaName = Q("5.6 O Altar do Alhoon"); az.subtitle = kSess + Q(" · utilitarismo");
        w.addZone(Q("altar"), ZoneShape::makeBox(Vec3(86, -300, -554), Vec3(114, -280, -529.5f)), az, 4);

        ZoneEffects ar; ar.areaName = Q("5.7 O Arquivo de Calibração"); ar.subtitle = kSess; ar.temperature = 38.f;
        ar.fogColor = Vec3(0.05f, 0.03f, 0.04f); ar.fogDensity = 0.014f; ar.ambientColor = Vec3(0.10f, 0.07f, 0.09f); ar.ambientGround = Vec3(0.05f, 0.035f, 0.045f);
        w.addZone(Q("arquivo"), ZoneShape::makeBox(Vec3(35, -302, -510), Vec3(58, -288, -490)), ar, 4);

        // fosso de magma: perigo (28 cilindros em anel) — o centro do corpo so entra se cair no fosso
        ZoneEffects hz; hz.hazard = true; hz.hazardText = Q("Magma do fosso: 10d10 de dano de fogo!"); hz.temperature = 80.f;
        for (int k = 0; k < 28; ++k) {
            Vec3 p = pol(26.f, k * 360.f / 28.f, 0);
            w.addZone(Q("hazard_fosso_%1").arg(k), ZoneShape::makeCylinder(Vec3(p.x(), 0, p.z()), 4.6f, -308.f, -299.7f), hz, 9);
        }
    }

    // ======================================================== LAVA DO FOSSO
    {
        MeshBuilder lm; lm.setFlow(0.4f, 0.9f);
        lm.cylinder(Vec3(HC.x(), -301.15f, HC.z()), Vec3(HC.x(), -301.f, HC.z()), 29.7f, 29.7f, 56, QColor(255, 105, 20), MAT_LAVA, true, false);
        w.addStatic(std::move(lm));
        for (int k = 0; k < 4; ++k) {
            Vec3 p = pol(26.f, 20.f + 90.f * k, -299.5f);
            Light* l = w.addLight(p, Vec3(1.f, 0.38f, 0.1f), 7.f, 26.f, LightAnim::Pulse, Q("fosso"));
            l->animFreq = 0.55f; l->animDepth = 0.25f;
        }
        EmitterDesc e; e.preset = ParticlePreset::Embers; ParticleEmitter::applyPreset(e);
        e.pos = Vec3(HC.x(), -300.f, HC.z()); e.halfExtent = Vec3(30.f, 0.2f, 30.f); e.rate = 120.f; e.lifeMin = 3.f; e.lifeMax = 7.f; e.velocity = Vec3(0, 1.8f, 0); e.activeRange = 160.f;
        w.addEmitter(e);
        EmitterDesc s; s.preset = ParticlePreset::Steam; ParticleEmitter::applyPreset(s);
        s.pos = Vec3(HC.x(), -300.f, HC.z()); s.halfExtent = Vec3(28.f, 0.1f, 28.f); s.rate = 22.f; s.activeRange = 160.f;
        s.colorStart = QColor(255, 150, 100, 55); s.colorEnd = QColor(255, 80, 50, 0);
        w.addEmitter(s);
        EmitterDesc d; d.preset = ParticlePreset::Dust; ParticleEmitter::applyPreset(d);
        d.pos = Vec3(HC.x(), -275.f, HC.z()); d.halfExtent = Vec3(40.f, 22.f, 38.f); d.rate = 50.f; d.activeRange = 160.f; d.colorStart = QColor(255, 170, 120, 70);
        w.addEmitter(d);
    }

    // ======================================================== COSTELAS FOSSEIS, ESPINHA E CRANIO
    {
        MeshBuilder bm;
        QColor bone(196, 178, 150);
        Rng rg(31);
        for (int k = 0; k < 8; ++k) {
            float dz = -31.5f + 9.f * k;
            float z = HC.z() + dz, f = std::sqrt(1.f - (dz / HR.z()) * (dz / HR.z()));
            float phi0 = std::asin(std::max(-0.97f, -15.f / (HR.y() * f)));
            for (int side = -1; side <= 1; side += 2) {
                std::vector<Vec3> pts; std::vector<float> rad;
                const int n = 16;
                for (int i = 0; i <= n; ++i) {
                    float t = (float)i / n;
                    float phi = phi0 + (kPi_ * 0.5f - phi0) * t;
                    float x = HC.x() + side * HR.x() * f * std::cos(phi) * 0.972f;
                    float y = HC.y() + HR.y() * f * std::sin(phi) * 0.972f;
                    pts.push_back(Vec3(x, y, z + std::sin(t * 6.f + k) * 0.4f));
                    rad.push_back(1.15f - 0.7f * t);
                }
                bm.tube(pts, rad, 8, QColor(bone.red() + (int)rg.r(-12, 12), bone.green() + (int)rg.r(-12, 12), bone.blue()), MAT_BONE);
                bm.sphere(pts.front() - Vec3(0, 0.3f, 0), 1.5f, bone, MAT_BONE, 8);
            }
        }
        // espinha dorsal ao longo do teto (eixo norte-sul)
        std::vector<Vec3> sp; std::vector<float> sr;
        for (float z = -546.f; z <= -464.f; z += 4.f) { sp.push_back(Vec3(HC.x(), ceilY(HC.x(), z) - 1.2f, z)); sr.push_back(1.0f); }
        bm.tube(sp, sr, 8, QColor(176, 158, 132), MAT_BONE);
        for (size_t i = 0; i < sp.size(); ++i) {
            float vz = 2.8f - std::fabs((float)i - 10.f) * 0.04f;
            bm.ellipsoid(sp[i], Vec3(1.9f, 1.15f, vz * 0.55f), bone, MAT_BONE, 8);
            bm.cone(sp[i] + Vec3(0, 0.6f, 0), sp[i] + Vec3(0, 2.4f, 0), 0.5f, 5, QColor(170, 152, 126), MAT_BONE);
        }
        w.addStatic(std::move(bm));
    }
    {   // crânio do dragao emergindo da parede norte (boca aberta, para o sul)
        MeshBuilder sk;
        const Vec3 o(100, -281, -547.5f);
        sk.push(); sk.translate(o);
        QColor bone(190, 172, 144), dark(34, 24, 22);
        sk.ellipsoid(Vec3(0, 1.0f, -2.0f), Vec3(4.0f, 3.2f, 5.0f), bone, MAT_BONE, 14);                   // cranio
        sk.ellipsoid(Vec3(0, 0.2f, 3.2f), Vec3(2.4f, 1.7f, 5.2f), bone, MAT_BONE, 12);                    // focinho
        sk.ellipsoid(Vec3(0, 1.6f, 1.2f), Vec3(2.6f, 1.2f, 2.8f), QColor(176, 158, 130), MAT_BONE, 10);   // crista
        sk.push(); sk.translate(0, -2.6f, 1.2f); sk.rotate(-14.f, Vec3(1, 0, 0));                         // mandibula inferior, aberta
        sk.ellipsoid(Vec3(0, 0, 3.0f), Vec3(1.9f, 0.8f, 4.8f), QColor(180, 162, 136), MAT_BONE, 10);
        for (int i = 0; i < 9; ++i) { float zz = 0.4f + i * 0.9f; for (int s = -1; s <= 1; s += 2) sk.cone(Vec3(s * 1.3f, 0.5f, zz), Vec3(s * 1.25f, 1.9f, zz), 0.16f, 5, QColor(232, 224, 200), MAT_BONE); }
        sk.pop();
        for (int i = 0; i < 9; ++i) { float zz = 0.8f + i * 0.9f; for (int s = -1; s <= 1; s += 2) sk.cone(Vec3(s * 1.55f, -1.2f, zz), Vec3(s * 1.5f, -2.9f, zz), 0.2f, 5, QColor(232, 224, 200), MAT_BONE); }
        for (int s = -1; s <= 1; s += 2) {
            sk.ellipsoid(Vec3(s * 2.45f, 1.5f, 0.7f), Vec3(1.1f, 1.0f, 1.0f), dark, MAT_COAL, 9);                         // orbita
            sk.sphere(Vec3(s * 2.55f, 1.4f, 1.0f), 0.38f, QColor(255, 80, 30), MAT_EMISSIVE, 8);                           // brilho fraco
            sk.ellipsoid(Vec3(s * 0.9f, 0.9f, 7.9f), Vec3(0.3f, 0.35f, 0.3f), dark, MAT_COAL, 6);                          // narinas
            sk.cone(Vec3(s * 2.9f, 3.0f, -1.5f), Vec3(s * 5.4f, 6.8f, -5.5f), 0.85f, 7, QColor(176, 158, 130), MAT_BONE);   // chifres
        }
        sk.pop();
        w.addStatic(std::move(sk));
        for (int s = -1; s <= 1; s += 2)
            w.addLight(o + Vec3(s * 2.4f, 1.4f, 1.3f), Vec3(1.f, 0.3f, 0.1f), 2.5f, 12.f, LightAnim::Pulse, Q("orbita"))->animFreq = 0.55f;
        w.addLight(Vec3(100, -279.f, -538.f), Vec3(1.f, 0.55f, 0.3f), 9.f, 34.f, LightAnim::Pulse, Q("luz do cranio"))->animFreq = 0.55f;
    }

    // ======================================================== O CORACAO DO DRAGAO (dinamico, pulsa ~1,8 s)
    DynHandle heart = nullptr;
    {
        MeshBuilder hm; Rng rg(5);
        hm.sphere(Vec3(0, 0, 0), 7.3f, QColor(255, 96, 24), MAT_LAVA, 22);
        hm.ellipsoid(Vec3(2.8f, 1.5f, 0.4f), Vec3(3.4f, 3.8f, 3.6f), QColor(190, 40, 24), MAT_FLESH, 16);
        hm.ellipsoid(Vec3(-3.0f, 0.4f, -1.2f), Vec3(3.2f, 3.6f, 3.2f), QColor(180, 36, 22), MAT_FLESH, 16);
        hm.ellipsoid(Vec3(0.5f, -3.0f, 3.2f), Vec3(3.4f, 2.4f, 3.0f), QColor(185, 38, 22), MAT_FLESH, 16);
        hm.ellipsoid(Vec3(-0.5f, 3.4f, -3.0f), Vec3(3.0f, 2.6f, 2.6f), QColor(180, 36, 22), MAT_FLESH, 14);
        // veias de fogo: caminhadas sobre a superficie
        for (int i = 0; i < 46; ++i) {
            float u = rg.r(0, 6.283f), v = rg.r(-1.f, 1.f), cv = std::sqrt(1.f - v * v);
            Vec3 p = Vec3(std::cos(u) * cv, v, std::sin(u) * cv) * 7.55f;
            Vec3 dir = Vec3::crossProduct(p, Vec3(rg.r(-1, 1), rg.r(-1, 1), rg.r(-1, 1))).normalized();
            std::vector<Vec3> pts; std::vector<float> rad;
            for (int k = 0; k < 8; ++k) {
                pts.push_back(p);
                rad.push_back(0.16f - 0.01f * k);
                p = (p + dir * 1.1f).normalized() * (7.55f + 0.05f * std::sin(k));
                dir = (dir + Vec3(rg.r(-0.35f, 0.35f), rg.r(-0.35f, 0.35f), rg.r(-0.35f, 0.35f))).normalized();
                dir = (dir - p.normalized() * Vec3::dotProduct(dir, p.normalized())).normalized();
            }
            hm.tube(pts, rad, 5, QColor(255, 200 + (int)rg.r(-30, 30), 70), MAT_EMISSIVE);
        }
        // vasos: aorta rumo ao Vazio e tubos laterais
        hm.cylinder(Vec3(0, 6.5f, 0), Vec3(0, 11.5f, 0), 1.7f, 1.2f, 10, QColor(130, 20, 16), MAT_FLESH);
        hm.torus(Vec3(0, 10.5f, 0), Vec3(0, 1, 0), 1.4f, 0.28f, QColor(255, 170, 60), MAT_EMISSIVE, 14, 5);
        for (int k = 0; k < 6; ++k) {
            float a = k * 1.047f + 0.3f;
            hm.cylinder(Vec3(std::cos(a) * 6.5f, 1.0f - (k % 2) * 3.f, std::sin(a) * 6.5f), Vec3(std::cos(a) * 9.f, 1.5f - (k % 2) * 3.5f, std::sin(a) * 9.f), 1.0f, 0.75f, 8, QColor(120, 18, 14), MAT_FLESH);
        }
        heart = w.addDynamic(Q("coracao"), std::move(hm), HEART);
        w.addSphereCollider(Q("coracao"), HEART, 7.4f);
        Light* l = w.addLight(HEART + Vec3(0, 1, 0), Vec3(1.f, 0.30f, 0.09f), 22.f, 62.f, LightAnim::Custom, Q("coracao"));
        l->custom = [](float t) { return 0.45f + 0.75f * heartBeat(t, 1.8f); };
        Light* l2 = w.addLight(HEART + Vec3(0, 6, 0), Vec3(1.f, 0.5f, 0.2f), 9.f, 95.f, LightAnim::Custom, Q("coracao amplo"));
        l2->custom = [](float t) { return 0.5f + 0.6f * heartBeat(t, 1.8f); };
        w.addUpdater([heart](float, float t) {
            float b = heartBeat(t, 1.8f);
            float s = 1.f + 0.055f * b;
            heart->scl = Vec3(s, s * (1.f + 0.02f * b), s);
            heart->setYaw(std::sin(t * 0.1f) * 8.f);
        });
    }

    // ======================================================== ANDAIMES (3 aneis), CAPSULAS (36), VAGA, CANOS
    {
        MeshBuilder sc, cap;
        const QColor iron(84, 64, 52);
        // aneis de grade: 12 trechos por anel
        for (int ring = 0; ring < 3; ++ring) {
            float y = kRingY[ring];
            for (int k = 0; k < 12; ++k) {
                Vec3 a = pol(kRingR, k * 30.f, y + 0.0f), b = pol(kRingR, (k + 1) * 30.f, y + 0.0f);
                Prefabs::gratedWalkway(sc, a, b, 3.0f, true);
                Vec3 mid = (a + b) * 0.5f, d = b - a;
                w.addBoxCollider(Q("anel%1_%2").arg(ring).arg(k), mid + Vec3(0, -0.1f, 0), Vec3(1.55f, 0.1f, d.length() * 0.5f + 0.15f), alongZ(d), false);
            }
            // vigas radiais e travessas
            for (int k = 0; k < 12; ++k) {
                Vec3 p = pol(kRingR + 1.5f, k * 30.f, y - 0.25f);
                sc.cylinder(pol(kRingR - 1.5f, k * 30.f, y - 0.25f), p, 0.09f, 6, iron, MAT_RUSTY_IRON);
            }
            // pilares ate o chao (so em aneis elevados) + vigas diagonais cravadas nas paredes
            for (int k = 0; k < 8; ++k) {
                float ang = 22.5f + k * 45.f;
                Vec3 base = pol(kRingR + 1.5f, ang, y - 0.3f);
                if (ring > 0) sc.cylinder(pol(kRingR + 1.5f, ang, FL), base, 0.16f, 6, QColor(70, 54, 46), MAT_RUSTY_IRON);
                Vec3 from = pol(kRingR + 1.5f, ang, y + 3.2f);
                float wr = wallR(ang, y + 14.f) - 0.5f;
                Vec3 to = pol(wr, ang, y + 14.f);
                sc.cylinder(from, to, 0.28f, 0.22f, 6, QColor(90, 66, 50), MAT_RUSTY_IRON);
                sc.cylinder(base, from, 0.12f, 6, iron, MAT_RUSTY_IRON);
                // viga de cunha cravada
                for (int c = 1; c < 4; ++c) { Vec3 q = from + (to - from) * (c / 4.f); sc.box(q, Vec3(0.25f, 0.05f, 0.25f), QColor(70, 52, 44), MAT_RUSTY_IRON); }
                sc.sphere(to, 0.45f, QColor(60, 46, 40), MAT_RUSTY_IRON, 6);
            }
            // anel de ferro (guarda-corpo exterior contínuo)
            std::vector<Vec3> rp;
            for (int k = 0; k <= 48; ++k) rp.push_back(pol(kRingR + 1.5f, k * 7.5f, y + 1.0f));
            sc.tube(rp, 0.05f, 5, QColor(100, 74, 56), MAT_RUSTY_IRON);
        }
        // escadas entre aneis (chords de 60 graus): 0->1 em 0..60 e 180..240; 1->2 em 120..180 e 300..360
        struct Fl { int ring; float a0, a1; };
        const Fl flights[] = {{0, 0.f, 60.f}, {0, 180.f, 240.f}, {1, 120.f, 180.f}, {1, 300.f, 360.f}};
        int fi = 0;
        for (const Fl& f : flights) {
            Vec3 b = pol(kRingR, f.a0, kRingY[f.ring]), t = pol(kRingR, f.a1, kRingY[f.ring + 1]);
            Prefabs::ironStairs(sc, b, t, 1.6f);
            slabCol(w, Q("escada_anel_%1").arg(fi), b + Vec3(0, 0, 0), t, 1.8f, 0.3f);
            // suporte
            Vec3 m = (b + t) * 0.5f;
            sc.cylinder(Vec3(m.x(), kRingY[f.ring] - 0.3f, m.z()), m - Vec3(0, 0.3f, 0), 0.1f, 6, iron, MAT_RUSTY_IRON);
            ++fi;
        }
        // 36 capsulas
        std::vector<Vec3> capTop;
        const std::vector<std::pair<int, int>> depleted = {{0, 2}, {0, 5}, {0, 9}, {1, 0}, {1, 4}, {1, 7}, {2, 1}, {2, 4}, {2, 6}, {2, 10}, {0, 11}};
        int vagaRing = 1, vagaSlot = 9;
        for (int ring = 0; ring < 3; ++ring)
            for (int k = 0; k < 12; ++k) {
                float ang = 15.f + 30.f * k;
                Vec3 base = pol(16.6f, ang, kRingY[ring] + 0.12f);
                bool dep = std::find(depleted.begin(), depleted.end(), std::make_pair(ring, k)) != depleted.end();
                bool vaga = (ring == vagaRing && k == vagaSlot);
                cap.push(); cap.translate(base);
                cap.rotateY(vaga ? 0.f : 90.f - ang);   // frente para o centro (exceto a Vaga, que encara o altar: norte)
                Prefabs::glassCapsule(cap, Vec3(0.0003f * (ring * 12 + k), 0, 0), 2.4f, 0.7f, !vaga, dep && !vaga, vaga);
                cap.pop();
                w.addCylinderCollider(Q("capsula_%1_%2").arg(ring).arg(k), base + Vec3(0, 1.2f, 0), 0.85f, 1.3f);
                // cano de bronze ate o Coracao
                Vec3 top = base + Vec3(0, 2.55f, 0);
                Vec3 dirIn = (HEART - Vec3(base.x(), HEART.y(), base.z())); dirIn.setY(0); dirIn.normalize();
                float hy = HEART.y() + (ring - 1) * 3.f;
                Vec3 end = Vec3(HEART.x(), hy, HEART.z()) - dirIn * 7.0f;
                Vec3 ctrl = top + dirIn * 3.2f + Vec3(0, 3.4f + 0.8f * (k % 3), 0);
                Prefabs::pipe(cap, bezier(top, ctrl, end, 9), 0.11f, dep ? MAT_IRON : MAT_BRONZE, true);
                capTop.push_back(top);
            }
        w.addStatic(std::move(sc));
        w.addStatic(std::move(cap));
        // brilho doentio verde das capsulas
        for (int ring = 0; ring < 3; ++ring)
            for (int k = 0; k < 3; ++k) {
                Vec3 p = pol(15.f, 60.f + k * 120.f, kRingY[ring] + 1.4f);
                Light* l = w.addLight(p, Vec3(0.30f, 1.f, 0.45f), 2.6f, 10.f, LightAnim::Pulse, Q("capsulas"));
                l->animFreq = 0.35f + 0.07f * k; l->animDepth = 0.3f;
            }
    }

    // ======================================================== PONTE DE FERRO (sul) e ESCADARIA DA DESCIDA (gantry espiral)
    {
        MeshBuilder bm;
        const float ba = 100.f;
        Vec3 a = pol(34.f, ba, FL - 0.1f + 0.0f), b = pol(19.f, ba, FL - 0.1f);
        a.setY(-299.85f); b.setY(-299.85f);
        Prefabs::gratedWalkway(bm, a, b, 3.2f, true);
        w.addBoxCollider(Q("ponte_ferro"), (a + b) * 0.5f + Vec3(0, -0.1f, 0), Vec3(1.65f, 0.1f, (a - b).length() * 0.5f), alongZ(b - a), false);
        // arcos de ferro e vigas sob a ponte
        for (int i = 0; i <= 6; ++i) {
            Vec3 p = a + (b - a) * (i / 6.f);
            bm.cylinder(p + Vec3(0, -0.2f, 0), p + Vec3(0, -3.4f, 0), 0.1f, 6, QColor(76, 58, 48), MAT_RUSTY_IRON);
        }
        // gantry espiral na parede SE: r 38,5 de 60 a 108 graus, y -284 -> -300
        const int N = 5;
        const float aA = 60.f, aB = 108.f, rr = 38.5f;
        Vec3 landA(121.6f, -284.f, -468.2f), landB = pol(rr, aA, -284.f);
        Prefabs::gratedWalkway(bm, landA, landB, 3.2f, true);
        w.addBoxCollider(Q("gantry_patamar"), (landA + landB) * 0.5f + Vec3(0, -0.1f, 0), Vec3(1.65f, 0.1f, (landA - landB).length() * 0.5f + 0.2f), alongZ(landB - landA), false);
        for (int i = 0; i < N; ++i) {
            float a0 = aA + (aB - aA) * i / N, a1 = aA + (aB - aA) * (i + 1) / N;
            float y0 = -284.f + (FL + 284.f) * i / N, y1 = -284.f + (FL + 284.f) * (i + 1) / N;
            Vec3 p0 = pol(rr, a0, y0), p1 = pol(rr, a1, y1);
            Prefabs::ironStairs(bm, p0, p1, 2.3f);
            slabCol(w, Q("gantry_%1").arg(i), p0, p1, 2.5f, 0.3f);
            // pilar ao chao + viga na parede
            bm.cylinder(Vec3(p1.x(), FL, p1.z()), p1 - Vec3(0, 0.3f, 0), 0.16f, 6, QColor(70, 54, 46), MAT_RUSTY_IRON);
            Vec3 wall = pol(wallR(a1, y1 + 4.f) - 0.5f, a1, y1 + 4.f);
            bm.cylinder(p1 + Vec3(0, 0.9f, 0), wall, 0.2f, 6, QColor(88, 66, 52), MAT_RUSTY_IRON);
        }
        w.addStatic(std::move(bm));
    }

    // ======================================================== SANGUESSUGAS MECANICAS (10), DISSIPADOR DO VAZIO, BATERIAS
    {
        MeshBuilder lm;
        std::vector<Vec3> batt;
        for (int k = 0; k < 4; ++k) batt.push_back(pol(26.f, 45.f + 90.f * k, -299.6f + 6.8f));
        for (int i = 0; i < 10; ++i) {
            float a = 10.f + i * 36.f, r = (i % 2 == 0) ? 12.f + (i % 4) * 3.f : 26.f + (i % 3) * 3.f;
            Vec3 p = pol(r, a, 0);
            float cy = ceilY(p.x(), p.z()) - 1.4f;
            lm.cylinder(Vec3(p.x(), cy + 2.2f, p.z()), Vec3(p.x(), cy + 0.6f, p.z()), 0.13f, 6, QColor(150, 105, 55), MAT_BRONZE);
            lm.cylinder(Vec3(p.x(), cy + 0.6f, p.z()), Vec3(p.x(), cy + 0.25f, p.z()), 0.62f, 0.5f, 10, QColor(150, 105, 55), MAT_BRONZE);
            lm.cylinder(Vec3(p.x(), cy + 0.25f, p.z()), Vec3(p.x(), cy - 2.5f, p.z()), 0.5f, 0.5f, 12, QColor(120, 230, 150, 90), MAT_GLASS, false);
            lm.cylinder(Vec3(p.x(), cy + 0.15f, p.z()), Vec3(p.x(), cy - 2.3f, p.z()), 0.4f, 0.4f, 10, QColor(80, 255, 120, 130), MAT_NECRO_LIQUID, false);
            lm.cylinder(Vec3(p.x(), cy - 2.5f, p.z()), Vec3(p.x(), cy - 3.0f, p.z()), 0.5f, 0.2f, 10, QColor(150, 105, 55), MAT_BRONZE);
            for (int s = 0; s < 3; ++s) { float aa = s * 2.094f; lm.torus(Vec3(p.x(), cy - 0.4f - s * 0.9f, p.z()), Vec3(0, 1, 0), 0.52f, 0.05f, QColor(110, 80, 50), MAT_BRONZE, 12, 4); (void)aa; }
            // tubo de bronze para a bateria mais proxima
            Vec3 bt = batt[(i / 3) % 4];
            Vec3 s0(p.x(), cy - 3.0f, p.z());
            Vec3 ctrl = (s0 + bt) * 0.5f + Vec3(0, -4.f, 0);
            Prefabs::pipe(lm, bezier(s0, ctrl, bt + Vec3(0, 0.5f, 0), 12), 0.10f, MAT_BRONZE, true);
            w.addLight(Vec3(p.x(), cy - 1.2f, p.z()), Vec3(0.3f, 1.f, 0.45f), 1.6f, 7.f, LightAnim::Pulse, Q("sanguessuga"))->animFreq = 0.4f;
        }
        w.addStatic(std::move(lm));
    }
    {   // Dissipador do Vazio: disco negro giratorio acima do Coracao
        const Vec3 vc(100, -256, -505);
        MeshBuilder vm;
        vm.cylinder(Vec3(0, -0.2f, 0), Vec3(0, 0.25f, 0), 5.2f, 5.2f, 40, QColor(4, 2, 8), MAT_COAL, true, false);
        vm.cylinder(Vec3(0, 0.25f, 0), Vec3(0, 0.3f, 0), 5.25f, 5.25f, 40, QColor(10, 4, 20), MAT_COAL);
        vm.torus(Vec3(0, 0.05f, 0), Vec3(0, 1, 0), 5.45f, 0.22f, QColor(170, 80, 255), MAT_EMISSIVE, 40, 6);
        vm.torus(Vec3(0, 0.05f, 0), Vec3(0, 1, 0), 3.4f, 0.1f, QColor(120, 50, 200), MAT_EMISSIVE, 32, 5);
        vm.torus(Vec3(0, 0.05f, 0), Vec3(0, 1, 0), 1.7f, 0.08f, QColor(200, 120, 255), MAT_EMISSIVE, 24, 5);
        for (int i = 0; i < 12; ++i) { float a = i * 0.5236f; vm.box(Vec3(std::cos(a) * 4.4f, 0.05f, std::sin(a) * 4.4f), Vec3(0.12f, 0.03f, 0.9f), QColor(150, 70, 230), MAT_EMISSIVE); }
        DynHandle vd = w.addDynamic(Q("vazio"), std::move(vm), vc);
        w.addUpdater([vd](float, float t) { vd->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, t * 55.f) * QQuaternion::fromAxisAndAngle(1, 0, 0, std::sin(t * 0.7f) * 2.f)); });
        MeshBuilder sup;   // suportes de ferro para a espinha
        for (int i = 0; i < 4; ++i) {
            float a = i * 1.5708f + 0.785f;
            sup.cylinder(vc + Vec3(std::cos(a) * 5.4f, 0.2f, std::sin(a) * 5.4f), Vec3(vc.x() + std::cos(a) * 3.f, ceilY(100, -505) + 1.f, vc.z() + std::sin(a) * 3.f), 0.26f, 6, QColor(80, 60, 48), MAT_RUSTY_IRON);
        }
        w.addStatic(std::move(sup));
        EmitterDesc e; e.preset = ParticlePreset::VoidStream; ParticleEmitter::applyPreset(e);
        e.name = Q("fluxo para o vazio"); e.pos = Vec3(100, -277.f, -505); e.halfExtent = Vec3(3.2f, 0.5f, 3.2f); e.target = vc; e.rate = 130.f;
        e.lifeMin = 5.f; e.lifeMax = 7.f; e.velocity = Vec3(0, 3.5f, 0); e.sizeMin = 0.12f; e.sizeMax = 0.3f; e.activeRange = 170.f;
        w.addEmitter(e);
        w.addLight(vc + Vec3(0, -2, 0), Vec3(0.6f, 0.25f, 1.f), 6.f, 30.f, LightAnim::Pulse, Q("vazio"))->animFreq = 0.8f;
    }
    {   // 4 baterias arcanas (violeta) nos pilares do fosso
        MeshBuilder bm;
        for (int k = 0; k < 4; ++k) {
            Vec3 b = pol(26.f, 45.f + 90.f * k, -299.6f);
            bm.cylinder(b, b + Vec3(0, 0.9f, 0), 2.2f, 2.0f, 10, QColor(70, 54, 46), MAT_RUSTY_IRON);
            bm.cylinder(b + Vec3(0, 0.9f, 0), b + Vec3(0, 6.9f, 0), 0.55f, 0.55f, 10, QColor(170, 90, 255), MAT_EMISSIVE);
            for (int s = 0; s < 9; ++s) bm.torus(b + Vec3(0, 1.4f + s * 0.62f, 0), Vec3(0, 1, 0), 1.45f - 0.04f * s, 0.2f, QColor(92, 72, 58), MAT_RUSTY_IRON, 16, 5);
            for (int s = 0; s < 4; ++s) { float a = s * 1.5708f; bm.cylinder(b + Vec3(std::cos(a) * 1.9f, 0.9f, std::sin(a) * 1.9f), b + Vec3(std::cos(a) * 1.7f, 7.0f, std::sin(a) * 1.7f), 0.1f, 5, QColor(150, 105, 55), MAT_BRONZE); }
            bm.cone(b + Vec3(0, 6.9f, 0), b + Vec3(0, 8.4f, 0), 0.5f, 6, QColor(210, 150, 255), MAT_EMISSIVE);
            Vec3 top = b + Vec3(0, 7.2f, 0);
            Vec3 dirIn = (HEART - Vec3(b.x(), HEART.y(), b.z())); dirIn.setY(0); dirIn.normalize();
            Prefabs::pipe(bm, bezier(top, top + dirIn * 8.f + Vec3(0, 3.f, 0), Vec3(HEART.x(), HEART.y() - 1.f, HEART.z()) - dirIn * 7.0f, 12), 0.2f, MAT_BRONZE, true);
            w.addLight(top, Vec3(0.62f, 0.26f, 1.f), 7.f, 28.f, LightAnim::Pulse, Q("bateria"))->animFreq = 1.1f;
            EmitterDesc e; e.preset = ParticlePreset::Sparks; ParticleEmitter::applyPreset(e);
            e.pos = top + Vec3(0, 1.f, 0); e.halfExtent = Vec3(0.4f, 0.3f, 0.4f); e.rate = 14.f; e.colorStart = QColor(210, 160, 255, 255); e.colorEnd = QColor(120, 60, 255, 0); e.activeRange = 140.f;
            w.addEmitter(e);
            w.addCylinderCollider(Q("bateria_%1").arg(k), b + Vec3(0, 3.5f, 0), 2.1f, 3.6f);
        }
        w.addStatic(std::move(bm));
    }

    // ======================================================== DESCIDA DE FERRO: escadas, arcos, canos
    {
        MeshBuilder dm;
        for (size_t i = 0; i + 1 < T.size(); ++i) {
            Vec3 a = T[i] + Vec3(0, -0.15f, 0), b = T[i + 1] + Vec3(0, -0.15f, 0);
            if (i == 0) continue;     // trecho horizontal junto a porta: piso de grade simples
            Prefabs::ironStairs(dm, a, b, 2.6f);
        }
        Prefabs::gratedWalkway(dm, T[0] + Vec3(0, 0.02f, 0), T[1] + Vec3(0, 0.02f, 0), 3.f, true);
        // arcos de ferro enferrujado a cada ~6 m + canos de bronze na parede
        for (size_t i = 0; i + 1 < T.size(); ++i) {
            Vec3 d = T[i + 1] - T[i]; float L = d.length(); Vec3 dh(d.x(), 0, d.z()); dh.normalize();
            Vec3 side = Vec3::crossProduct(Vec3(0, 1, 0), dh).normalized();
            int n = (int)(L / 6.f);
            for (int k = 1; k <= n; ++k) {
                Vec3 p = T[i] + d * (k / (float)(n + 1));
                std::vector<Vec3> arc;
                for (int s = 0; s <= 10; ++s) { float a = kPi_ * s / 10.f; arc.push_back(p + side * (std::cos(a) * 2.45f) + Vec3(0, 0.4f + std::sin(a) * 3.0f, 0)); }
                dm.tube(arc, 0.14f, 6, QColor(86, 64, 52), MAT_RUSTY_IRON);
                dm.sphere(arc.front(), 0.24f, QColor(60, 46, 40), MAT_RUSTY_IRON, 6);
                dm.sphere(arc.back(), 0.24f, QColor(60, 46, 40), MAT_RUSTY_IRON, 6);
            }
            std::vector<Vec3> pp = {T[i] + side * 2.3f + Vec3(0, 1.6f, 0), T[i + 1] + side * 2.3f + Vec3(0, 1.6f, 0)};
            Prefabs::pipe(dm, pp, 0.09f, MAT_BRONZE, true);
        }
        w.addStatic(std::move(dm));
        w.addLight(Vec3(106, -252.8f, -410), Vec3(0.3f, 0.9f, 0.5f), 3.f, 14.f, LightAnim::Pulse, Q("descida"));
        w.addLight(Vec3(115, -262.2f, -430), Vec3(1.f, 0.4f, 0.15f), 4.f, 16.f, LightAnim::Pulse, Q("descida"))->animFreq = 0.55f;
        w.addLight(Vec3(118, -275.2f, -455), Vec3(1.f, 0.35f, 0.12f), 5.f, 18.f, LightAnim::Pulse, Q("descida"))->animFreq = 0.55f;
        w.addLight(Vec3(121, -281.5f, -468), Vec3(1.f, 0.33f, 0.1f), 6.f, 22.f, LightAnim::Pulse, Q("descida"))->animFreq = 0.55f;
        w.addEmitter(ParticlePreset::Embers, Vec3(118, -276, -455), Vec3(3, 2, 8), 24.f, Q("brasas da descida"));
    }

    // ======================================================== ALTAR DO ALHOON
    {
        MeshBuilder am;
        using namespace Prefabs;
        // escada sobre o fosso ate a plataforma
        Vec3 sb(100, FL, -524.2f), st(100, -296.f, -529.6f);
        ironStairs(am, sb, st, 2.0f);
        slabCol(w, Q("escada_altar"), sb, st, 2.2f, 0.3f);
        for (int s = -1; s <= 1; s += 2) am.cylinder(Vec3(100 + s * 1.2f, FL - 2.f, -528.f), Vec3(100 + s * 1.2f, -298.2f, -528.f), 0.16f, 6, QColor(70, 54, 46), MAT_RUSTY_IRON);
        // pulpito de controle com alavancas, runas e medidores
        altar(am, Vec3(100, -296, -534.4f), 180.f, Vec3(3.2f, 1.15f, 1.1f), MAT_OBSIDIAN);
        am.box(Vec3(100, -294.75f, -534.5f), Vec3(1.5f, 0.05f, 0.45f), QColor(170, 255, 200), MAT_EMISSIVE);   // runas verdes no tampo
        for (int i = 0; i < 4; ++i) lever(am, Vec3(98.8f + i * 0.8f, -294.85f, -534.0f), 180.f, i % 2 ? 40.f : -30.f);
        // painel/estrutura traseira com medidores
        am.box(Vec3(100, -293.6f, -539.4f), Vec3(4.6f, 2.8f, 0.3f), QColor(40, 36, 36), MAT_IRON);
        for (int i = 0; i < 5; ++i) {
            am.cylinder(Vec3(96.5f + i * 1.7f, -292.8f, -539.05f), Vec3(96.5f + i * 1.7f, -292.8f, -538.85f), 0.45f, 0.45f, 14, QColor(220, 200, 150), MAT_PAPER);
            am.box(Vec3(96.5f + i * 1.7f, -292.8f, -538.8f), Vec3(0.03f, 0.3f, 0.01f), QColor(220, 50, 40), MAT_EMISSIVE);
        }
        for (int i = 0; i < 6; ++i) am.cylinder(Vec3(94.5f + i * 2.f, -296.f, -541.f), Vec3(94.5f + i * 2.f, -289.5f, -541.f), 0.12f, 6, QColor(150, 105, 55), MAT_BRONZE);
        // mesas dos Diarios de Calibracao
        table(am, Vec3(94, -296, -531.2f), 12.f, Vec3(2.2f, 0.95f, 1.1f), true);
        table(am, Vec3(106, -296, -531.2f), -12.f, Vec3(2.2f, 0.95f, 1.1f), true);
        for (int s = -1; s <= 1; s += 2) {
            Vec3 b(100 + s * 6.f, -296.f + 0.96f, -531.f);
            for (int i = 0; i < 5; ++i) am.box(b + Vec3(0.2f * i - 0.4f, 0.05f * i, 0.1f * (i % 2)), Vec3(0.28f, 0.025f, 0.2f), QColor(228 - 10 * i, 214 - 8 * i, 176), MAT_PAPER);
        }
        // circulo de runas no piso
        am.torus(Vec3(100, -295.97f, -537.5f), Vec3(0, 1, 0), 4.2f, 0.04f, QColor(150, 255, 190), MAT_EMISSIVE, 40, 4);
        am.torus(Vec3(100, -295.97f, -537.5f), Vec3(0, 1, 0), 3.4f, 0.04f, QColor(170, 110, 255), MAT_EMISSIVE, 36, 4);
        // lampadas / braseiros de bronze
        brazier(am, Vec3(89.5f, -296, -533.f), 1.1f, true);
        brazier(am, Vec3(110.5f, -296, -533.f), 1.1f, true);
        // O Alhoon: manto purpura, capuz, olhos brancos, tentaculos
        am.push(); am.translate(100, -296, -536.3f); am.rotateY(180.f);
        HumanoidParams hp; hp.height = 1.95f; hp.build = 0.82f; hp.cloth = QColor(140, 62, 196); hp.cloth2 = QColor(96, 40, 140); hp.skin = QColor(140, 120, 150);
        hp.cape = true; hp.hood = true; hp.glowingEyes = true; hp.eyeColor = QColor(255, 255, 255);
        humanoid(am, Vec3(0, 0, 0), 0.f, hp);
        for (int i = 0; i < 5; ++i) {
            float sx = (i - 2) * 0.055f;
            std::vector<Vec3> tp = {Vec3(sx, 1.66f, -0.17f), Vec3(sx * 1.6f, 1.55f, -0.32f), Vec3(sx * 2.2f, 1.30f, -0.36f + 0.03f * (i % 2)), Vec3(sx * 2.6f, 1.05f, -0.32f)};
            am.tube(tp, std::vector<float>{0.045f, 0.04f, 0.03f, 0.014f}, 6, QColor(120, 86, 150), MAT_FLESH);
        }
        am.sphere(Vec3(-0.075f, 1.71f, -0.17f), 0.026f, QColor(255, 255, 255), MAT_EMISSIVE, 5);
        am.sphere(Vec3(0.075f, 1.71f, -0.17f), 0.026f, QColor(255, 255, 255), MAT_EMISSIVE, 5);
        am.pop();
        w.addStatic(std::move(am));
        w.addLight(Vec3(100, -293.5f, -533.f), Vec3(0.5f, 1.f, 0.7f), 4.f, 14.f, LightAnim::Pulse, Q("pulpito"))->animFreq = 0.4f;
        w.addLight(Vec3(100, -294.2f, -533.2f), Vec3(0.8f, 0.55f, 1.f), 5.f, 9.f, LightAnim::Pulse, Q("olhos do alhoon"))->animFreq = 0.3f;
        w.addLight(Vec3(90, -294.f, -542.f), Vec3(0.7f, 0.3f, 1.f), 4.f, 16.f, LightAnim::Pulse, Q("altar roxo"))->animFreq = 0.5f;
        // interativos: alavancas e diarios
        auto lv = std::make_shared<int>(0);
        w.addInteractable(Q("alavancas_altar"), Vec3(100, -294.9f, -534.1f), 1.5f, Q("Tocar nas alavancas do Alhoon"), [&w, lv]() {
            static const QStringList msgs = {
                Q("As alavancas estão quentes. Um medidor oscila: 'RENDIMENTO 97,4%'. O Alhoon, sem se virar: 'Não toque nisso. Cada décimo é uma rua da cidade.'"),
                Q("As engrenagens do teto gemem. O Alhoon: 'Se quiser ajudar, ajuste a Vaga. Está desbalanceada desde que o último conduíte se esgotou.'"),
                Q("Uma runa pisca vermelha: 'ESGOTADO: 9 de 36'. Atrás de você, algo racha em uma cápsula.")};
            w.state.toast(msgs[*lv % msgs.size()], QColor(210, 255, 220), 7.f); ++*lv;
        });
        auto pg = std::make_shared<int>(0);
        auto diary = [&w, pg](const Vec3& p, const QString& nm) {
            w.addInteractable(nm, p, 1.4f, Q("Ler os Diários de Calibração"), [&w, pg]() {
                static const QStringList pages = {
                    Q("«Sujeito 11 (humana, 34 anos): ruptura psicológica ao 19º dia. Eficiência térmica 0,81 kJ/h·kg. Observação: os gritos são dados, não ruído.»"),
                    Q("«Sujeito 23 (anão, 61 anos): resistência recorde, 41 dias. Eficiência 1,12. Recomendação: priorizar anões. Pesar custo de captura.»"),
                    Q("«Sujeito 30 (meio-elfo, 9 anos): colapso imediato. Eficiência 0,22. Retirado do arranjo. Registro: não repetir faixa etária.»"),
                    Q("«A agonia de cinquenta é o custo de operação para a paz de cinquenta milhões. Anotar: ainda não consegui produzir culpa. Tentar de novo amanhã.»")};
                w.state.toast(pages[*pg % pages.size()] + Q("  [%1/4]").arg(*pg % pages.size() + 1), QColor(235, 225, 190), 9.f); ++*pg;
            });
        };
        diary(Vec3(94, -294.9f, -531.2f), Q("diarios_oeste"));
        diary(Vec3(106, -294.9f, -531.2f), Q("diarios_leste"));
    }

    // ======================================================== ARQUIVO DE CALIBRACAO
    {
        MeshBuilder am;
        using namespace Prefabs;
        Rng rg(88);
        // estantes: norte e sul da sala
        auto shelf = [&](const Vec3& c, float yaw, float len) {
            am.push(); am.translate(c); am.rotateY(yaw);
            am.box(Vec3(0, 1.1f, 0), Vec3(len * 0.5f, 1.1f, 0.35f), QColor(52, 40, 34), MAT_WOOD);
            for (int lvl = 0; lvl < 4; ++lvl) {
                float y = 0.3f + lvl * 0.6f;
                am.box(Vec3(0, y, 0.05f), Vec3(len * 0.5f, 0.02f, 0.33f), QColor(80, 60, 44), MAT_WOOD);
                int nb = (int)(len * 4.f);
                for (int b = 0; b < nb; ++b) {
                    if (rg.f() < 0.15f) continue;
                    float hh = rg.r(0.18f, 0.34f);
                    am.box(Vec3(-len * 0.5f + 0.1f + b * (len - 0.2f) / nb, y + hh * 0.5f + 0.02f, 0.05f), Vec3(0.05f, hh * 0.5f, 0.2f), QColor((int)rg.r(70, 170), (int)rg.r(40, 90), (int)rg.r(30, 70)), MAT_PAPER);
                }
                if (lvl == 2) for (int j = 0; j < 4; ++j) {   // frascos de especimes
                    Vec3 jp(-len * 0.35f + j * len * 0.22f, y + 0.18f, 0.08f);
                    am.cylinder(jp - Vec3(0, 0.15f, 0), jp + Vec3(0, 0.15f, 0), 0.1f, 0.1f, 8, QColor(180, 220, 200, 90), MAT_GLASS, false);
                    am.sphere(jp - Vec3(0, 0.05f, 0), 0.05f, QColor(200, 150, 140), MAT_FLESH, 5);
                }
            }
            am.pop();
        };
        shelf(Vec3(46, FL, -506.4f), 0.f, 9.f);
        shelf(Vec3(46, FL, -493.6f), 180.f, 9.f);
        shelf(Vec3(38.4f, FL, -500), 90.f, 6.f);
        table(am, Vec3(46, FL, -500), 90.f, Vec3(2.4f, 0.95f, 1.2f), true);
        for (int i = 0; i < 3; ++i) glassCapsule(am, Vec3(41 + i * 0.01f, FL, -496.3f + i * 1.2f), 1.5f, 0.38f, false, true, false);
        HumanoidParams hp; hp.pose = Pose::Sit; hp.cloth = QColor(60, 50, 70); hp.skin = QColor(180, 170, 160);
        // pedra solta + filacteria escondida no nicho do muro oeste
        am.box(Vec3(37.95f, -298.85f, -500), Vec3(0.3f, 0.4f, 0.45f), QColor(20, 16, 24), MAT_OBSIDIAN);                       // nicho
        am.box(Vec3(38.05f, -299.f, -500), Vec3(0.12f, 0.12f, 0.16f), QColor(30, 26, 30), MAT_IRON);                           // cofre
        am.sphere(Vec3(38.18f, -298.9f, -500), 0.09f, QColor(190, 80, 255), MAT_EMISSIVE, 8);                                  // gema
        w.addStatic(std::move(am));
        w.addLight(Vec3(46, -296.f, -500), Vec3(1.f, 0.85f, 0.55f), 7.f, 18.f, LightAnim::TorchFlicker, Q("arquivo"));
        w.addLight(Vec3(38.4f, -298.8f, -500), Vec3(0.75f, 0.3f, 1.f), 0.9f, 4.f, LightAnim::Pulse, Q("filacteria"))->animFreq = 0.6f;
        MeshBuilder sm;
        sm.box(Vec3(0, 0, 0), Vec3(0.28f, 0.45f, 0.5f), QColor(76, 66, 70), MAT_OBSIDIAN);
        sm.box(Vec3(-0.05f, 0.1f, 0.1f), Vec3(0.2f, 0.25f, 0.3f), QColor(96, 84, 88), MAT_OBSIDIAN);
        DynHandle stone = w.addDynamic(Q("pedra_solta"), std::move(sm), Vec3(38.75f, -298.8f, -500));
        auto revealed = std::make_shared<bool>(false);
        w.addInteractable(Q("pedra_solta"), Vec3(38.9f, -298.8f, -500), 1.3f, Q("Investigar a pedra solta (Investigação CD 18)"), [&w, revealed, stone]() {
            if (*revealed) { w.state.toast(Q("O compartimento está aberto: um cofre de ferro com uma gema violeta pulsando. A Filactéria do Alhoon."), QColor(220, 170, 255), 6.f); return; }
            *revealed = true;
            stone->pos += Vec3(0, 0, 1.4f);
            w.state.toast(Q("Investigação CD 18: uma pedra solta! Atrás dela, um nicho com um cofre de ferro e uma gema violeta: a FILACTÉRIA do Alhoon."), QColor(220, 170, 255), 8.f);
            w.state.setFlag(Q("filacteria_encontrada"));
        });
    }

    // ======================================================== EVENTO: COLAPSO [K]
    {
        struct Rock { DynHandle d = nullptr; bool on = false; Vec3 vel; float landY = FL; };
        struct Col {
            bool on = false; float t = 0.f, nextRock = 0.f, nextShake = 0.f; int idx = 0; int stage = 0;
            std::vector<Rock> rocks; Light* red = nullptr; EmitterHandle dust = nullptr, debris = nullptr;
        };
        auto cs = std::make_shared<Col>();
        for (int i = 0; i < 18; ++i) {
            MeshBuilder rm; Rng rg(900 + i);
            float s = rg.r(0.5f, 1.5f);
            rm.ellipsoid(Vec3(0, 0, 0), Vec3(s, s * 0.8f, s * 0.9f), QColor(92, 52, 40), MAT_HOT_BASALT, 7);
            rm.ellipsoid(Vec3(s * 0.4f, s * 0.2f, 0), Vec3(s * 0.55f, s * 0.5f, s * 0.5f), QColor(110, 62, 46), MAT_HOT_BASALT, 6);
            Rock r; r.d = w.addDynamic(Q("rocha_colapso_%1").arg(i), std::move(rm), Vec3(HC.x(), -400.f, HC.z())); r.d->visible = false;
            cs->rocks.push_back(r);
        }
        cs->red = w.addLight(Vec3(100, -268, -505), Vec3(1.f, 0.04f, 0.02f), 14.f, 90.f, LightAnim::Custom, Q("emergencia"));
        cs->red->custom = [](float t) { return 0.2f + 0.8f * std::pow(0.5f + 0.5f * std::sin(t * 7.f), 2.f); };
        cs->red->enabled = false;
        EmitterDesc d; d.preset = ParticlePreset::Dust; ParticleEmitter::applyPreset(d);
        d.pos = Vec3(100, -275, -505); d.halfExtent = Vec3(40, 20, 38); d.rate = 700.f; d.sizeMin = 0.1f; d.sizeMax = 0.35f; d.colorStart = QColor(190, 150, 120, 130); d.colorEnd = QColor(170, 130, 100, 0);
        d.velocity = Vec3(0, -0.4f, 0); d.turbulence = 0.7f; d.lifeMin = 3.f; d.lifeMax = 7.f; d.activeRange = 190.f; d.enabled = false;
        cs->dust = w.addEmitter(d);
        EmitterDesc d2; d2.preset = ParticlePreset::Sparks; ParticleEmitter::applyPreset(d2);
        d2.pos = Vec3(100, -255, -505); d2.halfExtent = Vec3(38, 3, 36); d2.rate = 260.f; d2.velocity = Vec3(0, -9.f, 0); d2.velJitter = 1.5f; d2.gravity = Vec3(0, -9.f, 0);
        d2.colorStart = QColor(255, 140, 60, 255); d2.colorEnd = QColor(120, 40, 20, 0); d2.sizeMin = 0.06f; d2.sizeMax = 0.16f; d2.lifeMin = 1.5f; d2.lifeMax = 3.5f; d2.activeRange = 190.f; d2.enabled = false;
        cs->debris = w.addEmitter(d2);

        w.registerEvent(Q("colapso"), Q("Colapso: o tremor da montanha (5.8)"), Qt::Key_K, [&w, cs]() {
            cs->on = !cs->on; cs->t = 0.f; cs->stage = 0; cs->nextRock = 0.f; cs->nextShake = 0.f;
            cs->red->enabled = cs->on; cs->dust->setEnabled(cs->on); cs->debris->setEnabled(cs->on);
            if (cs->on) { w.state.toast(Q("COLAPSO! A montanha treme: o Coração do Dragão bate fora de ritmo..."), QColor(255, 120, 100), 5.f); w.state.screenShake(0.14f, 3.f); w.state.flash(QColor(255, 60, 40), 0.5f); }
            else w.state.toast(Q("O tremor cessa (Mestre)."), QColor(220, 220, 220), 3.f);
        });
        w.addUpdater([&w, cs, heart](float dt, float t) {
            if (!cs->on) {
                for (Rock& r : cs->rocks) if (r.on) { r.on = false; r.d->visible = false; }
                return;
            }
            cs->t += dt;
            // toasts roteirizados
            struct Msg { float t; const char* txt; };
            static const Msg msgs[] = {{4.f, "Pedaços do teto despencam! Destreza CD 14 ou 4d10 de dano contundente (CD 10 em áreas cobertas)."},
                                       {9.f, "Os andaimes gemem. Uma cápsula racha e o líquido verde escorre: um conduíte grita."},
                                       {15.f, "O Coração do Dragão acelera. O chão racha: o fosso de magma transborda as bordas."},
                                       {22.f, "FUJAM! A Descida de Ferro é o único caminho de volta. Corram para cima!"},
                                       {32.f, "O tremor diminui... por enquanto. Tem um novo ritmo no Coração: é a contagem regressiva."}};
            while (cs->stage < 5 && cs->t >= msgs[cs->stage].t) { w.state.toast(QString::fromUtf8(msgs[cs->stage].txt), QColor(255, 180, 140), 6.f); ++cs->stage; }
            if (cs->t >= cs->nextShake) { w.state.screenShake(0.10f + 0.04f * std::sin(cs->t), 1.6f); cs->nextShake = cs->t + 1.4f; }
            // rochas caindo ao redor do jogador
            cs->nextRock -= dt;
            if (cs->nextRock <= 0.f) {
                cs->nextRock = 0.28f;
                Rock& r = cs->rocks[cs->idx++ % cs->rocks.size()];
                Vec3 pp = w.playerPos();
                float ang = (cs->idx * 2.3f), dd = 3.f + std::fmod(cs->idx * 7.3f, 16.f);
                Vec3 p(pp.x() + std::cos(ang) * dd, 0, pp.z() + std::sin(ang) * dd);
                p.setY(ceilY(p.x(), p.z()) - 1.5f);
                p.setY(std::max(p.y(), pp.y() + 4.f));
                RayHit h = w.raycast(p, Vec3(0, -1, 0), 120.f, false);
                r.landY = h.hit ? h.pos.y() + 0.5f : FL;
                r.d->pos = p; r.vel = Vec3(0, -2.f, 0); r.on = true; r.d->visible = true;
            }
            for (Rock& r : cs->rocks) if (r.on) {
                r.vel += Vec3(0, -16.f * dt, 0);
                r.d->pos += r.vel * dt;
                r.d->setRotation(QQuaternion::fromAxisAndAngle(Vec3(1, 0.4f, 0.2f), t * 160.f));
                if (r.d->pos.y() <= r.landY) {
                    r.on = false; r.d->visible = false;
                    if ((r.d->pos - w.playerPos()).length() < 9.f) w.state.screenShake(0.16f, 0.5f);
                }
            }
            heart->scl *= (1.f + 0.04f * std::sin(t * 18.f));
            if (cs->t > 60.f) { cs->on = false; cs->red->enabled = false; cs->dust->setEnabled(false); cs->debris->setEnabled(false); }
        });
    }

    // ======================================================== POIs
    auto poi = [&](const Vec3& p, PoiCategory c, const QString& t, const QString& txt) { w.addPoi(p, c, t, txt, kSess, kArea); };
    poi(Vec3(104, -253.4f, -407.5f), PoiCategory::Area, Q("5.1 A Descida de Ferro"),
        Q("Atrás da Porta de Adamante: um túnel de 5,6 m de largura que desce ~40 m em quatro lances (grade de ferro enferrujada, arcos de ferro a cada ~6 m, canos de bronze zumbindo na parede). A rocha vai de obsidiana fria (24 °C) para BASALTO QUENTE com rachaduras que pulsam (45 °C).\n\n"
          "PONTOS: (104,-255,-408) → (115,-265,-430) → (118,-278,-455) → boca da câmara (121,-284,-470) → patamar de grade → escada espiral na parede (r 38,5 de 60° a 108°) → chão (88,-300,-468).\n"
          "O som: o coração do dragão, ouvido de longe, a cada 1,8 s. O calor aumenta a cada lance: Constituição CD 10 por hora para evitar Exaustão por calor (a Escama do Dragão dá vantagem/resistência). A Lâmina da Brasa (se o grupo a tiver) brilha quase branca."));
    poi(Vec3(121, -282.2f, -468), PoiCategory::Area, Q("5.2 A Boca da Câmara do Coração (primeira vista)"),
        Q("O túnel abre em um patamar de grade pendurado na parede sudeste, ~16 m acima do chão. Deixe os jogadores olharem ANTES de descrever:\n"
          "'O ar bate quente no rosto. Abaixo, uma caverna do tamanho de uma catedral, vermelha como o interior de uma pálpebra. No centro, um órgão do tamanho de uma casa, incandescente, que SE CONTRAI. Em volta dele, andaimes de ferro enferrujado como uma gaiola, cheios de cilindros de vidro verde-doentio. Dentro de cada um, uma pessoa. Dezenas. Acima, um disco negro gira e engole a luz.'\n\n"
          "Teste: Sabedoria CD 12 para não ficar Amedrontado 1 rodada pelo horror (humanos reconhecíveis lá dentro: Brenn, a lenhadora Siv, o filho do padeiro...)."));
    poi(Vec3(100, -292.4f, -505), PoiCategory::Area, Q("5.2 A Câmara do Coração (a caverna é o dragão)"),
        Q("Elipsoide de centro (100,-285,-505), raios (48,38,45), chão plano em y -300, teto em ~-247 (a bíblia dá centro y -275 e raio vertical 28: foi ajustado para o chão comportar fosso, anéis e altar). A rocha é basalto quente com rachaduras que pulsam. TODA a caverna é o corpo do dragão morto: 8 pares de COSTELAS FÓSSEIS arqueiam do chão até a ESPINHA no teto (eixo norte–sul), o CRÂNIO emerge da parede norte com a boca aberta e as órbitas com brilho fraco.\n\n"
          "Temperatura 45 °C. Luz: o pulsar vermelho do Coração (1,8 s) domina; verde-doentio das cápsulas; violeta das baterias e do Vazio. Visibilidade >60 m. A tocha quase não importa aqui.\n\n"
          "Atalhos do Mestre: K = Colapso. Furtividade é quase impossível: o Alhoon está virado para os conduítes, mas 'sente' cada passo na rede (percepção passiva 20)."));
    poi(HEART + Vec3(0, 9.4f, 0), PoiCategory::Area, Q("5.2 O Coração do Dragão"),
        Q("Centro (100,-287,-505), raio ~8 m, sobre um pedestal de rocha de ~7,4 m de raio. Órgão incandescente com veias de fogo, pulsando em ~1,8 s (duplo batimento 'lub-dub'). A luz vermelha pulsa junto.\n\n"
          "É a bateria térmica da montanha — e está INCUBANDO. A Máquina do Frio o drena; se continuar, a vila congela. Se for destruído/desligado de forma errada, vira erupção. Tocar nele: 8d10 de fogo (nem se fala); mágicas de Dissipar Magia (CD 19) afetam apenas as sanguessugas. É o EVENTO CENTRAL: o que o grupo decidir aqui decide o fim da campanha.\n\n"
          "LORE: quando o Coração bate, a Escama do Dragão Ancestral (se o grupo a tiver) responde batendo junto. O Alhoon a vê e sorri: 'Você a trouxe de volta para casa.'"));
    poi(pol(26.f, 100.f, -299.5f), PoiCategory::Trap, Q("5.2 O Fosso de Magma e a Ponte de Ferro"),
        Q("Anel de magma entre r 22 e 30 m em torno do Coração (superfície em y -301, 1 m abaixo do piso). Cair: 10d10 de fogo (no programa: respawn no último chão seguro). A PONTE DE FERRO (3,2 m de largura, ~14 m) atravessa o fosso pelo sul (θ 100°). Há uma segunda travessia: as escadas de grade do altar, ao norte.\n\n"
          "Prisioneiro quase sempre ignorado: o magma está 'mais baixo' do que seria. O Alhoon abaixou o nível para impedir fugas, e se o grupo sabotar as baterias o fosso transborda (ver Colapso)."));
    poi(pol(16.6f, 15.f, -297.5f), PoiCategory::Creature, Q("5.3 Os Andaimes e as 36 Cápsulas"),
        Q("Três anéis de andaime de grade de ferro (r 16, largura 3 m) em y -300, -292 e -284, ligados por 4 lances de escada de ferro (anel 1→2 em 0°–60° e 180°–240°; anel 2→3 em 120°–180° e 300°–360°) e vigas diagonais enferrujadas CRAVADAS na parede. Cada anel tem 12 CÁPSULAS (cilindros de vidro e ferro, 2,4 m): 36 no total.\n\n"
          "Dentro, um humano com TUBOS CRAVADOS NO PEITO, pele queimando e cicatrizando sem parar, líquido necromântico verde. Tubos de bronze ligam cada cápsula ao Coração. Algumas estão ESGOTADAS (vidro rachado, escuro): as mentes viraram pó (11 no total). Os conduítes ainda vivos sussurram, às vezes com a voz de entes queridos dos jogadores.\n\n"
          "MECÂNICA: abrir/romper uma cápsula (CA 15, 20 PV, vidro) libera o conduíte; ele morre em 1d4 rodadas se desconectado do Coração (a menos que Cura Maior/Restauração Maior + ritual). Ver 'A Vaga' e o dilema final."));
    poi(pol(16.6f, 285.f, kRingY[1] + 1.4f), PoiCategory::Creature, Q("5.3 A Vaga (cápsula vazia e aberta)"),
        Q("No anel do meio, voltada para o Altar (norte): uma cápsula VAZIA e ABERTA, porta de vidro escancarada, tubos soltos balançando, ainda quentes. É a vaga que o Alhoon pede que um aventureiro ocupe voluntariamente.\n\n"
          "Se alguém entra: o Alhoon liga o conduíte e o 'rendimento' sobe de 97,4% para 100%. A vila ganha ~1 mês de calor extra e o Alhoon ENTREGA o grupo e o resto. O voluntário sofre 6d10 psíquico/fogo por dia (o grupo pode resgatá-lo após 1 dia; Teste de Constituição CD 20, falha = morte).\n"
          "FRAME DO DILEMA (problema do bonde): 36 pessoas sofrem para que 50.000 vivam; 1 voluntário reduz o sofrimento dos outros 36 (e retarda o apocalipse). Perguntas à mesa: O consentimento muda a conta? É diferente 'matar' o Alhoon e deixar de 'tocar' na alavanca? A vila merece sobreviver às custas deles?"));
    poi(Vec3(100, -289.f, -480), PoiCategory::Trap, Q("5.4 As Sanguessugas Mecânicas (10)"),
        Q("Dez cilindros de vidro com líquido necromântico e tubos de bronze pendem do teto irregular como sanguessugas mecânicas. Drenam o calor do Coração e o levam às baterias arcanas (tubos de bronze descem em arcos até o topo de cada bateria).\n\n"
          "Cada uma: CA 15, 25 PV; destruir 3 reduz o rendimento (o Alhoon fala: 'Isso tem custo.'). Se destruir todas as 10, o fluxo para o Vazio cai a 0 e o Coração acelera (Colapso iminente: use [K]). Vidro estilhaçado: Destreza CD 12 ou 2d6 cortante + 2d6 necrótico (líquido)."));
    poi(Vec3(100, -258.5f, -505), PoiCategory::Trap, Q("5.5 O Dissipador do Vazio"),
        Q("Acima do Coração, a 31 m do chão: um DISCO NEGRO GIRATÓRIO de ~11 m com anéis violeta, preso à espinha por vigas de ferro. Um fluxo de partículas de calor SOBE do Coração e some no disco. É literalmente a energia do continente sendo jogada fora.\n\n"
          "Tocar o disco: absorção (Constituição CD 20 ou 10d10 necrótico + perde 1 nível de exaustão em vez de curar). Magias de luz perto dele são consumidas (a tocha apaga; 'luz que entra e não sai'). Mage Hand/telecinese no disco: o objeto some. DESLIGAR: destruir as 4 baterias OU sabotar o púlpito: o disco para em ~3 rodadas e o Coração começa a bater mais forte."));
    for (int k = 0; k < 4; ++k) {
        Vec3 b = pol(26.f, 45.f + 90.f * k, -299.6f);
        poi(b + Vec3(0, 8.f, 0), PoiCategory::Trap, Q("5.5 Bateria Arcana %1").arg(k + 1),
            Q("Bobina de ferro enrolada em um pilar de rocha no meio do fosso, com núcleo violeta. CA 17, 60 PV; ao ser destruída explode (raio 6 m: 4d8 elétrico + 4d8 de força; Destreza CD 16 metade). Uma bateria destruída reduz o rendimento (sobrecarga nas outras). As quatro estão ligadas ao Coração por tubos de bronze; o pilar é cercado de magma: precisa de voo, magia ou alcance. (25–27 m do centro.)"));
    }
    poi(Vec3(100, -294.4f, -531.f), PoiCategory::Area, Q("5.6 O Altar do Alhoon"),
        Q("Plataforma elevada ao norte (4 m acima do chão), alcançada por uma escada de ferro que cruza o fosso. Púlpito de obsidiana com ALAVANCAS (quatro), runas verdes no tampo e um painel de medidores (rendimento 97,4%). As alavancas ajustam o fluxo: uma delas, se puxada, abre as cápsulas.\n\n"
          "O Alhoon está de costas para os jogadores, olhando os conduítes. Duas mesas com os Diários de Calibração. Braseiros de bronze. Atrás: a plataforma encosta no crânio do dragão acima."));
    poi(Vec3(100, -294.2f, -536.3f), PoiCategory::Creature, Q("5.6 O Alhoon (Caótico Neutro, exausto)"),
        Q("Alhoon (illithid lich; use o bloco do Alhoon: CA 15, 120 PV, Mind Blast, Mage Armor, Counterspell, Dominate Monster; reduza o dano de linha de ação para manter o foco na decisão). Manto púrpura, rosto de tentáculos, olhos brancos brilhantes. UTILITARISTA: frio, preciso, exausto — não é cruel por prazer. Fala em frases curtas, como um contador.\n\n"
          "FALA (leia à mesa): «Se vocês me matarem e devolverem o fogo à montanha, a cidade viverá por mais um mês, e então o céu choverá cinzas e o continente queimará. Sentem-se. Aqueçam-se no fogo que resta, e assistam essa cidade morrer para que o mundo viva.»\n\n"
          "A OFERTA: ele pede que UM aventureiro entre voluntariamente na Vaga. Argumentos dele: o maquinário colapsa (último conduíte esgotado), um novo reduziria o sofrimento total; o fogo do Coração é finito; 'cinquenta milhões contra cinquenta'. Ele NÃO mente (é caótico, mas honesto): oferece rolagem de Insight e responde com números. Se o grupo ataca: luta (psíquico + dissipador).\n\n"
          "SE DESTRUÍREM A MÁQUINA: o fogo volta à montanha, a vila se salva (o menino doente se cura), mas a montanha TREME — contagem regressiva para o apocalipse: dispare o COLAPSO [K]."));
    poi(Vec3(94, -294.4f, -531.2f), PoiCategory::Lore, Q("5.6 Diários de Calibração (mesa oeste)"),
        Q("Planilhas que catalogam o ponto exato de RUPTURA PSICOLÓGICA e a EFICIÊNCIA TERMODINÂMICA DA DOR de cada raça. Ele anota os gritos como dados: 'Sujeito 11 (humana, 34 anos): ruptura ao 19º dia', 'anões: resistência recorde'.\n\n"
          "Frase-chave, escrita 3 vezes, a última rasurada: «A agonia de cinquenta é o custo de operação para a paz de cinquenta milhões.» — e embaixo: 'Ainda não consegui produzir culpa. Tentar de novo amanhã.' [E] folheia 4 páginas. Insight/Medicina CD 12: ele escreve sobre si como se fosse mais um sujeito."));
    poi(Vec3(106, -294.4f, -531.2f), PoiCategory::Lore, Q("5.6 Diários de Calibração (mesa leste)"),
        Q("Mesa com os registros mais recentes e uma tabela de substituição: o último conduíte da fileira C esgotou-se há 4 dias; o próximo sujeito 'ideal' foi marcado a lápis: um nome da vila (a pessoa mais querida de um dos PJs). Também o plano 'B': 'candidato voluntário = rendimento +3,1% e menos manutenção'."));
    poi(Vec3(55, -298.f, -500), PoiCategory::Area, Q("5.7 O Arquivo de Calibração (sala lateral)"),
        Q("Alcova oeste (a bíblia dá centro (55,-298,-500) raio 7; no programa a sala ocupa x 37..55, ligada à câmara por um corredor de 13 m). Estantes de diários, frascos de espécimes (órgãos, mechas de cabelo, olhos), uma mesa de dissecção e três cápsulas de teste esgotadas.\n\n"
          "Conteúdo: o histórico completo da Máquina (cinquenta anos de experimentos em 'sujeitos' de todas as raças, incluindo goblins da superfície). Investigação CD 12: o Alhoon começou a Máquina quando uma cidade vizinha morreu congelada: ele jura que a 'matemática' dessa morte o persegue."));
    poi(Vec3(40.2f, -298.8f, -500), PoiCategory::Secret, Q("5.7 A Filactéria do Alhoon (segredo do Mestre)"),
        Q("Atrás de uma PEDRA SOLTA no muro oeste (Investigação CD 18, ou Percepção passiva 23): um nicho com um cofre de ferro e uma gema violeta pulsando — a FILACTÉRIA do Alhoon. [E] na pedra a revela.\n\n"
          "Destruir a filactéria (CA 20, 40 PV; imune a dano não mágico; resistência a necrótico) impede o Alhoon de se reerguer: sem ela, o Alhoon morto é MORTO. Sem destruí-la, ele reaparece em 1d10 dias (e o ritual de 'colapso' o reativa). O Alhoon, se interrogado, mente sobre a localização (a única mentira dele)."));
    poi(Vec3(100, -298.2f, -475), PoiCategory::Trap, Q("5.8 O Colapso (evento do Mestre [K])"),
        Q("Se a máquina for destruída: o fogo VOLTA à montanha (a vila e o menino doente se curam), mas um TREMOR GRAVE abala tudo: rochas caindo, luz vermelha pulsando de emergência, poeira, andaimes gemendo, o Coração acelerado. Fuga alucinada para a superfície.\n\n"
          "No programa: [K] liga/desliga o Colapso: screen shake contínuo, rochas dinâmicas caindo perto do jogador (4d10 contundente se atingido; Destreza CD 14), poeira e fagulhas, luz de emergência vermelha pulsante, 5 toasts roteirizados (0, 4, 9, 15, 22, 32 s). Dura 60 s ou até apertar [K] de novo. Sugestão: use a regra de perseguição com 10 obstáculos (CD 13) na subida da Descida de Ferro; quem falhar leva 2d10 fogo + 1 de Exaustão."));
    poi(Vec3(100, -288.f, -530.f), PoiCategory::Area, Q("5.2 O Crânio do Dragão e as Costelas Fósseis"),
        Q("Crânio colossal (~14 m de comprimento) emergindo da parede norte em (100,-280,-548), boca aberta, dentes de ~1,4 m, órbitas com um brilho fraco, laranja. Oito pares de costelas gigantes (raio ~1 m) arqueiam do chão até a espinha dorsal no teto; vértebras com espinhos. Para o Mestre: este é o último grande dragão de fogo da região; a máquina o usa como 'carcaça de refrigeração'.\n\n"
          "Quando o Coração bate, a luz nas órbitas pulsa junto. Percepção CD 15: um grande olho vermelho abre por um décimo de segundo a cada batida (pista de ressurreição)."));

    // ======================================================== PONTOS DE VISTA
    auto vp = [&](const QString& n, const Vec3& p, float yaw, float pitch, bool fly) { w.addViewpoint(n, p, yaw, pitch, fly, kGrp); };
    vp(Q("5.1 A Descida de Ferro (túnel)"), Vec3(115, -263.35f, -430), yawTo(Vec3(115, 0, -430), Vec3(118, 0, -455)), -6.f, false);
    vp(Q("5.2 A Boca da Câmara do Coração"), Vec3(121.5f, -282.35f, -468.f), yawTo(Vec3(121.5f, 0, -468), Vec3(100, 0, -505)), -12.f, false);
    vp(Q("5.2b O Coração, da ponte de ferro"), pol(26.f, 100.f, -298.2f), yawTo(pol(26.f, 100.f, 0), HC), 14.f, false);
    vp(Q("5.2c As Costelas Fósseis"), pol(18.f, 100.f, -298.35f), 10.f, 28.f, false);
    vp(Q("5.2d O Crânio do Dragão"), pol(19.5f, 270.f, -298.35f), 0.f, 34.f, false);
    vp(Q("5.3 Andaimes e Cápsulas (anel do meio)"), pol(15.f, 200.f, -290.35f), yawTo(pol(15.f, 200.f, 0), pol(15.f, 215.f, 0)) + 35.f, -2.f, false);
    vp(Q("5.3b A Vaga (vista do Altar)"), Vec3(100.5f, -294.35f, -530.5f), yawTo(Vec3(100.5f, 0, -530.5f), pol(16.6f, 285.f, 0)), 17.f, false);
    vp(Q("5.4 As Sanguessugas Mecânicas"), pol(19.5f, 225.f, -298.35f), yawTo(pol(19.5f, 225.f, 0), pol(12.f, 46.f, 0)), 52.f, false);
    vp(Q("5.5 O Dissipador do Vazio"), pol(15.5f, 330.f, -282.35f), yawTo(pol(15.5f, 330.f, 0), HC), 58.f, false);
    vp(Q("5.5b As Baterias Arcanas"), pol(19.5f, 180.f, -298.35f), yawTo(pol(19.5f, 180.f, 0), pol(26.f, 225.f, 0)) , 8.f, false);
    vp(Q("5.6 O Altar do Alhoon"), Vec3(103, -294.35f, -530.0f), 25.f, -4.f, false);   // na plataforma: do pe da escada o pulpito esconde o Alhoon
    vp(Q("5.6b Diários de Calibração"), Vec3(100, -294.35f, -530.2f), 0.f, -14.f, false);
    vp(Q("5.7 O Arquivo de Calibração"), Vec3(49, -298.35f, -500), 90.f, -2.f, false);
    vp(Q("5.8 Colapso (visão de fuga)"), pol(31.f, 45.f, -298.35f), yawTo(pol(31.f, 45.f, 0), Vec3(121, 0, -470)), 10.f, false);
    vp(Q("5.9 A Câmara do Coração inteira (voo, sul)"), Vec3(100, -262, -478), 0.f, -18.f, true);
    vp(Q("5.9b A Câmara do Coração inteira (voo, leste)"), Vec3(140, -280, -505), 90.f, -8.f, true);

    // ======================================================== ROTA
    w.addRoutePoint(Q("Descida de Ferro · início"), Vec3(104, -254.4f, -407.5f));
    w.addRoutePoint(Q("Descida de Ferro · lance 1"), T[2] + Vec3(0, 0.6f, 0));
    w.addRoutePoint(Q("Descida de Ferro · lance 2"), T[3] + Vec3(0, 0.6f, 0));
    w.addRoutePoint(Q("Boca da Câmara do Coração"), T[4] + Vec3(0, 0.6f, 0));
    w.addRoutePoint(Q("Escada espiral · meio"), pol(38.5f, 84.f, -292.f + 0.6f));
    w.addRoutePoint(Q("Escada espiral · chão"), pol(38.5f, 108.f, FL + 0.6f));
    w.addRoutePoint(Q("Ponte de Ferro · entrada"), pol(33.f, 100.f, FL + 0.55f));
    w.addRoutePoint(Q("Ilha do Coração (sul)"), pol(19.5f, 100.f, FL + 0.6f));
    w.addRoutePoint(Q("Ilha do Coração (oeste)"), pol(19.5f, 180.f, FL + 0.6f));
    w.addRoutePoint(Q("Ilha do Coração (noroeste)"), pol(19.5f, 225.f, FL + 0.6f));
    w.addRoutePoint(Q("Pé da escada do Altar"), Vec3(100, FL + 0.6f, -524.5f));
    w.addRoutePoint(Q("Altar do Alhoon"), Vec3(100, -295.4f, -532));
}
