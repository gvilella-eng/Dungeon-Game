// Camada 1 — props, criaturas, luzes e particulas (a historia contada pelos objetos).
#include "Layer1_Common.h"
#include "Prefabs.h"

using Prefabs::Pose;

namespace L1 {
namespace {

World* gw = nullptr;

float gy(float x, float z, float y0) { return groundY(*gw, x, z, y0); }
Vec3 onFloor(float x, float z, float y0) { return Vec3(x, gy(x, z, y0), z); }

// Ponto da superficie da rocha seguindo 'dir' a partir de 'from' (ar). Falha -> ok=false.
bool wallPoint(const Vec3& from, const Vec3& dir, float maxD, Vec3& p, Vec3& n)
{
    RayHit h = gw->raycast(from, dir.normalized(), maxD, false);
    if (!h.hit) return false;
    p = h.pos; n = h.normal;
    if (n.lengthSquared() < 0.5f) n = -dir.normalized();
    return true;
}

// Espiral de ocre pintada na parede, acompanhando a superficie real da rocha.
void wallSpiral(MeshBuilder& mb, const Vec3& from, const Vec3& dir, float r, float turns, const QColor& col = QColor(168, 66, 34))
{
    Vec3 p, n;
    if (!wallPoint(from, dir, 25.f, p, n)) return;
    Vec3 e1 = Vec3::crossProduct(Vec3(0, 1, 0), n);
    if (e1.lengthSquared() < 1e-4f) e1 = Vec3(1, 0, 0);
    e1.normalize();
    Vec3 e2 = Vec3::crossProduct(n, e1).normalized();
    std::vector<Vec3> pts;
    int cnt = (int)(18 * turns) + 4;
    for (int i = 0; i <= cnt; ++i) {
        float t = (float)i / cnt, a = t * turns * 6.2832f, rr = r * t;
        Vec3 q = p + e1 * (std::cos(a) * rr) + e2 * (std::sin(a) * rr);
        Vec3 pp, nn;
        if (wallPoint(q + n * 0.6f, -n, 1.4f, pp, nn)) pts.push_back(pp + nn * 0.03f); else pts.push_back(q + n * 0.03f);
    }
    mb.tube(pts, 0.024f, 4, col, MAT_CLOTH);
}
// Marcas de arranhao (protecao) em grupos de linhas
void wallScratches(MeshBuilder& mb, const Vec3& from, const Vec3& dir, int seed, const QColor& col = QColor(214, 204, 180))
{
    Vec3 p, n;
    if (!wallPoint(from, dir, 25.f, p, n)) return;
    Vec3 e1 = Vec3::crossProduct(Vec3(0, 1, 0), n);
    if (e1.lengthSquared() < 1e-4f) e1 = Vec3(1, 0, 0);
    e1.normalize();
    Vec3 e2 = Vec3::crossProduct(n, e1).normalized();
    for (int k = 0; k < 4; ++k) {
        float off = (k - 1.5f) * 0.13f;
        float tilt = (hrand(seed, k) - 0.5f) * 0.25f;
        std::vector<Vec3> pts;
        for (int i = 0; i <= 5; ++i) {
            float t = (i / 5.f - 0.5f) * 0.9f;
            Vec3 q = p + e1 * (off + tilt * t) + e2 * t;
            Vec3 pp, nn;
            if (wallPoint(q + n * 0.6f, -n, 1.4f, pp, nn)) pts.push_back(pp + nn * 0.02f); else pts.push_back(q + n * 0.02f);
        }
        mb.tube(pts, 0.012f, 3, col, MAT_CLOTH);
    }
}

// Pote/jarra de barro (base em y=0)
void clayPot(MeshBuilder& mb, const Vec3& c, float s, const QColor& col = QColor(150, 92, 58))
{
    mb.push(); mb.translate(c); mb.scale(s);
    mb.ellipsoid(Vec3(0, 0.22f, 0), Vec3(0.2f, 0.22f, 0.2f), col, MAT_LIMESTONE, 8);
    mb.cylinder(Vec3(0, 0.38f, 0), Vec3(0, 0.5f, 0), 0.1f, 0.12f, 8, col, MAT_LIMESTONE);
    mb.cylinder(Vec3(0, 0.48f, 0), Vec3(0, 0.505f, 0), 0.115f, 0.13f, 8, QColor(110, 66, 40), MAT_LIMESTONE);
    mb.pop();
}
// Cobertor roubado da vila: dobrado ou estendido
void blanket(MeshBuilder& mb, const Vec3& c, float yaw, float sx, float sz, float h, const QColor& col, const QColor& stripe)
{
    mb.push(); mb.translate(c); mb.rotateY(yaw);
    mb.box(Vec3(0, h * 0.5f, 0), Vec3(sx * 0.5f, h * 0.5f, sz * 0.5f), col, MAT_CLOTH);
    for (int i = 0; i < 3; ++i) mb.box(Vec3(0, h + 0.003f, -sz * 0.3f + i * sz * 0.3f), Vec3(sx * 0.5f, 0.004f, sz * 0.035f), stripe, MAT_CLOTH);
    mb.pop();
}
void fur(MeshBuilder& mb, const Vec3& c, float yaw, int seed)
{
    QColor col(70 + (int)(hrand(seed, 1) * 40), 52 + (int)(hrand(seed, 2) * 26), 38 + (int)(hrand(seed, 3) * 18));
    mb.push(); mb.translate(c); mb.rotateY(yaw);
    mb.ellipsoid(Vec3(0, 0.05f, 0), Vec3(0.5f, 0.06f, 0.8f), col, MAT_HIDE, 8);
    mb.ellipsoid(Vec3(0.1f, 0.1f, 0.35f), Vec3(0.2f, 0.07f, 0.17f), col.darker(120), MAT_HIDE, 6);   // travesseiro de trapos
    mb.pop();
}
void logStack(MeshBuilder& mb, const Vec3& c, float yaw)
{
    mb.push(); mb.translate(c); mb.rotateY(yaw);
    const QColor wood(104, 72, 44);
    for (int layer = 0; layer < 3; ++layer) {
        int n = 4 - layer;
        for (int i = 0; i < n; ++i) {
            float x = (i - (n - 1) * 0.5f) * 0.27f, y = 0.11f + layer * 0.2f;
            mb.cylinder(Vec3(x, y, -0.7f), Vec3(x, y, 0.7f), 0.11f, 7, wood, MAT_WOOD);
        }
    }
    mb.pop();
}
void coalPile(MeshBuilder& mb, const Vec3& c, float r, int seed)
{
    for (int i = 0; i < 46; ++i) {
        float a = hrand(seed, i, 1) * 6.283f, d = std::sqrt(hrand(seed, i, 2)) * r;
        float h = (1.f - d / r) * r * 0.7f;
        float s = 0.07f + hrand(seed, i, 3) * 0.09f;
        mb.ellipsoid(c + Vec3(std::cos(a) * d, h + s * 0.4f, std::sin(a) * d), Vec3(s, s * 0.7f, s * 0.9f), QColor(26, 25, 28), MAT_COAL, 6);
    }
}
void arrowClay(MeshBuilder& mb, const Vec3& c, float yaw)
{
    mb.push(); mb.translate(c); mb.rotateY(yaw);
    mb.cylinder(Vec3(-0.25f, 0.015f, 0), Vec3(0.2f, 0.02f, 0.f), 0.008f, 4, QColor(110, 80, 56), MAT_WOOD);
    mb.ellipsoid(Vec3(0.26f, 0.03f, 0), Vec3(0.05f, 0.03f, 0.03f), QColor(150, 98, 66), MAT_LIMESTONE, 6);   // ponta de barro
    mb.pop();
}
void fetish(MeshBuilder& mb, const Vec3& ceil, float len, int seed)
{
    std::vector<Vec3> cord = {ceil, ceil + Vec3(0.02f, -len * 0.5f, 0.01f), ceil + Vec3(0, -len, 0)};
    mb.tube(cord, 0.01f, 4, QColor(110, 86, 60), MAT_ROPE);
    Vec3 e = ceil + Vec3(0, -len, 0);
    int k = seed % 3;
    if (k == 0) Prefabs::skull(mb, e + Vec3(0, -0.12f, 0), hrand(seed, 4) * 360.f, 0.55f);
    else if (k == 1) {
        mb.cylinder(e + Vec3(-0.22f, 0, 0), e + Vec3(0.22f, 0, 0.03f), 0.018f, 5, QColor(220, 210, 184), MAT_BONE);
        mb.cylinder(e + Vec3(0.02f, 0.18f, 0), e + Vec3(-0.02f, -0.2f, 0.02f), 0.018f, 5, QColor(220, 210, 184), MAT_BONE);
        mb.torus(e + Vec3(0, 0, 0.01f), Vec3(0, 0, 1), 0.1f, 0.012f, QColor(130, 100, 70), MAT_ROPE, 10, 4);
    } else {
        mb.box(e + Vec3(0, -0.1f, 0), Vec3(0.03f, 0.18f, 0.008f), QColor(40, 36, 36), MAT_HIDE);   // pena
        mb.box(e + Vec3(0.06f, -0.2f, 0), Vec3(0.025f, 0.16f, 0.008f), QColor(60, 54, 50), MAT_HIDE);
        mb.sphere(e + Vec3(0, -0.02f, 0), 0.05f, QColor(220, 210, 184), MAT_BONE, 6);
    }
    if (seed % 2 == 0) mb.cylinder(e + Vec3(0, -0.3f, 0), e + Vec3(0.03f, -0.6f, 0), 0.008f, 4, QColor(110, 86, 60), MAT_ROPE);
}
void chalice(MeshBuilder& mb, const Vec3& c)
{
    const QColor gold(236, 190, 70);
    mb.push(); mb.translate(c);
    mb.cylinder(Vec3(0, 0, 0), Vec3(0, 0.03f, 0), 0.09f, 0.08f, 10, gold, MAT_GOLD);
    mb.cylinder(Vec3(0, 0.03f, 0), Vec3(0, 0.2f, 0), 0.018f, 0.02f, 8, gold, MAT_GOLD);
    mb.sphere(Vec3(0, 0.12f, 0), 0.032f, gold, MAT_GOLD, 8);
    mb.cylinder(Vec3(0, 0.2f, 0), Vec3(0, 0.38f, 0), 0.04f, 0.095f, 12, gold, MAT_GOLD);
    mb.torus(Vec3(0, 0.38f, 0), Vec3(0, 1, 0), 0.095f, 0.01f, gold, MAT_GOLD, 12, 4);
    mb.sphere(Vec3(0.03f, 0.29f, 0.09f), 0.014f, QColor(200, 30, 50), MAT_GLASS, 5);   // gema
    mb.pop();
}
void clayArrowBundle(MeshBuilder& mb, const Vec3& c)
{
    mb.push(); mb.translate(c);
    for (int i = 0; i < 9; ++i) {
        float a = i * 0.7f, r = 0.05f * (i % 3);
        mb.cylinder(Vec3(std::cos(a) * r, 0, std::sin(a) * r), Vec3(std::cos(a) * r + 0.03f, 0.55f, std::sin(a) * r), 0.007f, 4, QColor(110, 80, 56), MAT_WOOD);
        mb.ellipsoid(Vec3(std::cos(a) * r + 0.03f, 0.57f, std::sin(a) * r), Vec3(0.025f, 0.04f, 0.025f), QColor(150, 98, 66), MAT_LIMESTONE, 5);
    }
    mb.cylinder(Vec3(0, 0.2f, 0), Vec3(0, 0.23f, 0), 0.08f, 6, QColor(100, 74, 50), MAT_ROPE);
    mb.pop();
}
// carcaça congelada (cabra): corpo, pernas, cabeça com chifres, camada de gelo
void frozenGoat(MeshBuilder& mb, const Vec3& c, float yaw)
{
    mb.push(); mb.translate(c); mb.rotateY(yaw);
    const QColor fur(112, 102, 94);
    mb.ellipsoid(Vec3(0, 0.45f, 0), Vec3(0.62f, 0.3f, 0.26f), fur, MAT_HIDE, 9);
    for (int i = 0; i < 4; ++i) {
        float x = (i < 2 ? -0.38f : 0.38f), z = (i % 2 ? 0.14f : -0.14f);
        mb.cylinder(Vec3(x, 0.32f, z), Vec3(x * 1.12f, 0.02f, z * 1.3f), 0.045f, 0.03f, 5, fur.darker(115), MAT_HIDE);
    }
    mb.cylinder(Vec3(0.5f, 0.55f, 0), Vec3(0.82f, 0.74f, 0), 0.1f, 0.075f, 6, fur, MAT_HIDE);
    mb.ellipsoid(Vec3(0.9f, 0.77f, 0), Vec3(0.14f, 0.1f, 0.09f), fur, MAT_HIDE, 6);
    mb.cylinder(Vec3(0.88f, 0.86f, 0.05f), Vec3(0.78f, 1.05f, 0.1f), 0.022f, 0.01f, 5, QColor(200, 190, 170), MAT_BONE);
    mb.cylinder(Vec3(0.88f, 0.86f, -0.05f), Vec3(0.78f, 1.05f, -0.1f), 0.022f, 0.01f, 5, QColor(200, 190, 170), MAT_BONE);
    mb.ellipsoid(Vec3(0.1f, 0.47f, 0), Vec3(0.72f, 0.38f, 0.34f), QColor(190, 225, 245, 70), MAT_CLEAR_ICE, 9);
    mb.ellipsoid(Vec3(0.86f, 0.76f, 0), Vec3(0.22f, 0.17f, 0.15f), QColor(190, 225, 245, 70), MAT_CLEAR_ICE, 7);
    mb.pop();
}
void frozenBear(MeshBuilder& mb, const Vec3& c, float yaw)
{
    mb.push(); mb.translate(c); mb.rotateY(yaw);
    const QColor fur(76, 56, 44);
    mb.ellipsoid(Vec3(0, 0.65f, 0), Vec3(1.1f, 0.62f, 0.7f), fur, MAT_HIDE, 10);
    mb.ellipsoid(Vec3(1.15f, 0.7f, 0), Vec3(0.45f, 0.38f, 0.36f), fur, MAT_HIDE, 8);
    mb.ellipsoid(Vec3(1.5f, 0.6f, 0), Vec3(0.28f, 0.17f, 0.17f), fur.lighter(120), MAT_HIDE, 6);
    mb.sphere(Vec3(1.1f, 1.02f, 0.2f), 0.09f, fur, MAT_HIDE, 6);
    mb.sphere(Vec3(1.1f, 1.02f, -0.2f), 0.09f, fur, MAT_HIDE, 6);
    for (int i = 0; i < 4; ++i) {
        float x = (i < 2 ? -0.7f : 0.7f), z = (i % 2 ? 0.45f : -0.45f);
        mb.cylinder(Vec3(x, 0.5f, z), Vec3(x * 1.1f, 0.06f, z * 1.2f), 0.17f, 0.13f, 6, fur.darker(110), MAT_HIDE);
    }
    mb.ellipsoid(Vec3(0.1f, 0.68f, 0), Vec3(1.25f, 0.76f, 0.88f), QColor(190, 225, 245, 65), MAT_CLEAR_ICE, 10);
    mb.ellipsoid(Vec3(1.2f, 0.72f, 0), Vec3(0.6f, 0.5f, 0.48f), QColor(190, 225, 245, 65), MAT_CLEAR_ICE, 8);
    mb.pop();
}
void boulder(MeshBuilder& mb, const Vec3& c, float rx, float ry, float rz, int seed)
{
    int g = 90 + (int)(hrand(seed, 1) * 40);
    mb.ellipsoid(c, Vec3(rx, ry, rz), QColor(g, g - 4, g - 10), MAT_LIMESTONE, 8);
}

} // namespace

void buildProps(World& w)
{
    gw = &w;
    State& S = st();
    using namespace Prefabs;

    // ================================================================ 1.1 ANTECAMARA: O FALSO RITUAL
    {
        MeshBuilder mb;
        const float cx = 0.f, cz = -17.f;
        // cinco braseiros sagrados de bronze em circulo (raio 3,5 m), brasas fracas
        for (int k = 0; k < 5; ++k) {
            float a = (90.f + 72.f * k) * 0.0174533f;
            Vec3 p = onFloor(cx + 3.5f * std::cos(a), cz + 3.5f * std::sin(a), -1.2f);
            MeshBuilder lit, dark;
            brazier(lit, Vec3(0, 0, 0), 1.05f, true);
            brazier(dark, Vec3(0, 0, 0), 1.05f, false);
            DynHandle a1 = w.addDynamic(Q("braseiro_ritual_%1").arg(k), std::move(lit), p);
            DynHandle a2 = w.addDynamic(Q("braseiro_ritual_apagado_%1").arg(k), std::move(dark), p);
            a2->visible = false;
            S.braziers.push_back({a1, a2});
            Light* l = w.addLight(p + Vec3(0, 1.0f, 0), Vec3(1.0f, 0.5f, 0.16f), 2.8f, 9.f, LightAnim::TorchFlicker, Q("brasa_ritual_%1").arg(k));
            l->phase = k * 1.3f;
            S.embers.push_back(l);
            w.addEmitter(ParticlePreset::Embers, p + Vec3(0, 0.95f, 0), Vec3(0.18f, 0.05f, 0.18f), 7.f, Q("brasas_ritual_%1").arg(k));
        }
        // Relicario do Sol sobre uma pedra, no centro
        Vec3 rc = onFloor(cx, cz, -1.2f);
        mb.hexPrism(rc + Vec3(0, 0.42f, 0), 0.62f, 0.84f, QColor(104, 98, 90), MAT_LIMESTONE);
        mb.hexPrism(rc + Vec3(0, 0.9f, 0), 0.5f, 0.08f, QColor(128, 120, 108), MAT_LIMESTONE);
        const QColor gold(238, 192, 74);
        mb.push(); mb.translate(rc + Vec3(0, 1.62f, 0));
        mb.cylinder(Vec3(0, 0, -0.04f), Vec3(0, 0, 0.04f), 0.5f, 0.5f, 28, gold, MAT_GOLD);
        mb.cylinder(Vec3(0, 0, 0.04f), Vec3(0, 0, 0.065f), 0.34f, 0.3f, 24, gold.lighter(112), MAT_GOLD);
        mb.torus(Vec3(0, 0, 0.0f), Vec3(0, 0, 1), 0.5f, 0.035f, gold.darker(110), MAT_GOLD, 28, 6);
        for (int i = 0; i < 16; ++i) {
            float a = i * 6.2832f / 16.f;
            float l = (i % 2) ? 0.2f : 0.34f;
            mb.cone(Vec3(std::cos(a) * 0.52f, std::sin(a) * 0.52f, 0), Vec3(std::cos(a) * (0.52f + l), std::sin(a) * (0.52f + l), 0), 0.045f, 5, gold, MAT_GOLD);
        }
        mb.sphere(Vec3(0, 0, 0.085f), 0.1f, QColor(255, 224, 140), MAT_GOLD, 8);
        mb.cylinder(Vec3(0, -0.5f, 0), Vec3(0, -0.78f, 0), 0.05f, 0.07f, 8, QColor(150, 105, 55), MAT_BRONZE);   // haste
        mb.pop();
        w.addLight(rc + Vec3(0, 1.7f, 0.8f), Vec3(1.0f, 0.78f, 0.4f), 1.6f, 6.f, LightAnim::Pulse, Q("brilho_relicario"))->animFreq = 0.4f;
        // pilha de carvao (-6, -22), lenha (7, -24), cobertores roubados (-7, -10)
        Vec3 cp = onFloor(-6.f, -22.f, -1.5f); coalPile(mb, cp, 1.3f, 5);
        sack(mb, cp + Vec3(1.5f, 0, 0.5f), 20.f, 1.0f); sack(mb, cp + Vec3(1.9f, 0, -0.5f), 80.f, 0.9f);
        Vec3 lp = onFloor(7.f, -24.f, -1.8f);
        logStack(mb, lp, 20.f); logStack(mb, lp + Vec3(-1.0f, 0, 0.9f), -10.f);
        mb.cylinder(lp + Vec3(0.8f, 0.1f, 1.0f), lp + Vec3(1.7f, 0.1f, 1.2f), 0.09f, 6, QColor(110, 78, 50), MAT_WOOD);
        Vec3 bp = onFloor(-7.f, -10.f, -0.6f);
        blanket(mb, bp + Vec3(0, 0, 0), 15.f, 1.1f, 0.8f, 0.14f, QColor(150, 46, 40), QColor(210, 190, 150));
        blanket(mb, bp + Vec3(0.05f, 0.14f, 0.02f), -10.f, 1.0f, 0.75f, 0.12f, QColor(52, 94, 72), QColor(200, 190, 150));
        blanket(mb, bp + Vec3(-0.05f, 0.26f, 0.0f), 28.f, 0.9f, 0.7f, 0.12f, QColor(48, 74, 128), QColor(210, 200, 170));
        blanket(mb, bp + Vec3(0.02f, 0.38f, 0.03f), 5.f, 0.85f, 0.65f, 0.1f, QColor(168, 126, 52), QColor(110, 70, 40));
        blanket(mb, onFloor(-5.6f, -11.4f, -0.7f), 70.f, 1.7f, 1.2f, 0.02f, QColor(120, 70, 52), QColor(200, 180, 140));   // estendido (dormir)
        // sinais de que e abrigo e nao ritual: peles para dormir, ossos de rato, restos de comida
        for (int i = 0; i < 6; ++i) {
            float a = (30.f + i * 60.f) * 0.0174533f;
            Vec3 fp = onFloor(cx + 5.6f * std::cos(a), cz + 5.2f * std::sin(a), -1.2f);
            fur(mb, fp, 90.f - i * 60.f + 20.f, 30 + i);
            bonesPile(mb, fp + Vec3(0.7f, 0, 0.3f), 0.25f, 40 + i);
            skull(mb, fp + Vec3(-0.6f, 0, 0.5f), i * 70.f, 0.22f);
            for (int j = 0; j < 4; ++j)
                mb.ellipsoid(fp + Vec3(0.2f + hrand(i, j, 5) * 0.8f, 0.02f, -0.5f + hrand(i, j, 6)), Vec3(0.05f, 0.015f, 0.04f), QColor(110, 84, 56), MAT_HIDE, 5);
            if (i % 2 == 0) clayPot(mb, fp + Vec3(-0.9f, 0, -0.1f), 0.55f);
        }
        for (int i = 0; i < 7; ++i) {
            Vec3 p = onFloor(cx + (hrand(i, 71) - 0.5f) * 6.f, cz + (hrand(i, 72) - 0.5f) * 6.f, -1.2f);
            mb.ellipsoid(p + Vec3(0, 0.02f, 0), Vec3(0.12f, 0.02f, 0.06f), QColor(80, 70, 64), MAT_HIDE, 5);   // pele de rato
        }
        // espirais de ocre vermelho nas paredes (goblins tentando 'chamar o calor')
        int sp = 0;
        for (float z : {-5.f, -9.f, -13.f, -17.f, -21.f, -25.f}) {
            float y0 = gy(0.f, z, -1.2f);
            wallSpiral(mb, Vec3(-1.f, y0 + 1.6f + hrand(sp, 1) * 1.8f, z), Vec3(-1, 0, 0), 0.45f + 0.2f * hrand(sp, 2), 2.6f);
            wallSpiral(mb, Vec3(1.f, y0 + 1.8f + hrand(sp, 3) * 1.8f, z - 2.f), Vec3(1, 0, 0), 0.4f + 0.25f * hrand(sp, 4), 2.4f);
            ++sp;
        }
        wallSpiral(mb, Vec3(0, 2.2f, -20.f), Vec3(0, 0, -1), 0.8f, 3.f);
        for (int i = 0; i < 5; ++i) {   // marcas de maos em ocre
            Vec3 p, n;
            if (wallPoint(Vec3(1.f, 0.2f + i * 0.2f, -7.f - i * 3.f), Vec3(1, 0, 0), 25.f, p, n))
                mb.box(p + n * 0.025f, Vec3(0.07f, 0.1f, 0.012f), QColor(150, 56, 30), MAT_CLOTH);
        }
        w.addStatic(std::move(mb));
    }

    // ================================================================ 1.2 CORREDOR DAS FENDAS
    {
        MeshBuilder mb;
        // --- fosso de estacas: estacas no fundo (so aparecem quando a lona cede)
        float fy = corrFloor(-47.5f);
        for (int i = 0; i < 16; ++i) {
            float x = -1.25f + (i % 4) * 0.83f + (hrand(i, 3) - 0.5f) * 0.25f;
            float z = -48.6f + (i / 4) * 0.74f + (hrand(i, 4) - 0.5f) * 0.25f;
            float lean = (hrand(i, 5) - 0.5f) * 0.25f;
            mb.cylinder(Vec3(x, fy - 2.98f, z), Vec3(x + lean, fy - 0.95f - hrand(i, 6) * 0.3f, z), 0.07f, 0.005f, 6, QColor(96 + i % 3 * 8, 70, 46), MAT_WOOD, true, false);
        }
        bonesPile(mb, Vec3(0.3f, fy - 2.98f, -47.f), 0.7f, 9);
        skull(mb, Vec3(-0.6f, fy - 2.9f, -47.8f), 40.f, 0.3f);
        ribcage(mb, Vec3(0.5f, fy - 2.95f, -48.f), 130.f, 0.35f);
        // flechas de barro quebradas e pocas sob as seteiras
        for (float z : {-34.f, -46.f, -58.f}) {
            for (int i = 0; i < 3; ++i) arrowClay(mb, onFloor(-2.0f + hrand((int)z, i, 1) * 1.4f, z - 0.5f + hrand((int)z, i, 2), corrFloor(z)), hrand((int)z, i, 3) * 180.f);
            mb.cylinder(onFloor(-1.6f, z + 0.6f, corrFloor(z)) + Vec3(0, 0.004f, 0), onFloor(-1.6f, z + 0.6f, corrFloor(z)) + Vec3(0, 0.014f, 0), 0.3f, 10, QColor(46, 54, 62), MAT_COAL);
        }
        for (float z : {-40.f, -52.f, -64.f}) {
            for (int i = 0; i < 3; ++i) arrowClay(mb, onFloor(0.6f + hrand((int)z, i, 4) * 1.4f, z - 0.5f + hrand((int)z, i, 5), corrFloor(z)), hrand((int)z, i, 6) * 180.f);
            mb.cylinder(onFloor(1.6f, z - 0.6f, corrFloor(z)) + Vec3(0, 0.004f, 0), onFloor(1.6f, z - 0.6f, corrFloor(z)) + Vec3(0, 0.014f, 0), 0.3f, 10, QColor(46, 54, 62), MAT_COAL);
        }
        // fio de tropeço (z -59) + prateleira de potes no teto
        float wy = corrFloor(-59.f);
        mb.tube({Vec3(-2.35f, wy + 0.17f, -59.f), Vec3(0.f, wy + 0.155f, -59.03f), Vec3(2.35f, wy + 0.17f, -59.f)}, 0.009f, 4, QColor(38, 34, 30), MAT_ROPE);
        mb.cylinder(Vec3(-2.38f, wy, -59.f), Vec3(-2.38f, wy + 0.3f, -59.f), 0.03f, 5, QColor(92, 66, 44), MAT_WOOD);
        mb.cylinder(Vec3(2.38f, wy, -59.f), Vec3(2.38f, wy + 0.3f, -59.f), 0.03f, 5, QColor(92, 66, 44), MAT_WOOD);
        const float shelfY = wy + 7.55f;
        mb.box(Vec3(0, shelfY - 0.05f, -60.2f), Vec3(2.4f, 0.04f, 0.55f), QColor(96, 70, 46), MAT_ROTTEN_WOOD);
        for (int i = -1; i <= 1; ++i) mb.box(Vec3(i * 2.0f, shelfY + 0.15f, -60.7f), Vec3(0.04f, 0.2f, 0.04f), QColor(90, 64, 42), MAT_WOOD);
        mb.tube({Vec3(-2.38f, wy + 0.3f, -59.f), Vec3(-2.36f, wy + 4.f, -59.4f), Vec3(-2.2f, shelfY - 0.05f, -59.9f)}, 0.008f, 4, QColor(38, 34, 30), MAT_ROPE);
        mb.tube({Vec3(2.38f, wy + 0.3f, -59.f), Vec3(2.36f, wy + 4.f, -59.4f), Vec3(2.2f, shelfY - 0.05f, -59.9f)}, 0.008f, 4, QColor(38, 34, 30), MAT_ROPE);
        w.addStatic(std::move(mb));
        // potes de barro com oleo e pedras (dinamicos: despencam ao tropecar)
        float px[6] = {-1.9f, -1.1f, -0.3f, 0.5f, 1.3f, 2.0f};
        for (int i = 0; i < 6; ++i) {
            MeshBuilder pm; clayPot(pm, Vec3(0, 0, 0), 0.9f + 0.2f * hrand(i, 8));
            S.pots.push_back(w.addDynamic(Q("pote_oleo_%1").arg(i), std::move(pm), Vec3(px[i], shelfY - 0.03f, -60.2f + (i % 2 ? 0.2f : -0.15f))));
        }
        float rx[4] = {-1.5f, -0.7f, 0.9f, 1.7f};
        for (int i = 0; i < 4; ++i) {
            MeshBuilder rm; boulder(rm, Vec3(0, 0.18f, 0), 0.2f + 0.05f * i, 0.17f, 0.2f, 70 + i);
            S.rocks.push_back(w.addDynamic(Q("pedra_prateleira_%1").arg(i), std::move(rm), Vec3(rx[i], shelfY - 0.03f, -60.45f)));
        }
        MeshBuilder om; om.cylinder(Vec3(0, 0, 0), Vec3(0, 0.012f, 0), 1.5f, 1.5f, 18, QColor(14, 11, 8), MAT_COAL);
        S.oilPuddle = w.addDynamic(Q("poca_de_oleo"), std::move(om), Vec3(0.f, wy + 0.005f, -58.3f));
        S.oilPuddle->visible = false;

        // lona suja e folhas sobre o fosso (dinamica)
        MeshBuilder cm;
        cm.box(Vec3(0, 0.008f, 0), Vec3(1.62f, 0.012f, 1.62f), QColor(98, 84, 68), MAT_CLOTH);
        for (int i = 0; i < 46; ++i) {
            float x = (hrand(i, 31) - 0.5f) * 3.1f, z = (hrand(i, 32) - 0.5f) * 3.1f;
            QColor c = (i % 3 == 0) ? QColor(96, 80, 52) : ((i % 3 == 1) ? QColor(70, 66, 56) : QColor(110, 92, 70));
            cm.push(); cm.translate(Vec3(x, 0.03f, z)); cm.rotateY(hrand(i, 33) * 360.f);
            cm.box(Vec3(0, 0, 0), Vec3(0.05f + 0.08f * hrand(i, 34), 0.004f, 0.03f + 0.05f * hrand(i, 35)), c, MAT_CLOTH);
            cm.pop();
        }
        for (int i = 0; i < 5; ++i) cm.ellipsoid(Vec3(-1.f + i * 0.5f, 0.02f, (hrand(i, 36) - 0.5f) * 2.6f), Vec3(0.18f, 0.02f, 0.14f), QColor(120, 104, 82), MAT_LIMESTONE, 6);
        float coverY = 0.5f * (gy(0.f, -45.4f, corrFloor(-45.4f)) + gy(0.f, -49.6f, corrFloor(-49.6f)));
        S.pitCover = w.addDynamic(Q("lona_fosso"), std::move(cm), Vec3(0, coverY, -47.5f));
        S.pitCol = w.addBoxCollider(Q("lona_fosso"), Vec3(0, -0.07f, 0), Vec3(1.5f, 0.07f, 1.5f));
        w.attachCollider(S.pitCol, S.pitCover);

        // olhos verdes dos goblins nas seteiras (acendem no evento G)
        auto eyes = [&](const Vec3& p, int idx) {
            MeshBuilder em;
            em.sphere(Vec3(0, 0, 0.09f), 0.034f, QColor(160, 255, 70), MAT_EMISSIVE, 6);
            em.sphere(Vec3(0, 0, -0.09f), 0.034f, QColor(160, 255, 70), MAT_EMISSIVE, 6);
            DynHandle h = w.addDynamic(Q("olhos_seteira_%1").arg(idx), std::move(em), p);
            h->visible = false;
            S.eyes.push_back(h);
        };
        int ei = 0;
        for (float z : {-34.f, -46.f, -58.f}) eyes(Vec3(-3.4f, corrFloor(z) + 4.8f + 0.55f, z), ei++);
        for (float z : {-40.f, -52.f, -64.f}) eyes(Vec3(3.4f, corrFloor(z) + 4.8f + 0.55f, z), ei++);
        for (float z : {-14.f, -22.f}) eyes(Vec3(12.6f, 4.05f, z), ei++);
        {
            MeshBuilder em;
            em.sphere(Vec3(0.09f, 0, 0), 0.034f, QColor(160, 255, 70), MAT_EMISSIVE, 6);
            em.sphere(Vec3(-0.09f, 0, 0), 0.034f, QColor(160, 255, 70), MAT_EMISSIVE, 6);
            DynHandle h = w.addDynamic(Q("olhos_ninho"), std::move(em), Vec3(-7.9f, 1.25f, -69.7f));
            h->visible = false; S.eyes.push_back(h);
        }
    }

    // ================================================================ 1.3 ABISMO: agua, estacas, ponte, linha goblin
    {
        MeshBuilder wm; wm.setFlow(1.f, 0.f);
        wm.boxMinMax(Vec3(-31.5f, -18.62f, -84.2f), Vec3(28.2f, -18.5f, -69.8f), QColor(40, 98, 128, 150), MAT_WATER);
        wm.setFlow(0.f, 0.1f);   // pocos do sifao: agua turva e quase parada
        wm.boxMinMax(Vec3(26.4f, -5.56f, -65.4f), Vec3(33.6f, -5.5f, -58.6f), QColor(70, 96, 86, 190), MAT_WATER);
        wm.boxMinMax(Vec3(26.4f, -5.56f, -97.4f), Vec3(33.6f, -5.5f, -90.6f), QColor(70, 96, 86, 190), MAT_WATER);
        w.addStatic(std::move(wm));
        MeshBuilder mb;
        // estacas fincadas no leito raso sob a ponte
        for (int i = 0; i < 18; ++i) {
            float x = -3.8f + (i % 6) * 1.5f + (hrand(i, 51) - 0.5f) * 0.5f;
            float z = -80.5f + (i / 6) * 3.3f + (hrand(i, 52) - 0.5f) * 0.9f;
            float lean = (hrand(i, 53) - 0.5f) * 0.45f, lean2 = (hrand(i, 54) - 0.5f) * 0.45f;
            float h = 2.2f + hrand(i, 55) * 1.0f;
            mb.cylinder(Vec3(x, -19.4f, z), Vec3(x + lean, -19.2f + h, z + lean2), 0.09f, 0.006f, 6, QColor(100 + i % 3 * 10, 76, 50), MAT_WOOD, true, false);
        }
        bonesPile(mb, Vec3(1.8f, -19.3f, -76.f), 0.6f, 12); skull(mb, Vec3(-1.6f, -19.3f, -78.f), 60.f, 0.9f);
        // cabeceiras: postes
        for (int s = -1; s <= 1; s += 2)
            for (float z : {-70.2f, -83.8f})
                mb.cylinder(Vec3(s * 0.85f, -4.2f, z), Vec3(s * 0.85f, -2.8f, z), 0.065f, 0.055f, 6, QColor(105, 76, 50), MAT_ROTTEN_WOOD);
        // Linha Goblin: corda unica em x +24 entre (24,-2,-70) e a alcova (24,-3,-86)
        mb.tube(MeshBuilder::catenary(Vec3(24.f, -1.3f, -70.9f), Vec3(24.f, -2.2f, -85.6f), 0.9f, 16), 0.028f, 5, QColor(146, 120, 84), MAT_ROPE);
        mb.cylinder(Vec3(24.f, -2.f, -70.8f), Vec3(24.f, -1.2f, -70.8f), 0.06f, 5, QColor(100, 74, 50), MAT_WOOD);
        mb.cylinder(Vec3(24.f, -3.f, -85.7f), Vec3(24.f, -2.1f, -85.7f), 0.06f, 5, QColor(100, 74, 50), MAT_WOOD);
        sack(mb, Vec3(23.2f, gy(23.2f, -87.6f, -3.1f), -87.6f), 20.f, 0.8f);
        w.addStatic(std::move(mb));

        // A Ponte Podre (dinamica): 20 tabuas, cedendo 0,6 m no meio
        {
            MeshBuilder bm;
            ropeBridge(bm, Vec3(0, 0, 6.7f), Vec3(0, 0, -6.7f), 1.5f, 0.6f, 20, true, 3);
            S.bridge = w.addDynamic(Q("ponte_podre"), std::move(bm), Vec3(0, -4.f, -77.f));
            const int segs = 8;
            auto P = [&](float t) { return Vec3(0.f, -0.6f * 4.f * t * (1.f - t), 6.7f - 13.4f * t); };
            for (int i = 0; i < segs; ++i) {
                Vec3 a = P((float)i / segs), b = P((float)(i + 1) / segs);
                Vec3 mid = (a + b) * 0.5f;
                Vec3 d = b - a;
                float len = d.length();
                QQuaternion q = QQuaternion::rotationTo(Vec3(0, 0, 1), d / len);
                Collider* c = w.addBoxCollider(Q("ponte_podre_%1").arg(i), mid + Vec3(0, -0.04f, 0), Vec3(0.75f, 0.06f, len * 0.5f + 0.02f), q);
                w.attachCollider(c, S.bridge);
                S.bridgeCols.push_back(c);
            }
        }
        // arqueiros de emboscada: ninho (-8,0,-70) e alcova da Linha Goblin (24,-3,-86)
        {
            MeshBuilder g; goblin(g, Vec3(0, 0, 0), 0.f, Pose::Crouch, 1.f, true);
            DynHandle h = w.addDynamic(Q("arqueiro_ninho"), std::move(g), Vec3(-7.7f, 0.05f, -69.6f)); h->visible = false; S.archers.push_back(h);
            MeshBuilder g2; goblin(g2, Vec3(0, 0, 0), 20.f, Pose::Crouch, 1.f, true);
            DynHandle h2 = w.addDynamic(Q("arqueiro_alcova"), std::move(g2), Vec3(24.5f, gy(24.5f, -87.2f, -3.1f), -87.2f)); h2->visible = false; S.archers.push_back(h2);
        }
        w.addEmitter(ParticlePreset::Steam, Vec3(0.f, -18.3f, -77.f), Vec3(26.f, 0.1f, 5.f), 6.f, Q("nevoa_do_rio"));
        w.addLight(Vec3(0.f, -17.f, -77.f), Vec3(0.25f, 0.45f, 0.7f), 1.2f, 22.f, LightAnim::Pulse, Q("reflexo_do_rio"))->animFreq = 0.2f;
    }

    // ================================================================ 1.5 CASCATA CONGELADA (gelo dinamico)
    {
        MeshBuilder im;
        im.box(Vec3(0, 3.3f, 0), Vec3(1.95f, 3.3f, 0.09f), QColor(176, 220, 244, 95), MAT_CLEAR_ICE);
        for (int i = 0; i < 9; ++i) {
            float x = -1.7f + i * 0.43f + (hrand(i, 61) - 0.5f) * 0.15f, r = 0.18f + 0.2f * hrand(i, 62);
            float top = 6.4f - hrand(i, 63) * 0.6f;
            im.cylinder(Vec3(x, 0.25f, 0.15f), Vec3(x, top, 0.1f), r * 1.35f, r, 8, QColor(226, 244, 252, 205), MAT_CLEAR_ICE);
            im.cylinder(Vec3(x, 0.8f + hrand(i, 64) * 2.2f, 0.28f), Vec3(x, 0.2f + hrand(i, 64) * 1.5f, 0.3f), r * 0.5f, 0.f, 6, QColor(214, 238, 252, 170), MAT_CLEAR_ICE);
        }
        im.ellipsoid(Vec3(0, 0.16f, 0.45f), Vec3(2.1f, 0.3f, 0.7f), QColor(200, 230, 250, 150), MAT_CLEAR_ICE, 10);
        S.iceSheet = w.addDynamic(Q("cascata_gelo"), std::move(im), Vec3(-12.f, -3.f, -44.15f));
        S.iceCol = w.addBoxCollider(Q("cascata_gelo"), Vec3(0, 3.3f, 0.1f), Vec3(2.05f, 3.3f, 0.3f), QQuaternion(), true);
        w.attachCollider(S.iceCol, S.iceSheet);
    }

    // ================================================================ 1.4 A TOCA
    {
        MeshBuilder mb;
        const float fy = -5.f;
        struct T { float x, z, yaw, r, h; QColor c; };
        const T tents[6] = {
            {-10.5f, -94.f, 30.f, 1.7f, 1.8f, QColor(152, 48, 42)},
            {-12.5f, -102.5f, -20.f, 1.6f, 1.7f, QColor(54, 98, 72)},
            {-6.5f, -107.f, 60.f, 1.5f, 1.6f, QColor(52, 80, 132)},
            {9.f, -93.f, -40.f, 1.8f, 1.9f, QColor(170, 128, 54)},
            {12.f, -101.f, 10.f, 1.6f, 1.7f, QColor(112, 76, 52)},
            {6.5f, -107.f, 150.f, 1.5f, 1.6f, QColor(190, 176, 142)}};
        for (int i = 0; i < 6; ++i) {
            Vec3 p = onFloor(tents[i].x, tents[i].z, fy);
            hideTent(mb, p, tents[i].yaw, tents[i].r, tents[i].h, tents[i].c);
            Vec3 fp = onFloor(tents[i].x + 1.5f * std::cos(tents[i].yaw * 0.0174f), tents[i].z + 1.5f * std::sin(tents[i].yaw * 0.0174f), fy);
            fur(mb, fp, tents[i].yaw + 90.f, 60 + i);
            bonesPile(mb, onFloor(tents[i].x - 1.2f, tents[i].z + 1.3f, fy), 0.28f, 70 + i);
        }
        // braseiro central roubado (0,-5,-98)
        Vec3 bc = onFloor(0.f, -98.f, fy);
        {
            MeshBuilder lit, dark;
            brazier(lit, Vec3(0, 0, 0), 1.35f, true); brazier(dark, Vec3(0, 0, 0), 1.35f, false);
            DynHandle a1 = w.addDynamic(Q("braseiro_toca"), std::move(lit), bc);
            DynHandle a2 = w.addDynamic(Q("braseiro_toca_apagado"), std::move(dark), bc); a2->visible = false;
            S.braziers.push_back({a1, a2});
            Light* l = w.addLight(bc + Vec3(0, 1.4f, 0), Vec3(1.0f, 0.52f, 0.17f), 5.2f, 15.f, LightAnim::TorchFlicker, Q("braseiro_central"));
            S.embers.push_back(l);
            w.addEmitter(ParticlePreset::Embers, bc + Vec3(0, 1.2f, 0), Vec3(0.3f, 0.05f, 0.3f), 11.f, Q("brasas_toca"));
        }
        S.embers.push_back(w.addLight(Vec3(-9.2f, -4.2f, -96.f), Vec3(1.0f, 0.46f, 0.14f), 1.5f, 7.f, LightAnim::TorchFlicker, Q("brasa_tenda_a")));
        S.embers.push_back(w.addLight(Vec3(10.f, -4.2f, -99.f), Vec3(1.0f, 0.46f, 0.14f), 1.5f, 7.f, LightAnim::TorchFlicker, Q("brasa_tenda_b")));
        // ao redor do braseiro: carvao, lenha, peles, ossos de rato
        coalPile(mb, bc + Vec3(-2.3f, 0, 1.6f), 0.7f, 8);
        logStack(mb, bc + Vec3(2.6f, 0, 1.2f), 70.f);
        sack(mb, bc + Vec3(-3.2f, 0, -1.f), 10.f, 1.0f); sack(mb, bc + Vec3(-3.7f, 0, -0.2f), 60.f, 0.9f);
        crate(mb, bc + Vec3(3.4f, 0, -1.9f), 30.f, 0.9f); crate(mb, bc + Vec3(4.4f, 0, -2.4f), 60.f, 0.7f);
        barrel(mb, bc + Vec3(-4.2f, 0, 2.6f), 0.f, 1.0f); barrel(mb, bc + Vec3(-5.1f, 0, 2.2f), 0.f, 0.9f);
        fur(mb, bc + Vec3(-1.6f, 0, -0.4f), 70.f, 81); fur(mb, bc + Vec3(1.7f, 0, -0.9f), -80.f, 82); fur(mb, bc + Vec3(0.2f, 0, 1.6f), 190.f, 83);
        blanket(mb, bc + Vec3(-1.9f, 0.04f, 1.2f), 40.f, 1.6f, 1.2f, 0.02f, QColor(168, 126, 52), QColor(110, 70, 40));
        blanket(mb, bc + Vec3(2.2f, 0.04f, 0.9f), -30.f, 1.5f, 1.1f, 0.02f, QColor(52, 94, 72), QColor(200, 190, 150));
        for (int i = 0; i < 9; ++i) {
            Vec3 p = onFloor(-8.f + hrand(i, 91) * 16.f, -92.f - hrand(i, 92) * 17.f, fy);
            bonesPile(mb, p, 0.18f + 0.1f * hrand(i, 93), 100 + i);
            skull(mb, p + Vec3(0.4f, 0, 0.2f), hrand(i, 94) * 360.f, 0.22f);
            mb.ellipsoid(p + Vec3(-0.3f, 0.03f, 0.3f), Vec3(0.12f, 0.02f, 0.07f), QColor(76, 66, 60), MAT_HIDE, 5);   // pele de rato
        }
        clayPot(mb, bc + Vec3(1.0f, 0, -2.2f), 0.9f); clayPot(mb, bc + Vec3(1.5f, 0, -2.1f), 0.7f);
        mb.cylinder(bc + Vec3(-1.f, 0.6f, 0.75f), bc + Vec3(-0.3f, 0.35f, 0.75f), 0.01f, 4, QColor(100, 74, 50), MAT_WOOD);   // espeto de rato assado
        mb.ellipsoid(bc + Vec3(-0.65f, 0.47f, 0.75f), Vec3(0.12f, 0.05f, 0.05f), QColor(120, 76, 48), MAT_HIDE, 6);
        // O CALICE DOURADO do templo, sobre uma pedra baixa, diante de Grukk (0,-5,-100)
        Vec3 cc = onFloor(0.f, -100.f, fy);
        mb.cylinder(cc, cc + Vec3(0, 0.18f, 0), 0.34f, 0.28f, 8, QColor(98, 92, 86), MAT_LIMESTONE);
        chalice(mb, cc + Vec3(0, 0.18f, 0));
        w.addLight(cc + Vec3(0, 0.7f, 0.4f), Vec3(1.0f, 0.82f, 0.4f), 1.3f, 4.f, LightAnim::None, Q("brilho_calice"));
        // GOBLINS (magros, encolhidos): ~10 adultos
        struct Gb { float x, z, yaw; Pose p; };
        const Gb gb[10] = {{-2.3f, -96.2f, 40.f, Pose::Huddle}, {2.2f, -96.6f, -35.f, Pose::Huddle}, {2.4f, -100.8f, 200.f, Pose::Sit},
                           {-2.6f, -100.f, 20.f, Pose::Crouch}, {-10.8f, -97.f, 80.f, Pose::Lie}, {9.f, -96.f, 190.f, Pose::Lie},
                           {7.4f, -103.4f, 120.f, Pose::Huddle}, {-7.8f, -101.f, 60.f, Pose::Crouch}, {13.2f, -104.f, 230.f, Pose::Sit},
                           {-4.4f, -105.8f, 100.f, Pose::Lie}};
        for (int i = 0; i < 10; ++i) goblin(mb, onFloor(gb[i].x, gb[i].z, fy), gb[i].yaw, gb[i].p, 0.95f + 0.1f * hrand(i, 1), false);
        // 3 filhotes juntinhos do braseiro e a anciã
        goblinChild(mb, onFloor(-0.9f, -97.1f, fy), 100.f, Pose::Huddle);
        goblinChild(mb, onFloor(0.7f, -96.9f, fy), -80.f, Pose::Huddle);
        goblinChild(mb, onFloor(-0.1f, -99.2f, fy), 0.f, Pose::Huddle);
        {
            Vec3 ap = onFloor(1.1f, -99.5f, fy);
            goblin(mb, ap, 250.f, Pose::Sit, 0.86f, false);
            mb.ellipsoid(ap + Vec3(0, 0.42f, 0), Vec3(0.3f, 0.3f, 0.28f), QColor(106, 110, 98), MAT_CLOTH, 8);   // xale roubado
            mb.ellipsoid(ap + Vec3(0.02f, 0.62f, 0.05f), Vec3(0.2f, 0.16f, 0.2f), QColor(120, 124, 110), MAT_CLOTH, 7);
        }
        // Grukk Dente-Quebrado: boss esqueletico em um trono de caixotes, olhando para a ponte (sul)
        {
            Vec3 gp = onFloor(0.f, -102.8f, fy);
            crate(mb, gp + Vec3(-0.5f, 0, -0.5f), 20.f, 0.9f); crate(mb, gp + Vec3(0.5f, 0, -0.6f), -15.f, 0.9f);
            mb.box(gp + Vec3(0, 0.5f, -0.9f), Vec3(0.9f, 0.5f, 0.1f), QColor(88, 62, 42), MAT_WOOD);
            mb.ellipsoid(gp + Vec3(0, 0.5f, -0.8f), Vec3(0.9f, 0.55f, 0.15f), QColor(66, 50, 40), MAT_HIDE, 8);
            goblin(mb, gp + Vec3(0, 0.9f, 0), 180.f, Pose::Sit, 1.32f, false);
            for (int i = 0; i < 4; ++i)
                mb.cylinder(gp + Vec3(-0.9f + i * 0.6f, 0.9f, -1.0f), gp + Vec3(-0.9f + i * 0.6f, 1.5f, -1.0f), 0.012f, 4, QColor(220, 210, 184), MAT_BONE);
            skull(mb, gp + Vec3(1.2f, 0, 0.2f), 140.f, 0.5f);
            skull(mb, gp + Vec3(-1.3f, 0, 0.1f), 30.f, 0.5f);
        }
        // equipamento da galeria dos arqueiros e provisoes
        {
            Vec3 g0 = onFloor(-15.f, -98.f, -1.f);
            clayArrowBundle(mb, g0 + Vec3(1.2f, 0, 1.6f));
            clayArrowBundle(mb, g0 + Vec3(0.8f, 0, -4.8f));
            crate(mb, g0 + Vec3(-1.5f, 0, -1.0f), 12.f, 0.8f);
            barrel(mb, g0 + Vec3(-1.8f, 0, 1.4f), 0.f, 0.9f);
            for (int i = 0; i < 5; ++i) clayPot(mb, g0 + Vec3(-1.5f + (i % 3) * 0.35f, 0, 3.3f + (i / 3) * 0.35f), 0.65f);
            fur(mb, g0 + Vec3(0.2f, 0, -7.5f), 90.f, 111); fur(mb, g0 + Vec3(0.4f, 0, 5.6f), 80.f, 112);
        }
        // oficina de flechas de barro perto da saida leste
        {
            Vec3 e0 = onFloor(14.5f, -97.f, fy);
            table(mb, e0, 90.f, Vec3(1.2f, 0.75f, 0.7f), false);
            for (int i = 0; i < 6; ++i) arrowClay(mb, e0 + Vec3(-0.3f + hrand(i, 120) * 0.6f, 0.78f, -0.2f + hrand(i, 121) * 0.4f), hrand(i, 122) * 180.f);
            for (int i = 0; i < 6; ++i) clayPot(mb, e0 + Vec3(1.3f, 0, -0.6f + i * 0.28f), 0.5f);
            mb.ellipsoid(e0 + Vec3(0.2f, 0.82f, 0.2f), Vec3(0.18f, 0.06f, 0.14f), QColor(130, 86, 58), MAT_LIMESTONE, 7);   // bola de argila
        }
        // arqueiros da galeria (dinamicos, ocultos ate o evento G)
        {
            const float gz[3] = {-92.f, -98.f, -104.f}, gx[3] = {-14.4f, -15.f, -14.2f};
            for (int i = 0; i < 3; ++i) {
                MeshBuilder g; goblin(g, Vec3(0, 0, 0), -90.f + (i - 1) * 12.f, Pose::Crouch, 1.f, true);
                DynHandle h = w.addDynamic(Q("arqueiro_galeria_%1").arg(i), std::move(g), Vec3(gx[i], gy(gx[i], gz[i], -1.f), gz[i]));
                h->visible = false; S.archers.push_back(h);
            }
        }
        w.addStatic(std::move(mb));
        w.addEmitter(ParticlePreset::Dust, Vec3(0.f, -2.f, -98.f), Vec3(14.f, 2.5f, 12.f), 6.f, Q("poeira_da_toca"));
    }

    // ================================================================ 1.7 LABIRINTO: lixao, despensa, galeria desabada, marcas
    {
        MeshBuilder mb;
        static const Vec3 PN[11] = {Vec3(0, -5, -112), Vec3(0, -6, -120), Vec3(-15, -7, -126), Vec3(-26, -7, -132), Vec3(12, -7, -127), Vec3(22, -8, -122),
                                    Vec3(-6, -8, -136), Vec3(-16, -9, -146), Vec3(6, -8, -140), Vec3(12, -10, -150), Vec3(18, -13, -157)};
        const int links[11][2] = {{0, 1}, {1, 2}, {2, 3}, {1, 4}, {4, 5}, {2, 6}, {6, 7}, {6, 8}, {4, 8}, {8, 9}, {9, 10}};
        for (int l = 0; l < 11; ++l) {   // seixos de leito de rio
            Vec3 a = PN[links[l][0]], b = PN[links[l][1]];
            for (int i = 0; i < 9; ++i) {
                float t = 0.08f + 0.84f * hrand(l, i, 1);
                Vec3 p = a + (b - a) * t + Vec3((hrand(l, i, 2) - 0.5f) * 3.4f, 0, (hrand(l, i, 3) - 0.5f) * 3.4f);
                float yy = groundY(w, p.x(), p.z(), p.y(), 2.f);
                if (w.solidDistance(Vec3(p.x(), yy + 0.3f, p.z())) < 0.15f) continue;
                float s = 0.07f + hrand(l, i, 4) * 0.16f;
                int g = 104 + (int)(hrand(l, i, 5) * 40);
                mb.ellipsoid(Vec3(p.x(), yy + s * 0.35f, p.z()), Vec3(s * 1.3f, s * 0.7f, s), QColor(g, g - 6, g - 14), MAT_WET_LIMESTONE, 6);
            }
        }
        // O Lixao (N3)
        {
            Vec3 c = onFloor(-26.f, -132.f, -7.f);
            for (int i = 0; i < 6; ++i) {
                Vec3 p = c + Vec3((hrand(i, 131) - 0.5f) * 7.f, 0, (hrand(i, 132) - 0.5f) * 6.f);
                p.setY(gy(p.x(), p.z(), -7.f));
                bonesPile(mb, p, 0.9f + hrand(i, 133) * 0.8f, 140 + i);
                skull(mb, p + Vec3(0.5f, 0.3f, 0.2f), hrand(i, 134) * 360.f, 0.3f);
                clayPot(mb, p + Vec3(-0.7f, 0, 0.8f), 0.8f);
                mb.box(p + Vec3(0.4f, 0.04f, -0.8f), Vec3(0.4f, 0.04f, 0.28f), QColor(70 + i * 8, 56, 50), MAT_CLOTH);   // trapos
                mb.box(p + Vec3(-0.2f, 0.07f, 0.6f), Vec3(0.3f, 0.06f, 0.2f), QColor(88, 70, 50), MAT_ROTTEN_WOOD);
            }
            for (int i = 0; i < 12; ++i)
                mb.ellipsoid(c + Vec3((hrand(i, 135) - 0.5f) * 8.f, 0.04f, (hrand(i, 136) - 0.5f) * 7.f), Vec3(0.12f, 0.04f, 0.09f), QColor(96, 84, 52), MAT_HIDE, 5);
            sack(mb, c + Vec3(2.2f, 0, 2.f), 0.f, 1.0f);
            barrel(mb, c + Vec3(-2.6f, 0, 2.2f), 0.f, 0.9f);
            crate(mb, c + Vec3(3.1f, 0, -1.2f), 30.f, 0.6f);
        }
        // Despensa Congelada (N5)
        {
            Vec3 c = onFloor(22.f, -122.f, -8.f);
            frozenBear(mb, c + Vec3(0.5f, 0, 1.0f), 20.f);
            frozenGoat(mb, c + Vec3(-2.6f, 0, -1.2f), 70.f);
            frozenGoat(mb, c + Vec3(-1.4f, 0, 2.4f), -30.f);
            frozenGoat(mb, c + Vec3(3.1f, 0, -0.6f), 160.f);
            for (int i = 0; i < 3; ++i) {   // cabras penduradas em ganchos
                Vec3 hp = c + Vec3(-1.5f + i * 1.8f, 2.4f, -2.4f);
                mb.tube({hp + Vec3(0, 0.6f, 0), hp}, 0.015f, 4, QColor(90, 70, 50), MAT_ROPE);
                mb.push(); mb.translate(hp + Vec3(0, -1.0f, 0)); mb.rotate(-90.f, Vec3(0, 0, 1)); mb.scale(0.9f);
                frozenGoat(mb, Vec3(0.45f, 0, 0), 0.f);
                mb.pop();
            }
            Prefabs::iceFringe(mb, c + Vec3(-4.f, 2.7f, -3.f), c + Vec3(4.f, 2.7f, 3.f), 22, 0.9f, 15);
            for (int i = 0; i < 10; ++i) mb.ellipsoid(c + Vec3((hrand(i, 141) - 0.5f) * 8.f, 0.02f, (hrand(i, 142) - 0.5f) * 6.f), Vec3(0.3f, 0.02f, 0.22f), QColor(210, 232, 248, 150), MAT_CLEAR_ICE, 6);
            crate(mb, c + Vec3(-4.f, 0, 1.5f), 10.f, 0.8f);
        }
        // Galeria Desabada (N7)
        {
            Vec3 c = onFloor(-16.f, -146.f, -9.f);
            for (int i = 0; i < 9; ++i) {
                Vec3 p = c + Vec3((hrand(i, 151) - 0.5f) * 7.f, 0, 1.f + hrand(i, 152) * 4.f);
                p.setY(gy(p.x(), p.z(), -9.f));
                float s = 0.25f + hrand(i, 153) * 0.5f;
                boulder(mb, p + Vec3(0, s * 0.5f, 0), s, s * 0.7f, s * 0.9f, 160 + i);
            }
            for (int i = 0; i < 3; ++i)
                mb.cylinder(c + Vec3(-1.5f + i * 1.7f, 0.5f + i * 0.3f, 3.f + i * 0.5f), c + Vec3(-0.5f + i * 1.7f, 2.2f - i * 0.5f, 5.f + i * 0.4f), 0.11f, 7, QColor(86, 62, 42), MAT_ROTTEN_WOOD);
            mb.cylinder(c + Vec3(-1.1f, 0.6f, 5.0f), c + Vec3(-1.3f, 0.75f, 6.0f), 0.045f, 0.035f, 5, QColor(112, 150, 74), MAT_SKIN);   // braco goblin sob pedras
            bonesPile(mb, c + Vec3(1.6f, 0, 1.2f), 0.5f, 171);
        }
        // marcas nas paredes: espirais de ocre nos cruzamentos; X de aviso nos becos
        const Vec3 spots[6] = {PN[1], PN[2], PN[4], PN[6], PN[8], PN[9]};
        for (int i = 0; i < 6; ++i) {
            float yy = groundY(w, spots[i].x(), spots[i].z(), spots[i].y(), 2.f) + 1.7f;
            wallSpiral(mb, Vec3(spots[i].x(), yy, spots[i].z()), Vec3(i % 2 ? 1.f : -1.f, 0, 0.3f), 0.38f, 2.4f);
        }
        const Vec3 deadends[3] = {Vec3(-26, -7, -132), Vec3(22, -8, -122), Vec3(-16, -9, -146)};
        for (int i = 0; i < 3; ++i) {
            float yy = groundY(w, deadends[i].x(), deadends[i].z(), deadends[i].y(), 2.f) + 1.5f;
            wallScratches(mb, Vec3(deadends[i].x(), yy, deadends[i].z()), Vec3(i == 0 ? -1.f : (i == 1 ? 1.f : 0.f), 0, i == 2 ? -1.f : 0.2f), 200 + i);
        }
        w.addStatic(std::move(mb));
        w.addEmitter(ParticlePreset::Dust, Vec3(-16.f, -6.5f, -148.f), Vec3(4.f, 2.f, 4.f), 5.f, Q("poeira_galeria"));

        // pedras soltas que escondem o Esconderijo do Chefe (dinamico: o jogador pode afasta-las)
        {
            const Vec3 hp(-30.6f, -7.05f, -134.1f);
            MeshBuilder rm;
            for (int i = 0; i < 9; ++i) {
                float a = i * 0.9f;
                float s = 0.42f + hrand(i, 181) * 0.4f;
                float k = (i % 3 == 0) ? 0.3f : 1.f;
                rm.ellipsoid(Vec3(std::cos(a) * 0.7f * k, 0.35f + (i / 3) * 0.7f, std::sin(a) * 0.55f * k), Vec3(s, s * 0.85f, s * 0.9f),
                             QColor(104 + (int)(hrand(i, 182) * 30), 98, 90), MAT_LIMESTONE, 8);
            }
            S.hideoutRocks = w.addDynamic(Q("pedras_esconderijo"), std::move(rm), hp);
            S.hideoutCol = w.addCylinderCollider(Q("pedras_esconderijo"), Vec3(0, 1.3f, 0), 1.9f, 1.3f, true);
            w.attachCollider(S.hideoutCol, S.hideoutRocks);
        }
        // conteudo do esconderijo: bau com o resto dos tesouros do templo + 120 po
        {
            MeshBuilder hm;
            Vec3 c = onFloor(-34.f, -136.5f, -7.1f);
            chest(hm, c + Vec3(0.2f, 0, 0.2f), 40.f, true, 1.0f);
            for (int i = 0; i < 40; ++i) {
                float x = -1.5f + hrand(i, 191) * 1.2f, z = -1.f + hrand(i, 192) * 0.9f, y = 0.03f + (i / 14) * 0.03f;
                hm.cylinder(c + Vec3(x, y, z), c + Vec3(x, y + 0.015f, z), 0.032f, 8, QColor(238, 196, 80), MAT_GOLD);
            }
            chalice(hm, c + Vec3(-1.2f, 0, 1.4f));
            hm.cylinder(c + Vec3(1.3f, 0, -1.2f), c + Vec3(1.3f, 0.6f, -1.2f), 0.04f, 0.035f, 6, QColor(236, 190, 70), MAT_GOLD);   // candelabro
            hm.cylinder(c + Vec3(1.65f, 0, -1.0f), c + Vec3(1.65f, 0.5f, -1.0f), 0.035f, 6, QColor(236, 190, 70), MAT_GOLD);
            hm.torus(c + Vec3(1.f, 0.07f, 1.1f), Vec3(0, 1, 0), 0.22f, 0.025f, QColor(150, 105, 55), MAT_BRONZE, 12, 5);   // incensario
            hm.box(c + Vec3(-0.4f, 0.12f, -1.5f), Vec3(0.3f, 0.12f, 0.18f), QColor(122, 80, 46), MAT_WOOD);               // relicario menor
            hm.box(c + Vec3(-0.4f, 0.26f, -1.5f), Vec3(0.32f, 0.02f, 0.2f), QColor(238, 196, 80), MAT_GOLD);
            blanket(hm, c + Vec3(1.5f, 0, 1.6f), 25.f, 1.1f, 0.8f, 0.2f, QColor(150, 46, 40), QColor(210, 190, 150));
            w.addStatic(std::move(hm));
            w.addLight(c + Vec3(0.f, 1.2f, 0.f), Vec3(1.0f, 0.8f, 0.4f), 0.9f, 4.f, LightAnim::None, Q("brilho_tesouro"));
        }
    }

    // ================================================================ 1.8 FRONTEIRA DOS OSSOS
    {
        MeshBuilder mb;
        Vec3 a(18, -13, -157);
        Vec3 d2 = Vec3(0.62f, 0, -0.78f).normalized();
        Vec3 lat(-d2.z(), 0, d2.x());
        auto along = [&](float s, float off) { return a + d2 * s + lat * off; };
        auto floorAt = [&](float s) { return -13.f - s * 0.42f; };
        const float ts[6] = {0.5f, 2.2f, 4.4f, 6.2f, 8.5f, 10.4f};
        for (int i = 0; i < 6; ++i) {   // totens de ossos dos dois lados
            Vec3 p = along(ts[i], (i % 2 ? 1.5f : -1.5f));
            p.setY(groundY(w, p.x(), p.z(), floorAt(ts[i]), 3.f));
            totemOfBones(mb, p, 2.0f + hrand(i, 211) * 0.5f, 220 + i);
            bonesPile(mb, p + Vec3(0.4f, 0, 0.3f), 0.5f, 230 + i);
        }
        for (int i = 0; i < 14; ++i) {   // fetiches pendurados no teto
            float s = 0.8f + i * 0.85f;
            Vec3 p = along(s, (hrand(i, 241) - 0.5f) * 2.4f);
            float yy = groundY(w, p.x(), p.z(), floorAt(s), 2.f);
            RayHit h = w.raycast(Vec3(p.x(), yy + 1.f, p.z()), Vec3(0, 1, 0), 6.f, false);
            if (!h.hit) continue;
            fetish(mb, h.pos + Vec3(0, 0.05f, 0), 0.7f + hrand(i, 242) * 0.7f, 250 + i);
        }
        {   // linha de cinzas e sal no chao em (21,chao,-160), atravessando o tunel
            Vec3 c(21.f, 0, -160.f);
            float yy = groundY(w, c.x(), c.z(), -15.f, 3.f);
            float yawDeg = std::atan2(lat.x(), lat.z()) * 57.29578f;
            mb.push(); mb.translate(Vec3(c.x(), yy + 0.012f, c.z())); mb.rotateY(yawDeg);
            mb.box(Vec3(0, 0, 0), Vec3(0.11f, 0.012f, 2.5f), QColor(232, 230, 222), MAT_CLOTH);
            mb.box(Vec3(0.22f, 0, 0), Vec3(0.07f, 0.01f, 2.4f), QColor(70, 66, 64), MAT_CLOTH);
            mb.pop();
            for (int i = 0; i < 26; ++i) {
                float t = (i / 25.f - 0.5f) * 5.f;
                Vec3 p = c + lat * t + d2 * ((hrand(i, 261) - 0.5f) * 0.4f);
                float py = groundY(w, p.x(), p.z(), yy, 1.f);
                mb.box(Vec3(p.x(), py + 0.02f, p.z()), Vec3(0.035f, 0.02f, 0.035f), (i % 3) ? QColor(236, 234, 226) : QColor(80, 76, 72), MAT_CLOTH);
            }
        }
        for (int i = 0; i < 6; ++i) {   // arranhoes de protecao nas paredes
            float s = 1.5f + i * 1.9f;
            Vec3 p = along(s, 0);
            float yy = groundY(w, p.x(), p.z(), floorAt(s), 2.f) + 1.4f;
            Vec3 dd = (i % 2) ? lat : -lat;
            wallScratches(mb, Vec3(p.x(), yy, p.z()), dd, 300 + i);
        }
        w.addStatic(std::move(mb));
    }
}

} // namespace L1
