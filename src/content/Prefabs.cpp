// Implementacao dos prefabs (props e criaturas low-poly). Ver Prefabs.h.
#include "content/Prefabs.h"
#include <QQuaternion>
#include <algorithm>
#include <cmath>
#include <random>

namespace Prefabs {
namespace {

using Rng = std::mt19937;
const float kPi = 3.14159265f;
inline Vec3 V(float x, float y, float z) { return Vec3(x, y, z); }

// aleatorio deterministico em [a,b]
float rf(Rng& r, float a, float b) { return a + (b - a) * ((r() & 0xFFFFFF) / 16777216.f); }

int clampc(float v) { return std::max(0, std::min(255, (int)std::lround(v))); }
QColor shade(const QColor& c, float f) { return QColor(clampc(c.red() * f), clampc(c.green() * f), clampc(c.blue() * f), c.alpha()); }
QColor vary(const QColor& c, Rng& r, float a = 0.1f) { return shade(c, 1.f + rf(r, -a, a)); }

unsigned seedOf(const Vec3& p) {
    return (unsigned)(std::lround(p.x() * 37.f) * 73856093 ^ std::lround(p.y() * 37.f) * 19349663 ^ std::lround(p.z() * 37.f) * 83492791);
}

// translate + yaw durante o escopo
struct Placed {
    MeshBuilder& mb;
    Placed(MeshBuilder& m, const Vec3& pos, float yaw) : mb(m) { mb.push(); mb.translate(pos); if (yaw != 0.f) mb.rotateY(yaw); }
    ~Placed() { mb.pop(); }
};

// gira o referencial para que +Y local aponte em 'dir'
void orientY(MeshBuilder& mb, const Vec3& dir) {
    if (dir.lengthSquared() < 1e-10f) return;
    mb.rotate(QQuaternion::rotationTo(Vec3(0, 1, 0), dir.normalized()));
}

// referencial com +Z ao longo de d e +X horizontal (Y aproximadamente para cima)
QQuaternion frameAlong(const Vec3& d) {
    Vec3 z = d.normalized();
    Vec3 x(z.z(), 0, -z.x());
    if (x.lengthSquared() < 1e-6f) x = Vec3(1, 0, 0);
    x.normalize();
    Vec3 y = Vec3::crossProduct(z, x);
    return QQuaternion::fromAxes(x, y, z);
}

// caveira local: base em y=0, olhando para -Z
void skullLocal(MeshBuilder& mb, const QColor& bone) {
    QColor b2 = shade(bone, 0.9f), dark(25, 20, 18);
    mb.ellipsoid(V(0, 0.105f, 0), V(0.075f, 0.07f, 0.082f), bone, MAT_BONE, 8);
    mb.box(V(0, 0.04f, -0.05f), V(0.042f, 0.022f, 0.04f), b2, MAT_BONE);
    for (int s = -1; s <= 1; s += 2) {
        mb.sphere(V(s * 0.032f, 0.108f, -0.066f), 0.02f, dark, MAT_COAL, 6);
        mb.box(V(s * 0.015f, 0.022f, -0.092f), V(0.008f, 0.012f, 0.006f), shade(bone, 1.1f), MAT_BONE);
    }
    mb.triangle(V(0, 0.09f, -0.078f), V(-0.012f, 0.066f, -0.082f), V(0.012f, 0.066f, -0.082f), dark, MAT_COAL);
}

// osso: cilindro com bolas nas pontas
void boneBetween(MeshBuilder& mb, const Vec3& a, const Vec3& b, float r, const QColor& c) {
    mb.cylinder(a, b, r, r, 6, c, MAT_BONE);
    mb.sphere(a, r * 1.6f, c, MAT_BONE, 6);
    mb.sphere(b, r * 1.6f, c, MAT_BONE, 6);
}

// ====================== figura humanoide / goblin ======================
struct Fig {
    float s = 1, bx = 1, bz = 1, limbR = 1, headR = 1, hunch = 0, eyeR = 0.02f;
    int kind = 0;   // 0 humano, 1 goblin
    QColor skin, torso, legs, hair, boots, eye, loin;
    bool cape = false, hood = false, glow = false, axe = false, bow = false;
};

struct Rig {
    float hipY, hipZ, lean, headTilt, kneeY, kneeZ, footY, footZ, kneeX;
    int arms;
};

Rig rigFor(Pose p, const Fig& F) {
    switch (p) {
    case Pose::Crouch: return {0.5f, 0.1f, 35.f + F.hunch * 0.5f, 0.f, 0.05f, -0.4f, -0.44f, -0.25f, 0.13f, 1};
    case Pose::Huddle: return {0.3f, 0.25f, 55.f, 20.f, 0.35f, -0.4f, -0.24f, -0.35f, 0.12f, 2};
    case Pose::Lie:    return {0.15f, 0.3f, -90.f, 0.f, -0.03f, -0.45f, -0.06f, -0.9f, 0.1f, 0};
    case Pose::Hang:   return {1.15f, 0.f, 12.f, 30.f, -0.4f, -0.08f, -0.77f, 0.05f, 0.1f, 3};
    case Pose::Sit:    return {0.48f, 0.1f, 5.f + F.hunch * 0.5f, 0.f, 0.f, -0.45f, -0.42f, -0.47f, 0.12f, 4};
    case Pose::Stand:
    default:           return {0.92f, 0.f, F.hunch, 0.f, -0.44f, 0.f, -0.86f, 0.f, 0.1f, 0};
    }
}

void drawHead(MeshBuilder& mb, const Fig& F) {
    const QColor skin = F.skin;
    mb.ellipsoid(V(0, 0, 0), V(0.095f, 0.115f, 0.105f), skin, MAT_SKIN, 8);
    if (F.kind == 1) {
        // nariz longo e curvado, orelhas grandes e pontudas, presas
        mb.cylinder(V(0, -0.01f, -0.09f), V(0, -0.065f, -0.26f), 0.032f, 0.016f, 6, shade(skin, 0.95f), MAT_SKIN);
        mb.sphere(V(0, -0.066f, -0.265f), 0.02f, shade(skin, 0.8f), MAT_SKIN, 6);
        for (int s = -1; s <= 1; s += 2) {
            Vec3 b0(s * 0.09f, 0.07f, 0.0f), b1(s * 0.09f, -0.03f, 0.03f), tip(s * 0.36f, 0.14f, 0.07f);
            mb.triangle(b0, b1, tip, shade(skin, 0.95f), MAT_SKIN);
            mb.triangle(b0 + V(s * 0.012f, 0, 0.004f), b1 + V(s * 0.012f, 0.012f, 0.004f), tip + V(-s * 0.03f, -0.002f, 0.004f), QColor(190, 120, 110), MAT_SKIN);
            mb.cone(V(s * 0.03f, -0.07f, -0.09f), V(s * 0.03f, -0.025f, -0.1f), 0.014f, 5, QColor(235, 225, 190), MAT_BONE);
        }
        mb.box(V(0, -0.075f, -0.09f), V(0.04f, 0.008f, 0.01f), QColor(60, 25, 25), MAT_FLESH);
        // sobrancelha pesada
        mb.box(V(0, 0.055f, -0.09f), V(0.085f, 0.014f, 0.016f), shade(skin, 0.75f), MAT_SKIN);
    } else {
        mb.box(V(0, -0.012f, -0.108f), V(0.015f, 0.028f, 0.022f), shade(skin, 0.95f), MAT_SKIN);
        for (int s = -1; s <= 1; s += 2) mb.ellipsoid(V(s * 0.097f, 0, 0.0f), V(0.012f, 0.03f, 0.022f), skin, MAT_SKIN, 5);
        mb.box(V(0, -0.065f, -0.092f), V(0.03f, 0.006f, 0.01f), QColor(120, 70, 65), MAT_SKIN);
    }
    // olhos
    for (int s = -1; s <= 1; s += 2) {
        Vec3 ep(s * 0.04f, 0.022f, F.kind == 1 ? -0.088f : -0.094f);
        if (F.glow) mb.sphere(ep, F.eyeR, F.eye, MAT_EMISSIVE, 6);
        else mb.sphere(ep, F.eyeR * 0.8f, QColor(25, 20, 20), MAT_COAL, 5);
    }
    if (F.hood) {
        mb.ellipsoid(V(0, 0.02f, 0.06f), V(0.135f, 0.14f, 0.135f), F.torso, MAT_CLOTH, 8);
        mb.cone(V(0, 0.08f, 0.14f), V(0, -0.02f, 0.3f), 0.07f, 6, shade(F.torso, 0.9f), MAT_CLOTH);
        mb.box(V(0, 0.115f, -0.07f), V(0.1f, 0.012f, 0.03f), shade(F.torso, 0.8f), MAT_CLOTH);
    } else if (F.kind == 0) {
        mb.ellipsoid(V(0, 0.04f, 0.028f), V(0.1f, 0.095f, 0.098f), F.hair, MAT_HIDE, 8);
    }
}

void drawFigure(MeshBuilder& mb, const Fig& F, Pose pose) {
    const Rig R = rigFor(pose, F);
    const float lr = F.limbR;
    mb.push();
    mb.scale(V(F.s * F.bx, F.s, F.s * F.bz));
    mb.translate(0, R.hipY, R.hipZ);   // espaco do quadril

    const float la = R.lean * kPi / 180.f;
    const Vec3 u(0, std::cos(la), -std::sin(la)), f(0, -std::sin(la), -std::cos(la));
    const QColor handC = F.skin;
    const QColor upperArm = F.kind == 1 ? shade(F.skin, 0.95f) : F.torso;

    Vec3 handR, handL;
    // pernas
    for (int s = -1; s <= 1; s += 2) {
        Vec3 hj(s * 0.1f, 0, 0), kn(s * R.kneeX, R.kneeY, R.kneeZ), ft(s * 0.1f, R.footY, R.footZ);
        mb.capsule(hj, kn, 0.085f * lr, F.legs, MAT_CLOTH, 6);
        mb.capsule(kn, ft, 0.065f * lr, shade(F.legs, 0.92f), MAT_CLOTH, 6);
        bool barefoot = F.kind == 1;
        mb.box(ft + V(0, -0.02f, -0.07f), V(0.05f * lr + 0.01f, 0.04f, 0.11f), barefoot ? shade(F.skin, 0.9f) : F.boots, barefoot ? MAT_SKIN : MAT_HIDE);
        if (barefoot)
            for (int k = -1; k <= 1; ++k) mb.cone(ft + V(k * 0.03f, -0.03f, -0.17f), ft + V(k * 0.035f, -0.045f, -0.23f), 0.012f, 4, QColor(60, 50, 30), MAT_BONE);
    }
    // braco (modo depende da pose)
    for (int s = -1; s <= 1; s += 2) {
        Vec3 sh = u * 0.52f + V(s * 0.22f, 0, 0), el, hd;
        switch (R.arms) {
        case 1: el = V(s * 0.27f, 0.12f, -0.28f); hd = V(s * 0.16f, 0.0f, -0.42f); break;
        case 2: el = V(s * 0.28f, 0.18f, -0.3f);  hd = V(s * 0.12f, 0.3f, -0.45f); break;
        case 3: el = sh + V(s * 0.14f, 0.25f, -0.03f); hd = sh + V(s * 0.1f, 0.5f, -0.03f); break;
        case 4: el = V(s * 0.27f, 0.1f, -0.2f);   hd = V(s * 0.13f, 0.02f, -0.4f); break;
        default:
            el = sh + V(s * 0.06f, 0, 0) - u * 0.27f;
            hd = sh + V(s * 0.07f, 0, 0) - u * 0.52f + f * 0.04f; break;
        }
        mb.capsule(sh, el, 0.058f * lr, upperArm, F.kind == 1 ? MAT_SKIN : MAT_CLOTH, 6);
        mb.capsule(el, hd, 0.048f * lr, handC, MAT_SKIN, 6);
        mb.sphere(hd, 0.055f * lr, handC, MAT_SKIN, 6);
        if (s > 0) handR = hd; else handL = hd;
        if (R.arms == 3) {   // algemas e correntes para cima
            mb.torus(hd + V(0, -0.03f, 0), V(0, 1, 0), 0.06f, 0.014f, QColor(70, 66, 62), MAT_RUSTY_IRON, 8, 4);
            mb.cylinder(hd, hd + V(0, 0.55f, 0), 0.012f, 4, QColor(80, 75, 70), MAT_RUSTY_IRON);
        }
    }

    // tronco, cabeca, capa (referencial inclinado)
    mb.push();
    mb.rotate(-R.lean, V(1, 0, 0));
    mb.cylinder(V(0, 0, 0), V(0, 0.52f, 0), 0.14f, 0.18f, 8, F.torso, MAT_CLOTH);
    mb.box(V(0, 0.5f, 0), V(0.22f, 0.045f, 0.1f), shade(F.torso, 1.08f), MAT_CLOTH);
    mb.cylinder(V(0, -0.02f, 0), V(0, 0.07f, 0), 0.158f, 0.16f, 8, F.boots, MAT_HIDE);
    mb.box(V(0, 0.035f, -0.155f), V(0.03f, 0.03f, 0.008f), QColor(180, 150, 70), MAT_BRONZE);
    mb.cylinder(V(0, 0.52f, 0), V(0, 0.62f, 0), 0.05f, 6, F.skin, MAT_SKIN);
    if (F.kind == 1) {   // faixa de couro cruzada e tanga esfarrapada
        mb.push(); mb.translate(0, 0.28f, -0.02f); mb.rotate(35, V(0, 0, 1));
        mb.box(V(0, 0, -0.1f), V(0.03f, 0.3f, 0.02f), QColor(85, 60, 40), MAT_HIDE);
        mb.pop();
        mb.box(V(0, -0.14f, -0.1f), V(0.11f, 0.13f, 0.012f), F.loin, MAT_CLOTH);
        mb.triangle(V(-0.11f, -0.27f, -0.11f), V(-0.03f, -0.27f, -0.11f), V(-0.07f, -0.37f, -0.112f), F.loin, MAT_CLOTH);
        mb.triangle(V(0.0f, -0.27f, -0.11f), V(0.11f, -0.27f, -0.11f), V(0.06f, -0.4f, -0.112f), shade(F.loin, 0.85f), MAT_CLOTH);
        mb.box(V(0, -0.14f, 0.1f), V(0.1f, 0.1f, 0.012f), shade(F.loin, 0.8f), MAT_CLOTH);
    }
    if (F.cape) {
        mb.box(V(0, 0.5f, 0.02f), V(0.26f, 0.06f, 0.13f), shade(F.legs, 1.1f), MAT_CLOTH);
        mb.push(); mb.translate(0, 0.52f, 0.13f); mb.rotate(-8, V(1, 0, 0));
        mb.box(V(0, -0.5f, 0.015f), V(0.24f, 0.5f, 0.015f), shade(F.legs, 0.95f), MAT_CLOTH);
        mb.pop();
    }
    mb.push();
    mb.translate(0, 0.62f, 0);
    mb.rotate(-R.headTilt, V(1, 0, 0));
    mb.translate(0, 0.1f * F.headR, 0);
    mb.scale(F.headR);
    drawHead(mb, F);
    mb.pop();
    mb.pop();

    // armas
    if (pose != Pose::Lie && pose != Pose::Hang) {
        if (F.axe) {
            Vec3 a = handR + V(0, -0.3f, 0), b = handR + V(0, 0.6f, 0);
            mb.cylinder(a, b, 0.024f, 0.022f, 6, QColor(105, 75, 48), MAT_WOOD);
            mb.box(b + V(0, -0.08f, -0.1f), V(0.012f, 0.1f, 0.1f), QColor(150, 152, 160), MAT_IRON);
            mb.box(b + V(0, -0.08f, 0.03f), V(0.03f, 0.05f, 0.03f), QColor(120, 122, 130), MAT_IRON);
            mb.cone(b + V(0, 0, 0.0f), b + V(0, 0.1f, 0), 0.025f, 5, QColor(130, 132, 140), MAT_IRON);
        }
        if (F.bow) {
            Vec3 h = handL;
            std::vector<Vec3> pts = {h + V(0, -0.65f, 0.0f), h + V(0, -0.33f, -0.11f), h + V(0, 0, -0.15f), h + V(0, 0.33f, -0.11f), h + V(0, 0.65f, 0)};
            mb.tube(pts, std::vector<float>{0.014f, 0.022f, 0.026f, 0.022f, 0.014f}, 5, QColor(110, 78, 50), MAT_WOOD);
            mb.cylinder(pts.front(), pts.back(), 0.006f, 4, QColor(200, 190, 160), MAT_ROPE);
        }
    }
    mb.pop();
}

Fig humanFig(const HumanoidParams& p) {
    Fig F;
    F.s = p.height / 1.75f;
    F.bx = p.build;
    F.bz = 1.f + (p.build - 1.f) * 0.6f;
    F.skin = p.skin; F.torso = p.cloth; F.legs = p.cloth2; F.hair = p.hair; F.boots = shade(p.cloth2, 0.7f);
    F.cape = p.cape; F.hood = p.hood; F.glow = p.glowingEyes; F.eye = p.eyeColor; F.axe = p.axe;
    F.eyeR = 0.022f;
    return F;
}

} // namespace

// ----------------------------------------------------------------------
void humanoid(MeshBuilder& mb, const Vec3& pos, float yawDeg, const HumanoidParams& p) {
    Placed pl(mb, pos, yawDeg);
    drawFigure(mb, humanFig(p), p.pose);
}

void goblin(MeshBuilder& mb, const Vec3& pos, float yawDeg, Pose pose, float scale, bool withBow) {
    Rng r(seedOf(pos) + 11u);
    Fig F;
    F.s = 0.52f * scale; F.bx = 0.85f; F.bz = 0.8f; F.limbR = 0.65f; F.headR = 1.55f; F.hunch = 16.f; F.kind = 1;
    F.skin = vary(QColor(105, 146, 70), r, 0.12f);
    F.torso = shade(F.skin, 0.92f); F.legs = shade(F.skin, 0.88f); F.hair = F.skin; F.boots = F.skin;
    F.loin = vary(QColor(96, 70, 48), r, 0.15f);
    F.glow = true; F.eye = QColor(225, 255, 50); F.eyeR = 0.03f; F.bow = withBow;
    Placed pl(mb, pos, yawDeg);
    drawFigure(mb, F, pose);
}

void goblinChild(MeshBuilder& mb, const Vec3& pos, float yawDeg, Pose pose) {
    Rng r(seedOf(pos) + 23u);
    Fig F;
    F.s = 0.37f; F.bx = 0.85f; F.bz = 0.8f; F.limbR = 0.7f; F.headR = 1.75f; F.hunch = 20.f; F.kind = 1;
    F.skin = vary(QColor(125, 160, 88), r, 0.1f);
    F.torso = shade(F.skin, 0.93f); F.legs = shade(F.skin, 0.9f); F.hair = F.skin; F.boots = F.skin;
    F.loin = vary(QColor(110, 90, 70), r, 0.12f);
    F.glow = true; F.eye = QColor(235, 255, 80); F.eyeR = 0.034f;
    Placed pl(mb, pos, yawDeg);
    drawFigure(mb, F, pose);
}

// ---------------------------- lobo ----------------------------
namespace {
void wolfLeg3(MeshBuilder& mb, const Vec3& top, const Vec3& mid, const Vec3& paw, float r, const QColor& c) {
    mb.capsule(top, mid, r, c, MAT_HIDE, 6);
    mb.capsule(mid, paw + V(0, 0.03f, 0), r * 0.7f, shade(c, 0.9f), MAT_HIDE, 6);
    mb.box(paw + V(0, -0.015f, -0.04f), V(r * 0.9f, 0.025f, 0.065f), shade(c, 0.75f), MAT_HIDE);
}
} // namespace

void wolf(MeshBuilder& mb, const Vec3& pos, float yawDeg, bool hell, Pose pose, float scale) {
    Rng r(seedOf(pos) + (hell ? 77u : 5u));
    const QColor fur = vary(hell ? QColor(50, 38, 38) : QColor(124, 120, 116), r, 0.06f);
    const QColor furD = shade(fur, 0.72f), belly = hell ? QColor(36, 28, 28) : QColor(172, 166, 154);
    const QColor eyeC = hell ? QColor(90, 175, 255) : QColor(255, 190, 40);
    Placed pl(mb, pos, yawDeg);
    mb.scale(scale);

    float drop = 0.f; bool sit = false;
    switch (pose) {
    case Pose::Crouch: drop = 0.22f; break;
    case Pose::Huddle: drop = 0.36f; break;
    case Pose::Sit: sit = true; break;
    case Pose::Lie:
        mb.translate(0, 0.3f, 0); mb.rotate(90, V(0, 0, 1)); mb.translate(0, -0.6f, 0); break;
    default: break;
    }

    // pernas
    if (sit) {
        for (int s = -1; s <= 1; s += 2) {
            mb.capsule(V(s * 0.13f, 0.8f, -0.25f), V(s * 0.12f, 0.4f, -0.27f), 0.065f, fur, MAT_HIDE, 6);
            mb.capsule(V(s * 0.12f, 0.4f, -0.27f), V(s * 0.12f, 0.06f, -0.3f), 0.045f, shade(fur, 0.9f), MAT_HIDE, 6);
            mb.box(V(s * 0.12f, 0.025f, -0.34f), V(0.05f, 0.025f, 0.07f), furD, MAT_HIDE);
            mb.ellipsoid(V(s * 0.17f, 0.22f, 0.4f), V(0.12f, 0.2f, 0.2f), fur, MAT_HIDE, 7);
            mb.capsule(V(s * 0.15f, 0.12f, 0.3f), V(s * 0.14f, 0.05f, 0.08f), 0.04f, shade(fur, 0.9f), MAT_HIDE, 6);
            mb.box(V(s * 0.14f, 0.025f, 0.02f), V(0.045f, 0.025f, 0.07f), furD, MAT_HIDE);
        }
    } else {
        for (int s = -1; s <= 1; s += 2) {
            wolfLeg3(mb, V(s * 0.13f, 0.56f - drop, -0.42f), V(s * 0.14f, 0.3f - drop * 0.3f, -0.44f - drop * 0.2f),
                     V(s * 0.13f, 0.04f, -0.47f - drop * 0.35f), 0.065f, fur);
            Vec3 top(s * 0.15f, 0.56f - drop, 0.32f), knee(s * 0.17f, 0.36f - drop * 0.6f, 0.44f + drop * 0.25f);
            Vec3 hock(s * 0.15f, 0.18f, 0.42f + drop * 0.15f), paw(s * 0.14f, 0.04f, 0.33f + drop * 0.2f);
            mb.capsule(top, knee, 0.085f, fur, MAT_HIDE, 6);
            mb.capsule(knee, hock, 0.05f, shade(fur, 0.92f), MAT_HIDE, 6);
            mb.capsule(hock, paw + V(0, 0.03f, 0), 0.04f, shade(fur, 0.88f), MAT_HIDE, 6);
            mb.box(paw + V(0, -0.015f, -0.04f), V(0.05f, 0.025f, 0.07f), furD, MAT_HIDE);
        }
    }

    // corpo (levemente inclinado ao sentar)
    mb.push();
    if (sit) { mb.translate(0, 0.3f, 0.45f); mb.rotate(25, V(1, 0, 0)); mb.translate(0, -0.3f, -0.45f); }
    const float dy = -drop;
    mb.ellipsoid(V(0, 0.64f + dy, -0.25f), V(0.2f, 0.27f, 0.36f), fur, MAT_HIDE, 8);
    mb.ellipsoid(V(0, 0.6f + dy, 0.3f), V(0.19f, 0.24f, 0.3f), shade(fur, 0.96f), MAT_HIDE, 8);
    mb.ellipsoid(V(0, 0.62f + dy, 0.0f), V(0.19f, 0.25f, 0.3f), fur, MAT_HIDE, 8);
    mb.ellipsoid(V(0, 0.84f + dy, -0.3f), V(0.17f, 0.14f, 0.22f), furD, MAT_HIDE, 7);
    mb.sphere(V(0, 0.5f + dy, -0.55f), 0.1f, belly, MAT_HIDE, 6);
    // pescoco e cabeca
    const float hy = 0.8f + dy * 0.7f;
    mb.capsule(V(0, 0.7f + dy, -0.5f), V(0, hy, -0.72f), 0.12f, fur, MAT_HIDE, 7);
    mb.ellipsoid(V(0, hy + 0.02f, -0.82f), V(0.14f, 0.13f, 0.17f), fur, MAT_HIDE, 8);
    mb.cylinder(V(0, hy, -0.9f), V(0, hy - 0.03f, -1.1f), 0.085f, 0.05f, 6, shade(fur, 1.08f), MAT_HIDE);
    mb.sphere(V(0, hy - 0.03f, -1.11f), 0.035f, QColor(18, 14, 14), MAT_COAL, 6);
    mb.cylinder(V(0, hy - 0.085f, -0.9f), V(0, hy - 0.09f, -1.05f), 0.04f, 0.03f, 5, furD, MAT_HIDE);
    for (int s = -1; s <= 1; s += 2) {
        mb.cone(V(s * 0.03f, hy - 0.07f, -1.05f), V(s * 0.03f, hy - 0.125f, -1.05f), 0.012f, 4, QColor(235, 230, 210), MAT_BONE);
        mb.cone(V(s * 0.07f, hy + 0.1f, -0.78f), V(s * 0.09f, hy + 0.28f, -0.76f), 0.055f, 5, furD, MAT_HIDE);
        mb.sphere(V(s * 0.068f, hy + 0.04f, -0.93f), 0.024f, eyeC, MAT_EMISSIVE, 6);
    }
    // cauda
    std::vector<Vec3> tail = {V(0, 0.62f + dy, 0.5f), V(0, 0.52f + dy, 0.78f), V(0, 0.34f + dy * 0.5f, 0.95f), V(0, 0.22f, 0.98f)};
    mb.tube(tail, std::vector<float>{0.055f, 0.085f, 0.07f, 0.025f}, 6, shade(fur, 0.95f), MAT_HIDE);
    if (hell) {   // espinhos e rachaduras brilhantes
        for (int i = 0; i < 4; ++i) {
            float z = -0.35f + i * 0.2f;
            mb.cone(V(0, 0.84f + dy, z), V(0, 1.0f + dy, z + 0.05f), 0.04f, 5, QColor(22, 16, 16), MAT_HIDE);
        }
        const QColor orange(255, 120, 30), blue(90, 170, 255);
        for (int i = 0; i < 6; ++i) {
            int s = (i & 1) ? 1 : -1;
            float z = -0.35f + i * 0.14f;
            mb.box(V(s * 0.19f, 0.62f + dy + (i % 3) * 0.04f, z), V(0.008f, 0.07f, 0.012f), (i % 3 == 2) ? blue : orange, MAT_EMISSIVE);
        }
        mb.box(V(0, 0.855f + dy, -0.1f), V(0.012f, 0.006f, 0.1f), orange, MAT_EMISSIVE);
        mb.sphere(V(0, hy - 0.085f, -0.98f), 0.02f, orange, MAT_EMISSIVE, 5);
    }
    mb.pop();
}

// ---------------------------- fogo e iluminacao ----------------------------
void brazier(MeshBuilder& mb, const Vec3& pos, float scale, bool lit) {
    Rng r(seedOf(pos) + 3u);
    mb.push(); mb.translate(pos); mb.scale(scale);
    const QColor bronze(150, 105, 55);
    for (int i = 0; i < 3; ++i) {
        float a = i * 2.0944f + 0.3f;
        mb.cylinder(V(std::cos(a) * 0.36f, 0, std::sin(a) * 0.36f), V(std::cos(a) * 0.17f, 0.66f, std::sin(a) * 0.17f), 0.035f, 0.03f, 6, vary(bronze, r, 0.05f), MAT_BRONZE);
        mb.sphere(V(std::cos(a) * 0.36f, 0.02f, std::sin(a) * 0.36f), 0.045f, bronze, MAT_BRONZE, 6);
    }
    mb.torus(V(0, 0.3f, 0), V(0, 1, 0), 0.25f, 0.02f, shade(bronze, 0.9f), MAT_BRONZE, 12, 4);
    mb.cylinder(V(0, 0.6f, 0), V(0, 0.84f, 0), 0.14f, 0.31f, 10, bronze, MAT_BRONZE, true, false);
    mb.torus(V(0, 0.84f, 0), V(0, 1, 0), 0.31f, 0.03f, shade(bronze, 1.1f), MAT_BRONZE, 12, 4);
    // brasas
    int n = 11;
    for (int i = 0; i < n; ++i) {
        float a = rf(r, 0, 6.28f), d = rf(r, 0.0f, 0.24f), s = rf(r, 0.045f, 0.085f);
        Vec3 p(std::cos(a) * d, 0.85f + rf(r, 0.0f, 0.05f), std::sin(a) * d);
        if (lit) {
            QColor c = (i % 3 == 0) ? QColor(255, 200, 70) : QColor(255, 110, 25);
            mb.box(p, V(s, s * 0.8f, s), c, MAT_EMISSIVE);
        } else {
            mb.box(p, V(s, s * 0.8f, s), vary(QColor(32, 28, 28), r, 0.2f), MAT_COAL);
        }
    }
    if (lit) {
        mb.cone(V(0, 0.9f, 0), V(0.02f, 1.2f, 0), 0.17f, 6, QColor(255, 140, 30), MAT_EMISSIVE);
        mb.cone(V(0, 0.92f, 0), V(0.0f, 1.12f, 0.01f), 0.09f, 6, QColor(255, 220, 110), MAT_EMISSIVE);
    }
    mb.pop();
}

void wallTorch(MeshBuilder& mb, const Vec3& pos, const Vec3& away) {
    float yaw = std::atan2(-away.x(), -away.z()) * 180.f / kPi;
    Placed pl(mb, pos, yaw);   // -Z local = afastando da parede
    const QColor iron(60, 58, 56);
    mb.box(V(0, 0, 0.01f), V(0.07f, 0.1f, 0.01f), iron, MAT_RUSTY_IRON);
    mb.cylinder(V(0, -0.02f, 0), V(0, -0.02f, -0.2f), 0.015f, 5, iron, MAT_RUSTY_IRON);
    mb.torus(V(0, -0.02f, -0.2f), V(0, 1, 0), 0.05f, 0.012f, iron, MAT_RUSTY_IRON, 8, 4);
    mb.cylinder(V(0, -0.12f, -0.1f), V(0, 0.32f, -0.22f), 0.028f, 0.034f, 6, QColor(95, 66, 42), MAT_WOOD);
    mb.cone(V(0, 0.3f, -0.22f), V(0, 0.34f, -0.22f), 0.06f, 6, QColor(40, 30, 25), MAT_CLOTH);
    mb.cone(V(0, 0.33f, -0.22f), V(0.01f, 0.62f, -0.22f), 0.075f, 6, QColor(255, 140, 30), MAT_EMISSIVE);
    mb.cone(V(0, 0.34f, -0.22f), V(0.0f, 0.5f, -0.22f), 0.04f, 6, QColor(255, 225, 120), MAT_EMISSIVE);
}

void campfire(MeshBuilder& mb, const Vec3& pos, float scale) {
    Rng r(seedOf(pos) + 9u);
    mb.push(); mb.translate(pos); mb.scale(scale);
    for (int i = 0; i < 8; ++i) {
        float a = i * 0.785f + rf(r, -0.1f, 0.1f);
        mb.ellipsoid(V(std::cos(a) * 0.42f, 0.07f, std::sin(a) * 0.42f), V(0.12f, 0.09f, 0.1f), vary(QColor(95, 92, 88), r, 0.15f), MAT_LIMESTONE, 6);
    }
    mb.cylinder(V(0, 0.01f, 0), V(0, 0.03f, 0), 0.34f, 8, QColor(30, 25, 22), MAT_COAL);
    for (int i = 0; i < 4; ++i) {
        float a = i * 1.5708f + 0.4f;
        mb.cylinder(V(std::cos(a) * 0.38f, 0.07f, std::sin(a) * 0.38f), V(-std::cos(a) * 0.05f, 0.25f, -std::sin(a) * 0.05f), 0.05f, 0.04f, 6, vary(QColor(70, 48, 32), r, 0.15f), MAT_WOOD);
    }
    for (int i = 0; i < 6; ++i) {
        float a = rf(r, 0, 6.28f), d = rf(r, 0, 0.2f);
        mb.box(V(std::cos(a) * d, 0.08f, std::sin(a) * d), V(0.04f, 0.03f, 0.04f), QColor(255, 100, 25), MAT_EMISSIVE);
    }
    mb.cone(V(0, 0.15f, 0), V(0.03f, 0.75f, 0), 0.22f, 7, QColor(255, 130, 25), MAT_EMISSIVE);
    mb.cone(V(0.06f, 0.15f, 0.04f), V(0.1f, 0.5f, 0.02f), 0.12f, 6, QColor(255, 175, 50), MAT_EMISSIVE);
    mb.cone(V(0, 0.16f, 0), V(0, 0.45f, 0), 0.1f, 6, QColor(255, 235, 140), MAT_EMISSIVE);
    mb.pop();
}

// ---------------------------- caixas e recipientes ----------------------------
void crate(MeshBuilder& mb, const Vec3& pos, float yawDeg, float size) {
    Rng r(seedOf(pos) + 1u);
    Placed pl(mb, pos, yawDeg);
    const float h = size * 0.5f;
    QColor wood = vary(QColor(122, 88, 56), r, 0.08f), dark = shade(wood, 0.65f);
    mb.box(V(0, h, 0), V(h, h, h), wood, MAT_WOOD);
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2) mb.box(V(sx * (h - 0.03f), h, sz * (h - 0.03f)), V(0.045f, h + 0.005f, 0.045f), dark, MAT_WOOD);
    for (int sy = 0; sy <= 1; ++sy)
        for (int sz = -1; sz <= 1; sz += 2) {
            mb.box(V(0, sy ? size - 0.03f : 0.03f, sz * (h + 0.003f)), V(h, 0.03f, 0.012f), dark, MAT_WOOD);
            mb.box(V(sz * (h + 0.003f), sy ? size - 0.03f : 0.03f, 0), V(0.012f, 0.03f, h), dark, MAT_WOOD);
        }
    for (int sz = -1; sz <= 1; sz += 2) {   // diagonal na frente e atras
        mb.push(); mb.translate(0, h, sz * (h + 0.006f)); mb.rotate(45, V(0, 0, 1));
        mb.box(V(0, 0, 0), V(h * 1.3f, 0.035f, 0.01f), dark, MAT_WOOD);
        mb.pop();
    }
}

void barrel(MeshBuilder& mb, const Vec3& pos, float yawDeg, float scale) {
    Rng r(seedOf(pos) + 2u);
    Placed pl(mb, pos, yawDeg);
    mb.scale(scale);
    QColor wood = vary(QColor(110, 76, 48), r, 0.08f);
    mb.cylinder(V(0, 0, 0), V(0, 0.45f, 0), 0.29f, 0.36f, 10, wood, MAT_WOOD, true, false);
    mb.cylinder(V(0, 0.45f, 0), V(0, 0.9f, 0), 0.36f, 0.29f, 10, shade(wood, 1.05f), MAT_WOOD, true, false);
    mb.cylinder(V(0, 0.885f, 0), V(0, 0.905f, 0), 0.27f, 0.27f, 10, shade(wood, 0.8f), MAT_WOOD);
    const float ys[4] = {0.1f, 0.3f, 0.6f, 0.8f}, rs[4] = {0.3f, 0.355f, 0.355f, 0.31f};
    for (int i = 0; i < 4; ++i) mb.torus(V(0, ys[i], 0), V(0, 1, 0), rs[i] + 0.01f, 0.022f, QColor(60, 58, 58), MAT_IRON, 10, 4);
}

void sack(MeshBuilder& mb, const Vec3& pos, float yawDeg, float scale) {
    Rng r(seedOf(pos) + 4u);
    Placed pl(mb, pos, yawDeg);
    mb.scale(scale);
    QColor c = vary(QColor(160, 138, 100), r, 0.1f);
    mb.ellipsoid(V(0, 0.2f, 0), V(0.27f, 0.22f, 0.23f), c, MAT_CLOTH, 8);
    mb.ellipsoid(V(0, 0.34f, 0), V(0.2f, 0.16f, 0.18f), shade(c, 1.05f), MAT_CLOTH, 7);
    mb.cylinder(V(0, 0.42f, 0), V(0, 0.5f, 0), 0.09f, 0.06f, 6, shade(c, 0.9f), MAT_CLOTH);
    mb.torus(V(0, 0.43f, 0), V(0, 1, 0), 0.085f, 0.014f, QColor(110, 85, 55), MAT_ROPE, 8, 4);
    mb.cone(V(0, 0.48f, 0), V(0.05f, 0.6f, 0.02f), 0.07f, 5, shade(c, 1.1f), MAT_CLOTH);
}

void hideTent(MeshBuilder& mb, const Vec3& pos, float yawDeg, float radius, float height, const QColor& cloth) {
    Rng r(seedOf(pos) + 6u);
    Placed pl(mb, pos, yawDeg);
    mb.cylinder(V(0, 0, 0), V(0, height, 0), radius, 0.f, 8, cloth, MAT_CLOTH, false, false);
    for (int i = 0; i < 4; ++i) {   // varas cruzando no topo
        float a = i * 1.5708f + 0.4f;
        mb.cylinder(V(std::cos(a) * radius * 0.92f, 0, std::sin(a) * radius * 0.92f), V(std::cos(a + 3.14f) * 0.02f, height + 0.45f, 0.02f), 0.035f, 0.025f, 5, vary(QColor(100, 72, 48), r, 0.1f), MAT_WOOD);
    }
    // remendos de couro
    for (int i = 0; i < 3; ++i) {
        float a = 0.9f + i * 2.0f, hh = 0.15f + i * 0.08f;
        float rr = radius * (1.f - hh / height) * 1.01f;
        mb.box(V(std::cos(a) * rr, height * hh, std::sin(a) * rr), V(0.12f * radius, 0.1f, 0.012f), vary(shade(cloth, 0.75f), r, 0.1f), MAT_HIDE);
    }
    // entrada escura (frente = -Z)
    float k = 1.02f;
    mb.triangle(V(-0.24f * radius, 0.0f, -0.97f * radius * k), V(0.24f * radius, 0.0f, -0.97f * radius * k), V(0, 0.62f * height, -0.38f * radius * k), QColor(30, 20, 18), MAT_HIDE);
    mb.box(V(0, 0.04f, -radius * 0.97f), V(0.28f * radius, 0.04f, 0.04f), QColor(80, 60, 45), MAT_WOOD);
}

// ---------------------------- ossos ----------------------------
void bonesPile(MeshBuilder& mb, const Vec3& pos, float radius, int seed) {
    Rng r((unsigned)seed * 2654435761u + 17u);
    Placed pl(mb, pos, 0.f);
    int n = std::max(5, (int)(radius * 14));
    for (int i = 0; i < n; ++i) {
        float a = rf(r, 0, 6.283f), d = radius * std::sqrt(rf(r, 0, 1));
        float ang = rf(r, 0, 6.283f), len = rf(r, 0.14f, 0.4f) * std::min(1.f, 0.6f + radius * 0.5f);
        Vec3 c(std::cos(a) * d, 0.03f + rf(r, 0.0f, 0.07f), std::sin(a) * d);
        Vec3 dir(std::cos(ang), rf(r, -0.2f, 0.4f), std::sin(ang));
        dir.normalize();
        boneBetween(mb, c - dir * len * 0.5f, c + dir * len * 0.5f, rf(r, 0.014f, 0.024f), vary(QColor(222, 212, 184), r, 0.08f));
    }
    int nsk = 1 + (int)(radius > 0.8f);
    for (int i = 0; i < nsk; ++i) {
        float a = rf(r, 0, 6.283f), d = radius * rf(r, 0.2f, 0.7f);
        mb.push(); mb.translate(std::cos(a) * d, 0.02f, std::sin(a) * d); mb.rotateY(rf(r, 0, 360));
        mb.rotate(rf(r, -15, 15), V(0, 0, 1));
        skullLocal(mb, vary(QColor(225, 215, 190), r, 0.06f));
        mb.pop();
    }
    for (int i = 0; i < 2; ++i) {   // um par de costelas
        float a = rf(r, 0, 6.283f), d = radius * rf(r, 0.1f, 0.6f);
        mb.push(); mb.translate(std::cos(a) * d, 0.02f, std::sin(a) * d); mb.rotateY(rf(r, 0, 360));
        std::vector<Vec3> pts;
        for (int k = 0; k <= 6; ++k) { float t = k / 6.f * 2.4f; pts.push_back(V(std::sin(t) * 0.14f, 0.015f + (1 - std::cos(t)) * 0.05f, -t * 0.04f)); }
        mb.tube(pts, std::vector<float>{0.016f, 0.012f}, 5, QColor(215, 205, 178), MAT_BONE);
        mb.pop();
    }
}

void skull(MeshBuilder& mb, const Vec3& pos, float yawDeg, float scale) {
    Placed pl(mb, pos, yawDeg);
    mb.scale(scale);
    skullLocal(mb, QColor(228, 218, 192));
}

void ribcage(MeshBuilder& mb, const Vec3& pos, float yawDeg, float scale) {
    Placed pl(mb, pos, yawDeg);
    mb.scale(scale);
    QColor bone(222, 212, 184);
    mb.cylinder(V(0, 0.05f, -0.45f), V(0, 0.05f, 0.45f), 0.03f, 6, shade(bone, 0.9f), MAT_BONE);
    for (int i = 0; i < 7; ++i) {
        float z = -0.36f + i * 0.12f;
        float rr = 0.26f * (0.65f + 0.35f * std::sin((i + 1) / 8.f * kPi));
        std::vector<Vec3> pts;
        for (int k = 0; k <= 8; ++k) {
            float t = k / 8.f * kPi;
            pts.push_back(V(-std::cos(t) * rr, 0.05f + std::sin(t) * rr * 1.05f, z));
        }
        mb.tube(pts, std::vector<float>{0.014f, 0.02f, 0.02f, 0.02f, 0.02f, 0.02f, 0.02f, 0.02f, 0.014f}, 5, shade(bone, 1.f - 0.02f * (i % 3)), MAT_BONE);
    }
}

void giantRib(MeshBuilder& mb, const Vec3& base, const Vec3& tip, float radius, float archHeight, int seed) {
    Rng r((unsigned)seed * 40503u + 5u);
    const int n = 10;
    std::vector<Vec3> pts; std::vector<float> rad;
    Vec3 wob(rf(r, -0.2f, 0.2f), 0, rf(r, -0.2f, 0.2f));
    for (int i = 0; i <= n; ++i) {
        float t = (float)i / n;
        Vec3 p = base + (tip - base) * t;
        p.setY(p.y() + archHeight * std::sin(t * kPi * 0.85f));
        p += wob * std::sin(t * kPi);
        pts.push_back(p);
        rad.push_back(radius * (1.f - 0.65f * t));
    }
    QColor bone = vary(QColor(214, 202, 170), r, 0.06f);
    mb.tube(pts, rad, 7, bone, MAT_BONE);
    mb.sphere(base, radius * 1.4f, shade(bone, 0.95f), MAT_BONE, 7);
}

// ---------------------------- corrente / gaiola / ponte ----------------------------
void chain(MeshBuilder& mb, const Vec3& a, const Vec3& b, float sag, float linkSize, int material) {
    std::vector<Vec3> poly = MeshBuilder::catenary(a, b, sag, 24);
    QColor c(80, 74, 68);
    float step = linkSize * 1.15f;
    float acc = 0.f; int idx = 0; bool flip = false;
    for (size_t i = 0; i + 1 < poly.size(); ++i) {
        Vec3 seg = poly[i + 1] - poly[i];
        float L = seg.length();
        if (L < 1e-5f) continue;
        Vec3 d = seg / L;
        while (acc <= L) {
            Vec3 p = poly[i] + d * acc;
            mb.push(); mb.translate(p); orientY(mb, d);
            if (flip) mb.rotate(90, V(0, 1, 0));
            mb.scale(V(1, 1.5f, 1));
            mb.torus(V(0, 0, 0), V(1, 0, 0), linkSize * 0.42f, linkSize * 0.11f, shade(c, 1.f - 0.05f * (idx % 3)), material, 7, 4);
            mb.pop();
            flip = !flip; ++idx;
            acc += step;
        }
        acc -= L;
    }
}

void hangingCage(MeshBuilder& mb, const Vec3& attachTop, float size, float chainLength, bool withPrisoner) {
    const float R = size * 0.4f, topY = attachTop.y() - chainLength, botY = topY - size;
    const QColor iron(62, 58, 56);
    mb.push(); mb.translate(attachTop.x(), 0, attachTop.z());
    const Vec3 hook(0, attachTop.y(), 0);
    mb.torus(hook, V(1, 0, 0), 0.1f, 0.02f, iron, MAT_RUSTY_IRON, 8, 4);
    // correntes em V ate o anel superior
    for (int i = 0; i < 3; ++i) {
        float a = i * 2.0944f + 1.0f;
        chain(mb, hook, V(std::cos(a) * R * 0.5f, topY + 0.25f, std::sin(a) * R * 0.5f), 0.03f, 0.1f);
    }
    // domo: barras convergindo
    const Vec3 apex(0, topY + 0.25f, 0);
    for (int i = 0; i < 8; ++i) {
        float a = i * kPi / 4.f;
        mb.cylinder(V(std::cos(a) * R, topY, std::sin(a) * R), apex, 0.02f, 0.02f, 5, iron, MAT_RUSTY_IRON);
    }
    mb.torus(V(0, topY, 0), V(0, 1, 0), R, 0.03f, iron, MAT_RUSTY_IRON, 12, 4);
    mb.torus(V(0, botY + 0.05f, 0), V(0, 1, 0), R, 0.03f, iron, MAT_RUSTY_IRON, 12, 4);
    mb.torus(V(0, topY - size * 0.33f, 0), V(0, 1, 0), R, 0.022f, iron, MAT_RUSTY_IRON, 12, 4);
    mb.torus(V(0, topY - size * 0.66f, 0), V(0, 1, 0), R, 0.022f, iron, MAT_RUSTY_IRON, 12, 4);
    mb.cylinder(V(0, botY, 0), V(0, botY + 0.05f, 0), R + 0.02f, R + 0.02f, 12, QColor(55, 50, 46), MAT_RUSTY_IRON);
    // barras verticais (as da frente formam a porta)
    for (int i = 0; i < 12; ++i) {
        float a = i * kPi / 6.f;
        float x = std::sin(a) * R, z = -std::cos(a) * R;
        bool door = (std::fabs(z + R) < R * 0.4f && z < 0) && (std::fabs(x) < R * 0.55f);
        mb.cylinder(V(x, botY + 0.05f, z), V(x, topY, z), 0.018f, 5, door ? QColor(80, 72, 66) : iron, MAT_RUSTY_IRON);
    }
    // porta: dobradicas e cadeado
    mb.box(V(-R * 0.5f, topY - size * 0.5f, -R * 0.87f), V(0.02f, size * 0.38f, 0.02f), QColor(95, 90, 84), MAT_IRON);
    mb.box(V(R * 0.5f, topY - size * 0.5f, -R * 0.87f), V(0.02f, size * 0.38f, 0.02f), QColor(95, 90, 84), MAT_IRON);
    mb.box(V(R * 0.5f + 0.03f, topY - size * 0.55f, -R * 0.9f - 0.01f), V(0.05f, 0.06f, 0.025f), QColor(140, 100, 50), MAT_BRONZE);
    if (withPrisoner) {
        HumanoidParams p;
        p.height = 1.65f; p.build = 0.85f; p.pose = Pose::Huddle;
        p.skin = QColor(185, 150, 125); p.cloth = QColor(80, 70, 62); p.cloth2 = QColor(60, 55, 50);
        humanoid(mb, V(0, botY + 0.05f, 0), 20.f, p);
    }
    mb.pop();
}

void ropeBridge(MeshBuilder& mb, const Vec3& a, const Vec3& b, float width, float sag, int planks, bool rotten, int seed) {
    Rng r((unsigned)seed * 7919u + 13u);
    planks = std::max(2, planks);
    auto P = [&](float t) { Vec3 p = a + (b - a) * t; p.setY(p.y() - sag * 4.f * t * (1.f - t)); return p; };
    Vec3 dh = b - a; dh.setY(0);
    Vec3 sideV = Vec3(dh.z(), 0, -dh.x());
    if (sideV.lengthSquared() < 1e-6f) sideV = Vec3(1, 0, 0);
    sideV.normalize();
    const float len = (b - a).length();
    const float plankLen = len / planks * 0.8f;
    const float hw = width * 0.5f;
    // cordas de sustentacao sob as tabuas
    for (int s = -1; s <= 1; s += 2) {
        std::vector<Vec3> pts;
        for (int i = 0; i <= 16; ++i) pts.push_back(P(i / 16.f) + sideV * (s * (hw - 0.1f)) + V(0, -0.06f, 0));
        mb.tube(pts, 0.035f, 6, QColor(135, 110, 75), MAT_ROPE);
    }
    // tabuas
    for (int i = 0; i < planks; ++i) {
        float t = (i + 0.5f) / planks;
        bool missing = false, crooked = false, broken = false, rot = false;
        if (rotten) {
            float q = rf(r, 0, 1);
            missing = (q < 0.1f && i > 0 && i < planks - 1);
            crooked = q >= 0.1f && q < 0.3f;
            broken = q >= 0.3f && q < 0.42f;
            rot = rf(r, 0, 1) < 0.45f;
        }
        if (missing) continue;
        Vec3 c = P(t);
        Vec3 T = P(std::min(1.f, t + 0.5f / planks)) - P(std::max(0.f, t - 0.5f / planks));
        mb.push(); mb.translate(c); mb.rotate(frameAlong(T));
        float xoff = 0.f, halfW = hw * 0.97f;
        if (crooked) { mb.rotate(rf(r, -14, 14), V(0, 1, 0)); mb.rotate(rf(r, -9, 9), V(0, 0, 1)); mb.translate(0, rf(r, -0.04f, 0.0f), 0); }
        if (broken) { halfW = hw * 0.5f; xoff = (rf(r, 0, 1) < 0.5f ? -1.f : 1.f) * hw * 0.45f; }
        QColor col = rot ? vary(QColor(78, 62, 46), r, 0.1f) : vary(QColor(122, 90, 58), r, 0.1f);
        mb.box(V(xoff, 0, 0), V(halfW, 0.025f, plankLen * 0.5f), col, rot ? MAT_ROTTEN_WOOD : MAT_WOOD);
        mb.pop();
    }
    // postes e corrimaos
    std::vector<float> ts; ts.push_back(0.f);
    for (int i = 2; i < planks; i += 4) ts.push_back((i + 0.5f) / planks);
    ts.push_back(1.f);
    for (int s = -1; s <= 1; s += 2) {
        std::vector<Vec3> basePts;
        for (float t : ts) {
            Vec3 bp = P(t) + sideV * (s * hw);
            basePts.push_back(bp);
            bool endp = (t == 0.f || t == 1.f);
            mb.cylinder(bp + V(0, -0.1f, 0), bp + V(0, 1.1f, 0), endp ? 0.055f : 0.04f, endp ? 0.045f : 0.035f, 6, vary(QColor(105, 76, 50), r, 0.1f), rotten ? MAT_ROTTEN_WOOD : MAT_WOOD);
        }
        for (size_t k = 0; k + 1 < basePts.size(); ++k) {
            mb.tube(MeshBuilder::catenary(basePts[k] + V(0, 1.0f, 0), basePts[k + 1] + V(0, 1.0f, 0), 0.07f, 5), 0.025f, 5, QColor(140, 115, 80), MAT_ROPE);
            bool snap = rotten && rf(r, 0, 1) < 0.25f;
            if (!snap) mb.tube(MeshBuilder::catenary(basePts[k] + V(0, 0.5f, 0), basePts[k + 1] + V(0, 0.5f, 0), 0.08f, 5), 0.02f, 5, QColor(130, 105, 72), MAT_ROPE);
            else mb.cylinder(basePts[k] + V(0, 0.5f, 0), basePts[k] + V(0.04f, 0.1f, 0.0f), 0.015f, 4, QColor(130, 105, 72), MAT_ROPE);
        }
    }
}

// ---------------------------- mobiliario ----------------------------
void table(MeshBuilder& mb, const Vec3& pos, float yawDeg, const Vec3& size, bool withBooksAndPapers) {
    Rng r(seedOf(pos) + 8u);
    Placed pl(mb, pos, yawDeg);
    QColor wood = vary(QColor(112, 80, 52), r, 0.06f);
    const float hx = size.x() * 0.5f, hz = size.z() * 0.5f, H = size.y();
    mb.box(V(0, H - 0.035f, 0), V(hx, 0.035f, hz), wood, MAT_WOOD);
    mb.box(V(0, H - 0.1f, 0), V(hx - 0.06f, 0.04f, hz - 0.06f), shade(wood, 0.7f), MAT_WOOD);
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sz = -1; sz <= 1; sz += 2) mb.box(V(sx * (hx - 0.08f), (H - 0.07f) * 0.5f, sz * (hz - 0.08f)), V(0.045f, (H - 0.07f) * 0.5f, 0.045f), shade(wood, 0.85f), MAT_WOOD);
    if (withBooksAndPapers) {
        float y = H;
        const QColor bc[3] = {QColor(110, 40, 40), QColor(40, 70, 100), QColor(70, 90, 50)};
        for (int i = 0; i < 3; ++i) {
            float th = 0.04f + 0.01f * i;
            mb.push(); mb.translate(-hx * 0.55f, y + i * 0.045f, -hz * 0.2f); mb.rotateY(i * 7.f - 6.f);
            mb.box(V(0, th * 0.5f, 0), V(0.13f, th * 0.5f, 0.09f), bc[i], MAT_CLOTH);
            mb.box(V(0.005f, th * 0.5f, 0.0f), V(0.125f, th * 0.5f - 0.004f, 0.092f), QColor(220, 210, 180), MAT_PAPER);
            mb.pop();
        }
        for (int i = 0; i < 2; ++i) {
            mb.push(); mb.translate(hx * (0.1f + i * 0.2f), y + 0.004f + i * 0.004f, hz * 0.1f); mb.rotateY(-18.f + i * 40.f);
            mb.box(V(0, 0, 0), V(0.14f, 0.002f, 0.1f), vary(QColor(225, 215, 185), r, 0.04f), MAT_PAPER);
            mb.pop();
        }
        mb.cylinder(V(hx * 0.55f, y + 0.03f, -hz * 0.3f), V(hx * 0.55f + 0.2f, y + 0.03f, -hz * 0.3f + 0.04f), 0.03f, 6, QColor(215, 200, 165), MAT_PAPER);
        mb.cylinder(V(hx * 0.7f, y, hz * 0.35f), V(hx * 0.7f, y + 0.14f, hz * 0.35f), 0.022f, 6, QColor(230, 220, 190), MAT_BONE);
        mb.sphere(V(hx * 0.7f, y + 0.16f, hz * 0.35f), 0.016f, QColor(255, 200, 90), MAT_EMISSIVE, 5);
    }
}

void chest(MeshBuilder& mb, const Vec3& pos, float yawDeg, bool open, float scale) {
    Rng r(seedOf(pos) + 10u);
    Placed pl(mb, pos, yawDeg);
    mb.scale(scale);
    const float w = 0.5f, d = 0.3f, h = 0.4f;
    QColor wood = vary(QColor(110, 74, 44), r, 0.06f), band(70, 66, 62);
    mb.box(V(0, h * 0.5f, 0), V(w, h * 0.5f, d), wood, MAT_WOOD);
    for (int s = -1; s <= 1; s += 2) mb.box(V(s * w * 0.62f, h * 0.5f, 0), V(0.035f, h * 0.5f + 0.005f, d + 0.008f), band, MAT_IRON);
    mb.box(V(0, 0.02f, 0), V(w + 0.01f, 0.02f, d + 0.01f), band, MAT_IRON);
    mb.push();
    if (open) {
        mb.translate(0, h, d);
        mb.rotate(105, V(1, 0, 0));
        mb.translate(0, 0, -d);
    } else mb.translate(0, h, 0);
    mb.cylinder(V(-w, 0, 0), V(w, 0, 0), d, d, 10, shade(wood, 1.05f), MAT_WOOD);
    for (int s = -1; s <= 1; s += 2) mb.cylinder(V(s * w * 0.62f - 0.035f, 0, 0), V(s * w * 0.62f + 0.035f, 0, 0), d + 0.012f, d + 0.012f, 10, band, MAT_IRON);
    mb.pop();
    mb.box(V(0, h - 0.07f, -d - 0.012f), V(0.05f, 0.06f, 0.014f), QColor(150, 108, 52), MAT_BRONZE);
    if (open) {
        mb.box(V(0, h - 0.04f, 0), V(w - 0.03f, 0.01f, d - 0.03f), QColor(40, 28, 22), MAT_CLOTH);
        for (int i = 0; i < 9; ++i) {
            mb.ellipsoid(V(rf(r, -w * 0.8f, w * 0.8f), h - 0.01f + rf(r, 0, 0.04f), rf(r, -d * 0.7f, d * 0.7f)), V(0.05f, 0.025f, 0.05f), vary(QColor(235, 190, 60), r, 0.12f), MAT_GOLD, 6);
        }
    }
}

// ---------------------------- capsula necromantica ----------------------------
void glassCapsule(MeshBuilder& mb, const Vec3& pos, float height, float radius, bool figure, bool depleted, bool open) {
    Rng r(seedOf(pos) + 12u);
    Placed pl(mb, pos, 0.f);
    const float H = height, R = radius, baseH = 0.25f, capH = 0.25f;
    const QColor iron(58, 56, 56), bronze(150, 105, 55);
    mb.cylinder(V(0, 0, 0), V(0, 0.07f, 0), R + 0.22f, R + 0.22f, 12, shade(iron, 0.9f), MAT_RUSTY_IRON);
    mb.cylinder(V(0, 0.07f, 0), V(0, baseH, 0), R + 0.14f, R + 0.12f, 12, iron, MAT_IRON);
    mb.torus(V(0, baseH, 0), V(0, 1, 0), R + 0.02f, 0.045f, bronze, MAT_BRONZE, 14, 4);
    mb.cylinder(V(0, H - capH, 0), V(0, H, 0), R + 0.12f, R + 0.14f, 12, iron, MAT_IRON);
    mb.torus(V(0, H - capH, 0), V(0, 1, 0), R + 0.02f, 0.045f, bronze, MAT_BRONZE, 14, 4);
    mb.cone(V(0, H, 0), V(0, H + 0.14f, 0), R * 0.7f, 12, bronze, MAT_BRONZE);
    mb.cylinder(V(0, H + 0.1f, 0), V(0, H + 0.3f, 0), 0.09f, 0.08f, 8, bronze, MAT_BRONZE);
    // barras de sustentacao
    for (int k = 0; k < 4; ++k) {
        float a = (k * 90.f + 45.f) * kPi / 180.f;
        mb.cylinder(V(std::cos(a) * (R + 0.04f), baseH, std::sin(a) * (R + 0.04f)), V(std::cos(a) * (R + 0.04f), H - capH, std::sin(a) * (R + 0.04f)), 0.035f, 6, iron, MAT_IRON);
    }
    const float y0 = baseH, y1 = H - capH;
    QColor glass = depleted ? QColor(40, 52, 52, 150) : QColor(170, 225, 235, 70);
    if (!open) {
        mb.cylinder(V(0, y0, 0), V(0, y1, 0), R, R, 12, glass, MAT_GLASS, false);
    } else {
        const int N = 12;
        auto Pt = [&](int k, float y) { float th = k * 2.f * kPi / N; return V(R * std::sin(th), y, -R * std::cos(th)); };
        for (int k = 0; k < N; ++k) {
            float mid = (k + 0.5f) * 360.f / N; if (mid > 180.f) mid -= 360.f;
            if (std::fabs(mid) < 50.f) continue;   // painel frontal removido
            mb.quad(Pt(k, y0), Pt(k + 1, y0), Pt(k + 1, y1), Pt(k, y1), glass, MAT_GLASS);
        }
        // porta articulada, aberta para fora
        Vec3 hinge = Pt(2, 0);   // theta = 60 graus
        float w = 2.f * R * std::sin(30.f * kPi / 180.f) * 1.7f * 0.5f + R * 0.4f;
        mb.push(); mb.translate(hinge.x(), 0, hinge.z()); mb.rotateY(-95.f);
        mb.quad(V(0, y0, 0), V(-w, y0, 0), V(-w, y1, 0), V(0, y1, 0), glass, MAT_GLASS);
        mb.cylinder(V(0, y0, 0), V(0, y1, 0), 0.03f, 6, iron, MAT_IRON);
        mb.cylinder(V(-w, y0, 0), V(-w, y1, 0), 0.025f, 6, iron, MAT_IRON);
        mb.pop();
    }
    if (depleted) {   // rachaduras
        for (int c = 0; c < 4; ++c) {
            float a = rf(r, 0, 6.28f); float yy = rf(r, y0 + 0.2f, y1 - 0.3f);
            std::vector<Vec3> pts;
            for (int k = 0; k < 4; ++k) {
                float aa = a + rf(r, -0.12f, 0.12f) + k * 0.06f;
                pts.push_back(V(std::sin(aa) * (R + 0.006f), yy + k * 0.17f, -std::cos(aa) * (R + 0.006f)));
            }
            mb.tube(pts, 0.007f, 4, QColor(12, 16, 16), MAT_COAL);
        }
    } else if (!open) {
        mb.cylinder(V(0, y0 + 0.03f, 0), V(0, y1 - 0.2f, 0), R * 0.92f, R * 0.92f, 12, QColor(70, 235, 110, 110), MAT_NECRO_LIQUID, false);
        mb.cylinder(V(0, y1 - 0.2f, 0), V(0, y1 - 0.19f, 0), R * 0.92f, R * 0.92f, 12, QColor(110, 255, 150, 120), MAT_NECRO_LIQUID);
    } else {
        mb.cylinder(V(0, 0.005f, 0), V(0, 0.015f, 0), R * 1.5f, R * 1.5f, 12, QColor(70, 235, 110, 110), MAT_NECRO_LIQUID);
    }
    if (figure) {
        float figH = std::max(1.0f, std::min(1.7f, H - 0.9f));
        float fy = baseH + 0.3f;
        HumanoidParams p;
        p.height = figH; p.build = 0.9f; p.pose = Pose::Stand;
        p.skin = QColor(175, 200, 170); p.cloth = QColor(70, 82, 70); p.cloth2 = QColor(55, 62, 55);
        p.hair = QColor(60, 70, 60); p.glowingEyes = true; p.eyeColor = QColor(130, 255, 150);
        humanoid(mb, V(0, fy, 0), 0.f, p);
        float chestY = fy + figH * 1.3f / 1.75f;
        for (int s = -1; s <= 1; s += 2) {
            std::vector<Vec3> pts = {V(s * 0.07f, chestY, -0.12f * figH / 1.75f), V(s * 0.12f, chestY + 0.3f, -0.2f), V(s * 0.14f, y1 + 0.1f, -0.25f),
                                     V(s * 0.16f, H + 0.35f, -0.25f), V(s * 0.28f, H + 0.6f, -0.2f)};
            mb.tube(pts, 0.025f, 6, bronze, MAT_BRONZE);
            mb.torus(pts[0], V(0, 0, 1), 0.035f, 0.012f, shade(bronze, 0.8f), MAT_BRONZE, 8, 4);
        }
        std::vector<Vec3> mid = {V(0, chestY - 0.05f, -0.13f * figH / 1.75f), V(0, chestY + 0.25f, -0.22f), V(0, y1 + 0.1f, -0.27f), V(0, H + 0.4f, -0.27f), V(0, H + 0.7f, -0.1f)};
        mb.tube(mid, 0.03f, 6, bronze, MAT_BRONZE);
    }
}

void pipe(MeshBuilder& mb, const std::vector<Vec3>& pts, float radius, int material, bool clamps) {
    if (pts.size() < 2) return;
    QColor c = material == MAT_BRONZE ? QColor(150, 105, 55) : QColor(80, 76, 72);
    mb.tube(pts, radius, 8, c, material);
    if (!clamps) return;
    for (size_t i = 0; i + 1 < pts.size(); ++i) {
        Vec3 d = pts[i + 1] - pts[i];
        float L = d.length();
        if (L < 0.2f) continue;
        d /= L;
        int n = std::max(1, (int)(L / 1.2f));
        for (int k = 0; k < n; ++k) {
            Vec3 m = pts[i] + d * (L * (k + 0.5f) / n);
            mb.cylinder(m - d * 0.04f, m + d * 0.04f, radius * 1.4f, radius * 1.4f, 8, shade(c, 0.7f), MAT_IRON);
        }
    }
    for (size_t i = 1; i + 1 < pts.size(); ++i) mb.sphere(pts[i], radius * 1.2f, shade(c, 1.05f), material, 8);
}

// ---------------------------- estruturas de ferro ----------------------------
void scaffoldSegment(MeshBuilder& mb, const Vec3& a, const Vec3& b, float height, float width) {
    Rng r(seedOf(a) + 14u);
    Vec3 d = b - a; float L = d.length();
    if (L < 0.1f) return;
    mb.push(); mb.translate(a); mb.rotate(frameAlong(d));
    const QColor iron = QColor(88, 66, 52), ironD = QColor(70, 56, 48);
    const float hw = width * 0.5f;
    int bays = std::max(1, (int)std::ceil(L / 2.f));
    float bl = L / bays;
    for (int i = 0; i <= bays; ++i) {
        float z = i * bl;
        for (int s = -1; s <= 1; s += 2) mb.cylinder(V(s * hw, 0, z), V(s * hw, height, z), 0.045f, 6, vary(iron, r, 0.08f), MAT_RUSTY_IRON);
        for (int lv = 1; lv <= 2; ++lv) {
            float y = height * lv * 0.5f;
            mb.cylinder(V(-hw, y, z), V(hw, y, z), 0.035f, 5, ironD, MAT_RUSTY_IRON);
        }
    }
    for (int s = -1; s <= 1; s += 2)
        for (int lv = 1; lv <= 2; ++lv) mb.cylinder(V(s * hw, height * lv * 0.5f, 0), V(s * hw, height * lv * 0.5f, L), 0.035f, 5, ironD, MAT_RUSTY_IRON);
    for (int i = 0; i < bays; ++i)
        for (int s = -1; s <= 1; s += 2) {
            float z0 = i * bl, z1 = z0 + bl;
            mb.cylinder(V(s * hw, 0, z0), V(s * hw, height, z1), 0.022f, 4, iron, MAT_RUSTY_IRON);
            mb.cylinder(V(s * hw, 0, z1), V(s * hw, height, z0), 0.022f, 4, iron, MAT_RUSTY_IRON);
        }
    // tabuas no topo
    for (int i = 0; i < bays; ++i)
        for (int k = 0; k < 3; ++k) {
            float z = i * bl + (k + 0.5f) * bl / 3.f;
            mb.box(V(0, height + 0.03f, z), V(hw + 0.05f, 0.025f, bl / 6.f - 0.01f), vary(QColor(108, 78, 50), r, 0.12f), MAT_WOOD);
        }
    mb.pop();
}

void gratedWalkway(MeshBuilder& mb, const Vec3& a, const Vec3& b, float width, bool rails) {
    Rng r(seedOf(a) + 15u);
    Vec3 d = b - a; float L = d.length();
    if (L < 0.1f) return;
    mb.push(); mb.translate(a); mb.rotate(frameAlong(d));
    const QColor iron(84, 64, 52);
    const float hw = width * 0.5f;
    for (int s = -1; s <= 1; s += 2) mb.box(V(s * hw, -0.05f, L * 0.5f), V(0.04f, 0.08f, L * 0.5f), iron, MAT_RUSTY_IRON);
    int nb = std::max(2, (int)(L / 0.5f));
    for (int i = 0; i <= nb; ++i) mb.box(V(0, -0.02f, L * i / nb), V(hw, 0.015f, 0.015f), shade(iron, 0.85f), MAT_RUSTY_IRON);
    int nl = std::max(3, (int)(width / 0.14f));
    for (int k = 0; k <= nl; ++k) {
        float x = -hw + width * k / nl;
        mb.box(V(x, 0.0f, L * 0.5f), V(0.01f, 0.025f, L * 0.5f), vary(iron, r, 0.1f), MAT_RUSTY_IRON);
    }
    if (rails) {
        int np = std::max(2, (int)(L / 1.5f) + 1);
        for (int s = -1; s <= 1; s += 2) {
            for (int i = 0; i < np; ++i) mb.cylinder(V(s * hw, -0.05f, L * i / (np - 1)), V(s * hw, 1.0f, L * i / (np - 1)), 0.03f, 5, iron, MAT_RUSTY_IRON);
            mb.cylinder(V(s * hw, 1.0f, 0), V(s * hw, 1.0f, L), 0.028f, 5, shade(iron, 1.1f), MAT_RUSTY_IRON);
            mb.cylinder(V(s * hw, 0.5f, 0), V(s * hw, 0.5f, L), 0.02f, 4, shade(iron, 0.9f), MAT_RUSTY_IRON);
        }
    }
    mb.pop();
}

void ironStairs(MeshBuilder& mb, const Vec3& bottom, const Vec3& top, float width) {
    Vec3 d = top - bottom; float H = d.y();
    Vec3 dh = d; dh.setY(0);
    float Lh = dh.length();
    if (Lh < 0.05f || std::fabs(H) < 0.05f) return;
    Rng r(seedOf(bottom) + 16u);
    mb.push(); mb.translate(bottom); mb.rotate(frameAlong(dh));
    const QColor iron(84, 64, 52);
    const float hw = width * 0.5f;
    int n = std::max(3, (int)std::lround(std::fabs(H) / 0.2f));
    float dz = Lh / n, dh2 = H / n;
    for (int i = 0; i < n; ++i) {
        float z = (i + 0.5f) * dz, y = (i + 1) * dh2;
        mb.box(V(0, y - 0.02f, z), V(hw - 0.03f, 0.02f, dz * 0.5f - 0.005f), vary(shade(iron, 1.05f), r, 0.08f), MAT_RUSTY_IRON);
        mb.box(V(0, y - dh2 * 0.5f, z - dz * 0.5f + 0.01f), V(hw - 0.03f, dh2 * 0.5f - 0.02f, 0.008f), shade(iron, 0.8f), MAT_RUSTY_IRON);
    }
    float sl = std::sqrt(H * H + Lh * Lh), ang = -std::atan2(H, Lh) * 180.f / kPi;
    for (int s = -1; s <= 1; s += 2) {
        mb.push(); mb.translate(s * hw, H * 0.5f - 0.1f, Lh * 0.5f); mb.rotate(ang, V(1, 0, 0));
        mb.box(V(0, 0, 0), V(0.03f, 0.12f, sl * 0.5f), shade(iron, 0.9f), MAT_RUSTY_IRON);
        mb.pop();
        for (int i = 0; i < n; i += 2) {
            float z = (i + 0.5f) * dz, y = (i + 1) * dh2;
            mb.cylinder(V(s * hw, y, z), V(s * hw, y + 0.95f, z), 0.025f, 5, iron, MAT_RUSTY_IRON);
        }
        mb.cylinder(V(s * hw, 0.95f + dh2, 0.5f * dz), V(s * hw, H + 0.95f - dh2 * 0.0f, Lh - 0.5f * dz), 0.028f, 5, shade(iron, 1.1f), MAT_RUSTY_IRON);
    }
    mb.pop();
}

void ladder(MeshBuilder& mb, const Vec3& bottom, const Vec3& top, float yawDeg, float width) {
    const QColor iron(88, 66, 52);
    Placed pl(mb, bottom, yawDeg);
    Vec3 up = top - bottom;
    float H = up.y();
    const float hw = width * 0.5f;
    for (int s = -1; s <= 1; s += 2) mb.cylinder(V(s * hw, 0, 0.04f), V(s * hw, H, 0.04f), 0.03f, 6, iron, MAT_RUSTY_IRON);
    int n = std::max(2, (int)(H / 0.3f));
    for (int i = 1; i <= n; ++i) {
        float y = H * i / (n + 1);
        mb.cylinder(V(-hw, y, 0.04f), V(hw, y, 0.04f), 0.02f, 5, shade(iron, 1.1f), MAT_RUSTY_IRON);
    }
    for (float y = 0.5f; y < H; y += 1.6f) {
        for (int s = -1; s <= 1; s += 2) mb.box(V(s * hw, y, 0.09f), V(0.015f, 0.02f, 0.06f), shade(iron, 0.8f), MAT_RUSTY_IRON);
    }
}

void lever(MeshBuilder& mb, const Vec3& pos, float yawDeg, float angleDeg) {
    Placed pl(mb, pos, yawDeg);
    const QColor iron(70, 66, 64);
    mb.box(V(0, 0.06f, 0), V(0.22f, 0.06f, 0.14f), iron, MAT_IRON);
    for (int s = -1; s <= 1; s += 2) mb.box(V(s * 0.1f, 0.2f, 0), V(0.02f, 0.1f, 0.1f), shade(iron, 0.85f), MAT_RUSTY_IRON);
    mb.cylinder(V(-0.12f, 0.24f, 0), V(0.12f, 0.24f, 0), 0.025f, 6, QColor(140, 100, 50), MAT_BRONZE);
    mb.push(); mb.translate(0, 0.24f, 0); mb.rotate(angleDeg, V(1, 0, 0));
    mb.cylinder(V(0, -0.1f, 0), V(0, 0.65f, 0), 0.025f, 0.02f, 6, QColor(95, 90, 86), MAT_IRON);
    mb.sphere(V(0, 0.68f, 0), 0.055f, QColor(190, 50, 40), MAT_BRONZE, 7);
    mb.pop();
}

void gear(MeshBuilder& mb, const Vec3& center, const Vec3& axis, float radius, int teeth, float thickness, int material) {
    Rng r(seedOf(center) + 18u);
    teeth = std::max(6, teeth);
    mb.push(); mb.translate(center); orientY(mb, axis);
    QColor c = material == MAT_BRONZE ? QColor(150, 105, 55) : QColor(88, 80, 74);
    float minor = radius * 0.07f;
    mb.push(); mb.scale(V(1, std::max(0.2f, thickness / (2 * minor)), 1));
    mb.torus(V(0, 0, 0), V(0, 1, 0), radius * 0.86f, minor, c, material, std::max(16, teeth * 2), 4);
    mb.pop();
    float tw = kPi * radius / teeth * 0.42f;
    for (int i = 0; i < teeth; ++i) {
        mb.push(); mb.rotateY(i * 360.f / teeth);
        mb.box(V(radius * 0.95f, 0, 0), V(radius * 0.1f, thickness * 0.5f, tw), vary(c, r, 0.06f), material);
        mb.pop();
    }
    int spokes = 6;
    for (int i = 0; i < spokes; ++i) {
        mb.push(); mb.rotateY(i * 360.f / spokes + 10.f);
        mb.box(V(radius * 0.45f, 0, 0), V(radius * 0.42f, thickness * 0.28f, radius * 0.045f), shade(c, 0.9f), material);
        mb.pop();
    }
    mb.cylinder(V(0, -thickness * 0.75f, 0), V(0, thickness * 0.75f, 0), radius * 0.17f, radius * 0.17f, 8, shade(c, 1.1f), material);
    mb.cylinder(V(0, -thickness * 1.2f, 0), V(0, thickness * 1.2f, 0), radius * 0.06f, radius * 0.06f, 6, shade(c, 0.8f), material);
    mb.pop();
}

// ---------------------------- natureza ----------------------------
void crystalCluster(MeshBuilder& mb, const Vec3& pos, const Vec3& up, float radius, int count, int seed, const QColor& color, bool emissive) {
    Rng r((unsigned)seed * 2246822519u + 99u);
    mb.push(); mb.translate(pos); orientY(mb, up);
    int mat = MAT_CRYSTAL;
    for (int i = 0; i < count; ++i) {
        float az = (i + rf(r, -0.3f, 0.3f)) * 2.f * kPi / std::max(1, count);
        float tilt = (i == 0 ? 0.f : rf(r, 15.f, 55.f)) * kPi / 180.f;
        Vec3 dir(std::sin(tilt) * std::cos(az), std::cos(tilt), std::sin(tilt) * std::sin(az));
        float off = i == 0 ? 0.f : radius * rf(r, 0.1f, 0.4f);
        Vec3 base(std::cos(az) * off, 0, std::sin(az) * off);
        float L = radius * rf(r, 1.0f, 2.2f) * (i == 0 ? 1.3f : 1.f);
        float rr = radius * rf(r, 0.12f, 0.22f);
        QColor col = vary(emissive ? color : shade(color, 0.45f), r, 0.12f);
        Vec3 bodyEnd = base + dir * (L * 0.7f);
        mb.cylinder(base - dir * 0.05f, bodyEnd, rr, rr, 6, col, mat, true, false);
        mb.cylinder(bodyEnd, base + dir * L, rr, 0.f, 6, shade(col, 1.15f), mat, false, false);
    }
    mb.pop();
}

void mushroom(MeshBuilder& mb, const Vec3& pos, float height, const QColor& cap, bool glowing) {
    Rng r(seedOf(pos) + 19u);
    mb.push(); mb.translate(pos);
    const float h = height;
    mb.cylinder(V(0, 0, 0), V(0.01f * h, h * 0.72f, 0), h * 0.09f, h * 0.06f, 6, QColor(215, 215, 195), glowing ? MAT_FUNGUS : MAT_FLESH);
    mb.ellipsoid(V(0.01f * h, h * 0.72f, 0), V(h * 0.4f, h * 0.26f, h * 0.4f), cap, glowing ? MAT_FUNGUS : MAT_FLESH, 8);
    mb.cylinder(V(0.01f * h, h * 0.71f, 0), V(0.01f * h, h * 0.72f, 0), h * 0.38f, h * 0.38f, 8, shade(cap, 0.6f), glowing ? MAT_FUNGUS : MAT_FLESH);
    if (glowing)
        for (int i = 0; i < 4; ++i) {
            float a = rf(r, 0, 6.28f), d = h * rf(r, 0.1f, 0.28f);
            mb.sphere(V(std::cos(a) * d, h * 0.72f + h * 0.2f, std::sin(a) * d), h * 0.04f, shade(cap, 1.3f), MAT_EMISSIVE, 5);
        }
    mb.pop();
}

void iceFringe(MeshBuilder& mb, const Vec3& from, const Vec3& to, int count, float maxLen, int seed) {
    Rng r((unsigned)seed * 3266489917u + 7u);
    for (int i = 0; i < count; ++i) {
        float t = (i + rf(r, 0.1f, 0.9f)) / std::max(1, count);
        Vec3 p = from + (to - from) * t;
        float len = maxLen * (rf(r, 0, 1) < 0.2f ? rf(r, 0.8f, 1.f) : rf(r, 0.25f, 0.7f));
        float rr = std::min(0.06f, len * 0.11f + 0.012f);
        Vec3 tip = p + V(rf(r, -0.03f, 0.03f), -len, rf(r, -0.03f, 0.03f));
        mb.cylinder(p + V(0, 0.01f, 0), tip, rr, 0.f, 6, QColor(190, 228, 255, 150), MAT_CLEAR_ICE, false, false);
    }
}

void altar(MeshBuilder& mb, const Vec3& pos, float yawDeg, const Vec3& size, int material) {
    Placed pl(mb, pos, yawDeg);
    const bool obs = material == MAT_OBSIDIAN;
    QColor c = obs ? QColor(32, 30, 40) : QColor(112, 106, 98);
    const float hx = size.x() * 0.5f, hz = size.z() * 0.5f, H = size.y();
    mb.box(V(0, H * 0.08f, 0), V(hx + 0.12f, H * 0.08f, hz + 0.12f), shade(c, 0.9f), material);
    mb.box(V(0, H * 0.5f, 0), V(hx * 0.88f, H * 0.34f, hz * 0.85f), c, material);
    mb.box(V(0, H - 0.06f, 0), V(hx + 0.05f, 0.06f, hz + 0.05f), shade(c, 1.15f), material);
    for (int s = -1; s <= 1; s += 2) {
        mb.box(V(s * hx * 0.5f, H * 0.5f, -hz * 0.85f - 0.01f), V(hx * 0.18f, H * 0.2f, 0.012f), shade(c, 0.6f), material);
        mb.box(V(s * (hx - 0.02f), H - 0.1f, 0), V(0.03f, 0.05f, hz * 0.7f), QColor(150, 105, 55), MAT_BRONZE);
    }
    mb.box(V(0, H + 0.002f, 0), V(hx * 0.8f, 0.004f, 0.025f), QColor(100, 20, 25), MAT_FLESH);
}

void totemOfBones(MeshBuilder& mb, const Vec3& pos, float height, int seed) {
    Rng r((unsigned)seed * 668265263u + 3u);
    Placed pl(mb, pos, rf(r, 0, 360));
    mb.cylinder(V(0, 0, 0), V(0, height, 0), 0.07f, 0.055f, 7, QColor(88, 62, 42), MAT_WOOD);
    // base de pedras e ossos
    for (int i = 0; i < 5; ++i) {
        float a = i * 1.2566f;
        mb.ellipsoid(V(std::cos(a) * 0.2f, 0.06f, std::sin(a) * 0.2f), V(0.1f, 0.07f, 0.09f), vary(QColor(96, 92, 88), r, 0.15f), MAT_LIMESTONE, 6);
        boneBetween(mb, V(std::cos(a) * 0.15f, 0.16f, std::sin(a) * 0.15f), V(std::cos(a) * 0.05f, 0.32f, std::sin(a) * 0.05f), 0.016f, QColor(220, 210, 182));
    }
    int n = std::max(2, (int)(height / 0.5f));
    for (int i = 0; i < n; ++i) {
        float y = 0.35f + i * (height - 0.75f) / std::max(1, n - 1);
        float sc = 0.9f + 0.1f * (i % 2);
        mb.push(); mb.translate(0, y, 0); mb.rotateY(i * 70.f - 35.f); mb.translate(0, -0.1f * sc, 0.0f);
        mb.translate(0, 0, -0.08f);
        mb.scale(sc * 1.1f);
        skullLocal(mb, vary(QColor(226, 216, 190), r, 0.06f));
        mb.pop();
        if (i % 2 == 0) {   // travessa de ossos com penduricalhos
            mb.push(); mb.translate(0, y + 0.28f, 0); mb.rotateY(i * 55.f);
            boneBetween(mb, V(-0.3f, 0, 0), V(0.3f, 0, 0), 0.018f, QColor(215, 205, 178));
            mb.torus(V(0, 0, 0), V(0, 1, 0), 0.075f, 0.014f, QColor(130, 100, 70), MAT_ROPE, 8, 4);
            for (int s = -1; s <= 1; s += 2) mb.cylinder(V(s * 0.24f, 0, 0), V(s * 0.24f, -0.15f, 0), 0.01f, 4, QColor(215, 205, 178), MAT_BONE);
            mb.pop();
        }
    }
    // cranio de chifres no topo
    mb.push(); mb.translate(0, height - 0.04f, 0);
    mb.scale(1.5f);
    skullLocal(mb, QColor(232, 222, 196));
    for (int s = -1; s <= 1; s += 2) {
        std::vector<Vec3> h = {V(s * 0.06f, 0.14f, -0.01f), V(s * 0.14f, 0.17f, -0.02f), V(s * 0.19f, 0.26f, -0.03f)};
        mb.tube(h, std::vector<float>{0.018f, 0.012f, 0.004f}, 5, QColor(200, 190, 160), MAT_BONE);
    }
    mb.pop();
    // penas
    const QColor fc[3] = {QColor(180, 50, 40), QColor(40, 90, 150), QColor(30, 30, 34)};
    for (int i = 0; i < 5; ++i) {
        float a = i * 1.2566f;
        Vec3 b(std::cos(a) * 0.05f, height - 0.06f, std::sin(a) * 0.05f);
        Vec3 t = b + V(std::cos(a) * 0.14f, 0.34f, std::sin(a) * 0.14f);
        Vec3 side = V(-std::sin(a), 0, std::cos(a)) * 0.04f;
        mb.quad(b - side, b + side, t + side * 0.3f, t - side * 0.3f, fc[i % 3], MAT_CLOTH);
    }
}

} // namespace Prefabs
