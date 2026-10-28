// CAMADA 2 — O Poco do Eco (2.1 a 2.6) e 2b, Corredor do Contraste Termico (2.7)
// Ver docs/MASMORRA.md. Usa so a API de World (docs/ENGINE_API.md) e content/Prefabs.h.
#include "Dungeon.h"
#include "world/World.h"
#include "Prefabs.h"
#include "Layer2_Util.h"
#include <memory>

#define Q(s) QStringLiteral(s)
using namespace L23;

namespace {

const Vec3 PC(40.f, 0.f, -180.f);      // eixo do poco
const float kDrop = 32.f;               // queda por volta
const float kAng0 = 133.f;              // theta inicial (graus)
const float kTopY = -19.f;              // topo da cornija em t = 0
const Vec3 kHeart(100.f, -287.f, -505.f);

float topAt(float t) { return kTopY - kDrop * t; }
float angAt(float t) { return kAng0 + 360.f * t; }
// ponto da cornija: volta t, raio r, dy acima da superficie
Vec3 lp(float t, float r, float dy = 0.f)
{
    float a = rad(angAt(t));
    return Vec3(PC.x() + r * std::cos(a), topAt(t) + dy, PC.z() + r * std::sin(a));
}
Vec3 tangentH(float t) { float a = rad(angAt(t)); return Vec3(-std::sin(a), 0.f, std::cos(a)); }
float yawAlong(float t) { Vec3 d = tangentH(t); return yawDir(d.x(), d.z()); }
float yawToAxis(float t) { Vec3 p = lp(t, 20.f); return yawToward(p, Vec3(PC.x(), p.y(), PC.z())); }

struct CrumbleStep {
    float tA = 0, tB = 0;
    std::vector<DynHandle> dyn;
    std::vector<Collider*> col;
    std::vector<Vec3> basePos;
    std::vector<QQuaternion> baseRots;
    int state = 0;          // 0 intacto, 1 tremendo, 2 caindo, 3 caiu
    float timer = 0.f, vy = 0.f;
};

struct IceSlab {
    DynHandle dyn = nullptr;
    Collider* col = nullptr;
    Vec3 base;
    bool thin = false;
    int state = 0;          // 0 ok, 1 rachando, 2 caindo, 3 caiu
    float timer = 0.f, vy = 0.f;
    float cx = 0, cz = 0, hx = 2.6f, hz = 2.0f;
};

void roperProp(MeshBuilder& mb, const Vec3& pos, float yaw)
{
    mb.push(); mb.translate(pos); mb.rotateY(yaw);
    QColor body(112, 106, 98), dark(78, 74, 70);
    mb.cylinder(V3(0, 0, 0), V3(0.05f, 3.1f, 0), 0.95f, 0.55f, 10, body, MAT_LIMESTONE);
    mb.cylinder(V3(0.05f, 3.05f, 0), V3(0.1f, 3.55f, 0), 0.55f, 0.f, 8, dark, MAT_LIMESTONE);
    for (int i = 0; i < 6; ++i) {
        float a = i * 1.0472f + 0.3f;
        Vec3 s(std::cos(a) * 0.45f, 2.7f - (i % 3) * 0.35f, std::sin(a) * 0.45f);
        std::vector<Vec3> pts = {s, s + V3(std::cos(a) * 0.6f, 0.35f, std::sin(a) * 0.6f), s + V3(std::cos(a) * 1.4f, -0.2f, std::sin(a) * 1.4f),
                                 V3(std::cos(a) * 2.0f, 0.05f, std::sin(a) * 2.0f)};
        mb.tube(pts, std::vector<float>{0.09f, 0.07f, 0.05f, 0.03f}, 6, QColor(125, 112, 100), MAT_HIDE);
    }
    mb.ellipsoid(V3(0, 2.25f, -0.52f), V3(0.2f, 0.13f, 0.1f), QColor(90, 84, 78), MAT_HIDE, 7);   // olho fechado
    mb.pop();
}

void ropeLadder(MeshBuilder& mb, const Vec3& top, float yaw, float len)
{
    mb.push(); mb.translate(top); mb.rotateY(yaw);
    QColor rope(150, 125, 85), wood(95, 70, 45);
    for (int s = -1; s <= 1; s += 2) {
        std::vector<Vec3> pts = {V3(s * 0.32f, 0, 0), V3(s * 0.32f + 0.03f * s, -len * 0.5f, 0.15f), V3(s * 0.3f - 0.05f * s, -len, 0.4f)};
        mb.tube(pts, std::vector<float>{0.025f, 0.025f, 0.015f}, 5, rope, MAT_ROPE);
    }
    for (float y = 0.4f; y < len - 0.3f; y += 0.42f) {
        if (((int)(y * 10)) % 7 == 3) continue;                     // degraus faltando
        float f = y / len;
        float z = 0.4f * f * f;
        mb.box(V3(0, -y, z), V3(0.34f, 0.018f, 0.04f), wood, MAT_ROTTEN_WOOD);
    }
    for (int s = -1; s <= 1; s += 2)
        for (int j = 0; j < 3; ++j)
            mb.cylinder(V3(s * 0.3f, -len, 0.4f), V3(s * 0.3f + (j - 1) * 0.06f, -len - 0.3f - j * 0.05f, 0.42f), 0.01f, 4, rope, MAT_ROPE);
    mb.pop();
}

} // namespace

void buildLayer2Pit(World& w)
{
    const QString kSess = Q("Sessão 2 · Nível 7");
    const QString kGrp = Q("Camada 2 · O Poço do Eco");
    Rnd rg(2024);

    // ============================================================== O POCO (geometria)
    {
        w.carve(Prim::cylinder(PC, 22.f, -178.f, -148.f).smooth(2.f).noise(0.9f).mat(MAT_OBSIDIAN).floorAt(-178.f).named(Q("poco_obsidiana")));
        w.carve(Prim::cylinder(PC, 22.f, -155.f, -50.f).smooth(2.f).noise(0.95f).mat(MAT_LIMESTONE).named(Q("poco_calcario")));
        w.carve(Prim::cylinder(PC, 22.f, -55.f, -17.f).smooth(2.f).noise(0.95f).mat(MAT_WET_LIMESTONE).named(Q("poco_gelo")));
        w.carve(Prim::ellipsoid(Vec3(PC.x(), -17.f, PC.z()), Vec3(22.f, 7.f, 22.f)).smooth(3.f).noise(0.8f).mat(MAT_WET_LIMESTONE).named(Q("poco_cupula")));
        // reentrancias naturais da parede (nichos e bojos), materiais seguindo a profundidade
        for (int i = 0; i < 34; ++i) {
            float a = rad(rg.f(0, 360)), y = rg.f(-172.f, -24.f), rr = 22.4f + rg.f(0.2f, 1.4f);
            Vec3 c(PC.x() + rr * std::cos(a), y, PC.z() + rr * std::sin(a));
            Vec3 r(rg.f(2.2f, 4.6f), rg.f(3.f, 9.f), rg.f(2.2f, 4.6f));
            int m = y < -150.f ? MAT_OBSIDIAN : (y > -55.f ? MAT_WET_LIMESTONE : MAT_LIMESTONE);
            w.carve(Prim::ellipsoid(c, r).smooth(2.f).noise(0.55f).mat(m));
        }
        // veios de obsidiana subindo pela parede (transicao irregular)
        for (int i = 0; i < 9; ++i) {
            float a = rad(rg.f(0, 360)), y0 = rg.f(-160.f, -150.f);
            Vec3 c(PC.x() + 22.2f * std::cos(a), y0 + 7.f, PC.z() + 22.2f * std::sin(a));
            w.carve(Prim::ellipsoid(c, V3(2.f, rg.f(8.f, 14.f), 2.f)).smooth(1.5f).noise(0.4f).mat(MAT_OBSIDIAN));
        }
    }

    // ============================================================== 2.1 A BORDA (boca do tunel goblin + rampa)
    {
        tun(w, {V3(22.f, -16.f, -162.f), V3(26.f, -18.6f, -166.f), V3(29.f, -19.f, -169.f)}, 2.2f, MAT_LIMESTONE, 0.3f);
        // rampa de calcario (fill) entre o piso do tunel (y ~ -17,5 na parede) e o patamar (y -19,2)
        Vec3 din(0.7071f, 0.f, -0.7071f);
        float ph = std::atan2(1.6f, 3.4f);
        Vec3 T = Vec3(din.x() * std::cos(ph), -std::sin(ph), din.z() * std::cos(ph));
        QQuaternion q = frameTangent(T);
        Vec3 topMid(PC.x() - 20.8f * 0.7071f, -18.3f, PC.z() + 20.8f * 0.7071f);
        Vec3 up = q.rotatedVector(Vec3(0, 1, 0));
        w.fill(Prim::box(topMid - up * 0.6f, V3(2.0f, 0.6f, 1.9f), q, 0.15f).noise(0.1f).smooth(0.3f).mat(MAT_WET_LIMESTONE));
    }

    // ============================================================== 2.2 AS CORNIJAS EM ESPIRAL
    struct Seg { float t0, t1, rin, rout; int mat; };
    const float tEnd = 4.995f;
    std::vector<Seg> segs = {
        {0.000f, 0.085f, 17.0f, 24.0f, MAT_WET_LIMESTONE},     // patamar da borda (5 m)
        {0.085f, 0.550f, 19.0f, 24.0f, MAT_WET_LIMESTONE},
        // 0.55-0.60: degrau esfarelento 1 (colisores)
        {0.600f, 0.880f, 19.0f, 24.0f, MAT_LIMESTONE},
        {0.880f, 0.900f, 20.0f, 24.0f, MAT_LIMESTONE},
        {0.900f, 1.050f, 21.0f, 24.0f, MAT_LIMESTONE},         // Passo do Desespero (1,0 m)
        {1.050f, 1.070f, 20.0f, 24.0f, MAT_LIMESTONE},
        {1.070f, 1.700f, 19.0f, 24.0f, MAT_LIMESTONE},
        // 1.70-1.75: degrau 2
        {1.750f, 2.505f, 19.0f, 24.0f, MAT_LIMESTONE},
        // 2.505-2.515: vao de 1,3 m
        {2.515f, 3.300f, 19.0f, 24.0f, MAT_LIMESTONE},
        // 3.30-3.35: degrau 3
        {3.350f, 4.200f, 19.0f, 24.0f, MAT_LIMESTONE},
        {4.200f, tEnd, 19.0f, 24.0f, MAT_OBSIDIAN},
    };
    for (const Seg& s : segs) {
        HelixDesc hd; hd.center = PC; hd.rInner = s.rin; hd.rOuter = s.rout; hd.startY = kTopY; hd.dropPerTurn = kDrop;
        hd.startAngleDeg = kAng0; hd.t0 = s.t0; hd.t1 = s.t1; hd.thickness = 2.6f;
        w.fill(Prim::helix(hd).noise(0.16f).smooth(0.25f).mat(s.mat));
    }

    // ---- degraus esfarelentos: lajes colisoras que desabam ----
    std::vector<std::shared_ptr<CrumbleStep>> steps;
    {
        const float rng[3][2] = {{0.55f, 0.60f}, {1.70f, 1.75f}, {3.30f, 3.35f}};
        for (int si = 0; si < 3; ++si) {
            auto st = std::make_shared<CrumbleStep>();
            st->tA = rng[si][0]; st->tB = rng[si][1];
            for (int j = 0; j < 3; ++j) {
                float tm = st->tA + (j + 0.5f) * (st->tB - st->tA) / 3.f;
                float r = 20.6f, s = kDrop / (6.2831853f * r);
                Vec3 Tn = Vec3(tangentH(tm).x(), -s, tangentH(tm).z());
                QQuaternion q = frameTangent(Tn);
                Vec3 up = q.rotatedVector(Vec3(0, 1, 0));
                Vec3 pos = lp(tm, r) - up * 0.25f;
                MeshBuilder mb;
                Rnd rr(100 + si * 10 + j);
                mb.box(V3(0, 0, 0), V3(1.12f, 0.25f, 1.6f), si == 0 ? QColor(150, 150, 154) : QColor(138, 128, 112), MAT_LIMESTONE);
                mb.box(V3(0, -0.2f, 0.2f), V3(1.0f, 0.12f, 1.35f), QColor(104, 96, 84), MAT_LIMESTONE);
                for (int c = 0; c < 4; ++c)        // rachaduras sutis
                    mb.box(V3(rr.f(-0.9f, 0.9f), 0.252f, rr.f(-1.2f, 1.2f)), V3(rr.f(0.15f, 0.5f), 0.004f, 0.012f), QColor(40, 36, 32), MAT_COAL);
                QString nm = Q("degrau_%1_%2").arg(si + 1).arg(j);
                DynHandle d = w.addDynamic(nm, std::move(mb), pos, q);
                Collider* col = w.addBoxCollider(nm, Vec3(), V3(1.12f, 0.25f, 1.6f));
                w.attachCollider(col, d);
                st->dyn.push_back(d); st->col.push_back(col); st->basePos.push_back(pos); st->baseRots.push_back(q);
            }
            steps.push_back(st);
        }
        w.addUpdater([&w, steps](float dt, float t) {
            Vec3 p = w.playerPos();
            float dx = p.x() - PC.x(), dz = p.z() - PC.z();
            float r = std::sqrt(dx * dx + dz * dz);
            float th = std::atan2(dz, dx) * 57.2957795f;
            float u = (th - kAng0) / 360.f; u -= std::floor(u);
            float tT = (kTopY - p.y()) / kDrop;
            float tt = u + std::round(tT - u);
            for (auto& st : steps) {
                if (st->state == 0) {
                    if (r > 18.7f && r < 22.4f && tt >= st->tA - 0.002f && tt <= st->tB + 0.002f && std::fabs(p.y() - topAt(tt)) < 1.1f) {
                        st->state = 1; st->timer = 0.f;
                        w.state.toast(Q("O calcário podre estala e começa a esfarelar sob seus pés!"), QColor(255, 190, 120), 3.2f);
                        w.state.screenShake(0.03f, 0.6f);
                    }
                } else if (st->state == 1) {
                    st->timer += dt;
                    for (size_t i = 0; i < st->dyn.size(); ++i)
                        st->dyn[i]->pos = st->basePos[i] + Vec3(0, std::sin(t * 60.f + i) * 0.012f, 0);
                    if (st->timer > 0.9f) {
                        st->state = 2; st->vy = 0.f; st->timer = 0.f;
                        for (auto* c : st->col) c->enabled = false;
                        w.state.toast(Q("A cornija desaba! Destreza CD 14 para saltar para trás — ou agarre a corda de segurança."), QColor(255, 140, 110), 4.f);
                        w.state.screenShake(0.06f, 0.8f);
                    }
                } else if (st->state == 2) {
                    st->timer += dt; st->vy -= 9.8f * dt;
                    for (size_t i = 0; i < st->dyn.size(); ++i) {
                        st->dyn[i]->pos.setY(st->dyn[i]->pos.y() + st->vy * dt);
                        st->dyn[i]->setRotation(st->baseRots[i] * QQuaternion::fromAxisAndAngle(Vec3(0.4f + i * 0.3f, 0.2f, 1), st->timer * (40.f + i * 25.f)));
                    }
                    if (st->timer > 4.f) { for (auto* d : st->dyn) d->visible = false; st->state = 3; }
                }
            }
        });
        w.registerEvent(Q("reset_poco"), Q("Restaurar degraus e gelo (Poço / Corredor)"), Qt::Key_N, [&w, steps]() {
            for (auto& st : steps) {
                st->state = 0; st->timer = 0.f; st->vy = 0.f;
                for (size_t i = 0; i < st->dyn.size(); ++i) {
                    st->dyn[i]->visible = true; st->dyn[i]->pos = st->basePos[i]; st->dyn[i]->setRotation(st->baseRots[i]); st->col[i]->enabled = true;
                }
            }
            w.state.toast(Q("Degraus e placas de gelo restaurados."), QColor(200, 230, 255), 2.5f);
            w.fireEvent(Q("reset_gelo"));
        });
    }

    // ============================================================== 2.3 PONTE DA ESTALACTITE CAIDA
    {
        Vec3 a = lp(1.0f, 22.2f), b = lp(1.5f, 22.2f);       // pontas encostadas na parede
        float rA = 1.7f, rB = 1.15f;
        Vec3 apex = b + Vec3(0, -rB + 0.25f, 0), base = a + Vec3(0, -rA + 0.25f, 0);
        w.fill(Prim::cone(apex, base, rA, rB).smooth(0.8f).noise(0.35f).mat(MAT_LIMESTONE).named(Q("ponte_estalactite")));
        w.stalactite(Vec3(PC.x() + 6.f, -12.5f, PC.z() - 4.f), 1.6f, 1.8f, MAT_WET_LIMESTONE);   // toco de onde caiu
    }

    // ============================================================== 2.4 CAMARA DO ROPER GLACIAL (t 1,25)
    const Vec3 roperC(13.7f, -59.f, -204.6f);
    {
        w.carveChamber(Vec3(roperC.x(), -55.f, roperC.z()), V3(13.f, 18.f, 11.f), -59.f, MAT_WET_LIMESTONE, 0.75f, 2.f, Q("roper_camara"));
        tun(w, {lp(1.25f, 21.f), V3(19.f, -59.f, -201.5f), V3(15.f, -59.f, -203.f)}, 3.2f, MAT_WET_LIMESTONE, 0.4f);
        w.scatterStalagmites(Aabb(V3(1.f, -59.f, -216.f), V3(18.f, -52.f, -199.f)), 26, 41, 1.0f, 3.4f, 0.25f, 0.7f, MAT_WET_LIMESTONE);
        w.scatterStalactites(Aabb(V3(1.f, -56.f, -216.f), V3(26.f, -30.f, -193.f)), 46, 42, 2.0f, 7.5f, 0.3f, 0.9f, MAT_LIMESTONE);
    }

    // ============================================================== 2.5 NICHO DO ECO (t 2,75)
    const Vec3 nichoC(58.3f, -107.f, -162.9f);
    {
        float a = rad(angAt(2.75f));
        QQuaternion q = QQuaternion::fromAxisAndAngle(0, 1, 0, -deg(a));
        w.carve(Prim::box(Vec3(nichoC.x(), -105.5f, nichoC.z()), V3(3.4f, 1.5f, 2.0f), q, 0.5f).smooth(0.8f).noise(0.25f).mat(MAT_LIMESTONE).floorAt(-107.f).named(Q("nicho_eco")));
    }

    // ============================================================== 2.7 CORREDOR (geometria)
    {
        QQuaternion q = QQuaternion::fromAxisAndAngle(0, 1, 0, -2.73f);
        w.carve(Prim::box(V3(-3.f, -177.f, -181.f), V3(22.4f, 3.5f, 2.25f), q, 0.6f).smooth(0.5f).noise(0.22f).mat(MAT_HOT_BASALT).named(Q("corredor_termico")));
    }

    // ============================================================== PROPS ESTATICOS
    {
        MeshBuilder mb;
        using namespace Prefabs;
        {   // escada de corda partida na borda (2.1)
            Vec3 a1 = lp(0.0f, 17.5f); a1.setY(-19.0f);
            mb.cylinder(a1 + V3(-0.25f, -0.15f, 0.2f), a1 + V3(-0.25f, 0.45f, 0.2f), 0.07f, 6, QColor(100, 78, 52), MAT_ROTTEN_WOOD);
            mb.cylinder(a1 + V3(0.45f, -0.15f, -0.45f), a1 + V3(0.45f, 0.45f, -0.45f), 0.07f, 6, QColor(100, 78, 52), MAT_ROTTEN_WOOD);
            Vec3 top = lp(0.012f, 16.6f); top.setY(-19.0f);
            ropeLadder(mb, top, yawToAxis(0.012f) + 90.f, 9.5f);
            mb.tube(std::vector<Vec3>{a1 + V3(-0.25f, 0.3f, 0.2f), top + V3(0.28f, 0, 0.1f)}, 0.025f, 5, QColor(150, 125, 85), MAT_ROPE);
            bonesPile(mb, lp(0.04f, 20.f), 0.7f, 3);
            skull(mb, lp(0.05f, 19.6f), 40.f);
            totemOfBones(mb, lp(0.06f, 18.f), 1.8f, 5);
        }
        for (int ring = 0; ring < 3; ++ring) {          // franja de gelo na cupula
            float rr = ring == 0 ? 21.2f : (ring == 1 ? 17.5f : 11.5f);
            float yy = ring == 0 ? -17.3f : (ring == 1 ? -12.4f : -10.6f);
            int n = ring == 0 ? 30 : (ring == 1 ? 22 : 14);
            for (int i = 0; i < n; ++i) {
                float a0 = rad(i * 360.f / n), a1 = rad((i + 1) * 360.f / n);
                iceFringe(mb, V3(PC.x() + rr * std::cos(a0), yy, PC.z() + rr * std::sin(a0)), V3(PC.x() + rr * std::cos(a1), yy, PC.z() + rr * std::sin(a1)), 3, ring == 0 ? 2.2f : 3.4f, 100 + ring * 50 + i);
            }
        }
        for (int i = 0; i < 18; ++i) {                  // brilhos de gelo na parede alta
            float a = rad(rg.f(0, 360)), y = rg.f(-40.f, -20.f);
            Vec3 p(PC.x() + 21.4f * std::cos(a), y, PC.z() + 21.4f * std::sin(a));
            crystalCluster(mb, p, V3(-std::cos(a), 0.2f, -std::sin(a)), 0.5f, 3, 900 + i, QColor(150, 205, 255), true);
        }
        for (int i = 0; i < 9; ++i) bonesPile(mb, Vec3(roperC.x() + rg.f(-8, 9), -59.f, roperC.z() + rg.f(-6, 7)), rg.f(0.5f, 1.0f), 20 + i);
        skull(mb, Vec3(11.f, -59.f, -203.f), 120.f, 0.8f);
        goblin(mb, Vec3(15.f, -59.f, -200.f), 200.f, Pose::Lie);
        sack(mb, Vec3(15.9f, -59.f, -199.4f), 30.f, 0.7f);
        {   // nicho do eco: pedras soltas (barricada) e restos de acampamento
            float a = rad(angAt(2.75f));
            Vec3 rdir(std::cos(a), 0.f, std::sin(a)), tdir(-std::sin(a), 0.f, std::cos(a));
            Vec3 c = nichoC; c.setY(-107.f);
            for (int i = 0; i < 7; ++i) {
                Vec3 p = c - rdir * 2.4f + tdir * rg.f(-1.6f, 1.6f) + rdir * rg.f(-0.4f, 0.4f);
                mb.ellipsoid(Vec3(p.x(), -107.f + 0.25f, p.z()), V3(rg.f(0.35f, 0.7f), rg.f(0.22f, 0.4f), rg.f(0.35f, 0.6f)), QColor(116, 108, 98), MAT_LIMESTONE, 6);
            }
            for (int i = 0; i < 4; ++i) mb.cylinder(c + rdir * 1.2f + V3(-0.3f + i * 0.2f, 0.06f, 0.1f * (i % 2)), c + rdir * 1.2f + V3(0.3f - i * 0.15f, 0.1f, -0.2f + 0.1f * i), 0.06f, 5, QColor(28, 24, 22), MAT_COAL);   // restos de fogueira apagada
            sack(mb, c + tdir * 1.6f + rdir * 0.3f, 40.f, 0.8f);
            skull(mb, c - tdir * 1.4f + rdir * 1.4f, -30.f, 0.9f);
        }
        w.addStatic(std::move(mb));
    }
    {   // o Roper (estalagmite viva)
        MeshBuilder rm; roperProp(rm, Vec3(12.f, -59.f, -207.f), 25.f);
        w.addStatic(std::move(rm));
    }

    // ---- fundo do poco: entulho, ossos, esqueleto, corda partida ----
    {
        MeshBuilder mb;
        using namespace Prefabs;
        for (int i = 0; i < 42; ++i) {
            float a = rad(rg.f(0, 360)), d = std::sqrt(rg.f(0.f, 1.f)) * 20.f;
            Vec3 p(PC.x() + d * std::cos(a), -178.f, PC.z() + d * std::sin(a));
            if (p.x() < 24.f && std::fabs(p.z() + 180.f) < 3.f) continue;     // boca do corredor livre
            float s = rg.f(0.35f, 1.5f);
            mb.ellipsoid(p + V3(0, s * 0.3f, 0), V3(s, s * rg.f(0.4f, 0.8f), s * rg.f(0.7f, 1.1f)), QColor(34 + rg.i(0, 14), 32 + rg.i(0, 10), 38 + rg.i(0, 10)), MAT_OBSIDIAN, 6);
        }
        for (int i = 0; i < 20; ++i) {
            float a = rad(rg.f(0, 360)), d = std::sqrt(rg.f(0.f, 1.f)) * 19.f;
            Vec3 p(PC.x() + d * std::cos(a), -178.f, PC.z() + d * std::sin(a));
            if (p.x() < 24.f && std::fabs(p.z() + 180.f) < 3.f) continue;
            float k = rg.f();
            if (k < 0.5f) bonesPile(mb, p, rg.f(0.5f, 1.3f), 60 + i);
            else if (k < 0.8f) skull(mb, p, rg.f(0, 360), 1.f);
            else ribcage(mb, p, rg.f(0, 360), 1.f);
        }
        for (int i = 0; i < 6; ++i) {                    // lajes quebradas que cairam das cornijas
            Vec3 p(PC.x() + rg.f(-14, 14), -177.8f, PC.z() + rg.f(-14, 14));
            mb.push(); mb.translate(p); mb.rotateY(rg.f(0, 360)); mb.rotate(rg.f(-12, 12), V3(1, 0, 0));
            mb.box(V3(0, 0.15f, 0), V3(1.1f, 0.2f, 0.8f), QColor(130, 120, 105), MAT_LIMESTONE);
            mb.pop();
        }
        HumanoidParams hp; hp.pose = Pose::Lie; hp.skin = QColor(215, 205, 182); hp.cloth = QColor(70, 56, 48); hp.cloth2 = QColor(50, 44, 40); hp.height = 1.8f;
        humanoid(mb, V3(48.f, -178.f, -186.f), 160.f, hp);
        skull(mb, V3(46.8f, -177.95f, -185.4f), 200.f, 1.2f);
        ribcage(mb, V3(48.f, -178.f, -186.4f), 160.f, 0.9f);
        std::vector<Vec3> rope = MeshBuilder::catenary(V3(47.0f, -177.95f, -187.8f), V3(52.5f, -177.95f, -184.f), 0.0f, 10);
        for (auto& p : rope) p.setY(-177.93f + 0.02f * std::sin(p.x() * 3.f));
        mb.tube(rope, 0.04f, 5, QColor(140, 118, 80), MAT_ROPE);
        sack(mb, V3(49.5f, -178.f, -187.2f), 20.f, 0.8f);
        w.addStatic(std::move(mb));
    }

    // ---- Lamina da Brasa Adormecida (dinamico: some ao ser pega) ----
    DynHandle bladeDyn;
    {
        MeshBuilder bm;
        bm.push(); bm.rotateY(75.f);
        bm.box(V3(0, 0.03f, 0), V3(0.035f, 0.012f, 0.55f), QColor(70, 60, 58), MAT_IRON);
        bm.box(V3(0, 0.045f, 0), V3(0.012f, 0.004f, 0.5f), QColor(255, 110, 40), MAT_EMISSIVE);
        bm.box(V3(0, 0.03f, 0.58f), V3(0.16f, 0.015f, 0.025f), QColor(110, 80, 40), MAT_BRONZE);
        bm.cylinder(V3(0, 0.03f, 0.6f), V3(0, 0.03f, 0.86f), 0.022f, 6, QColor(60, 40, 30), MAT_WOOD);
        bm.sphere(V3(0, 0.03f, 0.89f), 0.04f, QColor(255, 120, 40), MAT_EMISSIVE, 6);
        bm.pop();
        bladeDyn = w.addDynamic(Q("lamina_brasa"), std::move(bm), V3(47.4f, -178.f, -185.1f));
    }
    Light* bladeLight = w.addLight(V3(47.4f, -177.5f, -185.1f), V3(1.0f, 0.38f, 0.1f), 2.6f, 9.f, LightAnim::Pulse, Q("lamina"));
    bladeLight->animFreq = 0.35f; bladeLight->animDepth = 0.4f;
    w.addInteractable(Q("lamina_brasa"), V3(47.4f, -177.7f, -185.1f), 1.1f, Q("Pegar a Lâmina da Brasa Adormecida"), [&w, bladeDyn, bladeLight]() {
        if (w.state.hasItem(Q("Lâmina da Brasa Adormecida"))) return;
        w.state.addItem(Q("Lâmina da Brasa Adormecida"));
        bladeDyn->visible = false; bladeLight->enabled = false;
        w.state.toast(Q("A lâmina está morna na sua mão. Ela parece apontar para baixo, para o calor distante."), QColor(255, 170, 90), 5.f);
        w.state.flash(QColor(255, 140, 60), 0.35f);
        w.addUpdater([&w](float, float) {
            float d = (w.playerPos() - kHeart).length();
            float v = clampf(1.f - (d - 15.f) / (345.f - 15.f), 0.f, 1.f);
            QColor c(255, (int)(150 - 100 * v), (int)(60 - 40 * v));
            w.state.setMeter(Q("Lâmina da Brasa"), v, c);
        });
    });

    // ============================================================== CORREDOR TERMICO: agua, gelo, vapor, luzes
    std::vector<std::shared_ptr<IceSlab>> ice;
    {
        MeshBuilder wm; wm.setFlow(-1.f, 0.f);
        wm.boxMinMax(V3(-24.6f, -180.5f, -184.2f), V3(18.8f, -178.6f, -177.8f), QColor(255, 120, 40, 185), MAT_BOILING_WATER);
        w.addStatic(std::move(wm));
        const int N = 8; const float x0 = 19.2f, x1 = -24.2f, L = (x0 - x1) / N;
        for (int i = 0; i < N; ++i) {
            auto s = std::make_shared<IceSlab>();
            s->thin = (i == 2 || i == 5);
            float cx = x0 - (i + 0.5f) * L;
            float cz = -180.f - 2.f * (18.f - cx) / 42.f;
            s->cx = cx; s->cz = cz; s->hx = L * 0.5f - 0.05f; s->hz = 2.05f;
            MeshBuilder im;
            float th = s->thin ? 0.11f : 0.21f;
            im.box(V3(0, 0.2f - th, 0), V3(s->hx, th, s->hz), s->thin ? QColor(205, 232, 250, 105) : QColor(165, 205, 232, 150), MAT_CLEAR_ICE);
            Rnd rr(300 + i);
            int nc = s->thin ? 9 : 3;
            for (int c = 0; c < nc; ++c)
                im.box(V3(rr.f(-s->hx * 0.8f, s->hx * 0.8f), 0.205f, rr.f(-1.6f, 1.6f)), V3(rr.f(0.3f, s->thin ? 1.6f : 0.8f), 0.003f, 0.012f), QColor(245, 252, 255, 220), MAT_CLEAR_ICE);
            s->base = V3(cx, -178.2f, cz);
            s->dyn = w.addDynamic(Q("gelo_%1").arg(i + 1), std::move(im), s->base, QQuaternion::fromAxisAndAngle(0, 1, 0, -2.73f));
            s->col = w.addBoxCollider(Q("gelo_%1").arg(i + 1), Vec3(), V3(s->hx, 0.2f, s->hz));
            w.attachCollider(s->col, s->dyn);
            ice.push_back(s);
        }
        w.addUpdater([&w, ice](float dt, float t) {
            Vec3 p = w.playerPos();
            for (auto& s : ice) {
                if (s->state == 0 && s->thin) {
                    if (std::fabs(p.x() - s->cx) < s->hx && std::fabs(p.z() - s->cz) < s->hz && p.y() > -178.4f && p.y() < -177.3f) {
                        s->state = 1; s->timer = 0.f;
                        w.state.toast(Q("O gelo estala! Rachaduras se abrem sob seus pés… (Acrobacia CD 13)"), QColor(190, 225, 255), 3.f);
                        w.state.screenShake(0.04f, 0.5f);
                    }
                } else if (s->state == 1) {
                    s->timer += dt;
                    s->dyn->pos = s->base + Vec3(0, std::sin(t * 70.f) * 0.01f, 0);
                    if (s->timer > 0.8f) {
                        s->state = 2; s->vy = 0.f; s->timer = 0.f; s->col->enabled = false;
                        w.state.toast(Q("O gelo se parte! 2d6 cortante + frio na queda e 4d10 de fogo ao mergulhar na água escaldante!"), QColor(255, 120, 90), 4.5f);
                        w.state.screenShake(0.07f, 0.7f);
                    }
                } else if (s->state == 2) {
                    s->timer += dt; s->vy -= 6.f * dt;
                    s->dyn->pos.setY(std::max(-180.2f, s->dyn->pos.y() + s->vy * dt));
                    s->dyn->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, -2.73f) * QQuaternion::fromAxisAndAngle(1, 0, 0.3f, std::min(s->timer, 1.f) * 14.f));
                    if (s->timer > 3.f) s->state = 3;
                }
            }
        });
        w.registerEvent(Q("reset_gelo"), Q("Restaurar o gelo fino do corredor"), 0, [ice]() {
            for (auto& s : ice) {
                s->state = 0; s->timer = 0.f; s->vy = 0.f; s->col->enabled = true; s->dyn->pos = s->base;
                s->dyn->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, -2.73f));
            }
        });
        for (int i = 0; i < 5; ++i) {                   // luz vermelha sob o gelo
            float x = 14.f - i * 8.7f;
            Light* l = w.addLight(V3(x, -179.6f, -180.f - 2.f * (18.f - x) / 42.f), V3(1.0f, 0.24f, 0.06f), 7.5f, 10.5f, LightAnim::Pulse, Q("brilho do corredor"));
            l->animFreq = 0.45f + 0.07f * i; l->animDepth = 0.35f; l->phase = i * 0.9f;
        }
        for (int i = 0; i < 6; ++i)
            w.addEmitter(ParticlePreset::Steam, V3(16.f - i * 7.2f, -178.f, -181.f - 0.3f * i), V3(2.2f, 0.1f, 2.0f), 11.f, Q("vapor do corredor"));
        for (int i = 1; i < 8; ++i)
            w.addEmitter(ParticlePreset::Steam, V3(19.2f - i * 5.425f, -178.f, -181.f - 0.1f * i), V3(0.2f, 0.05f, 1.8f), 9.f, Q("vapor das frestas"));
        ZoneEffects hz; hz.hazard = true; hz.temperature = 90.f;
        hz.hazardText = Q("Água vulcânica escaldante: 4d10 de dano de fogo (e 2d6 cortante + frio pela queda do gelo)!");
        w.addZone(Q("agua_escaldante"), ZoneShape::makeBox(V3(-24.f, -181.f, -184.5f), V3(18.6f, -178.55f, -177.5f)), hz, 6);
    }

    // ============================================================== ZONAS, LUZES, PARTICULAS DO POCO
    {
        ZoneEffects fx;
        fx.areaName = Q("Camada 2 · O Poço do Eco"); fx.subtitle = kSess;
        fx.temperature = -20.f; fx.fogColor = V3(0.004f, 0.006f, 0.011f); fx.fogDensity = 0.0105f;
        fx.ambientColor = V3(0.012f, 0.016f, 0.026f); fx.ambientGround = V3(0.004f, 0.005f, 0.007f); fx.ambientStrength = 1.f;
        w.addZone(Q("poco"), ZoneShape::makeCylinder(PC, 23.f, -179.f, -8.f), fx, 3);
        ZoneEffects fw; fw.wind = true; fw.windPeriod = 20.f; fw.windDuration = 2.f;
        w.addZone(Q("poco_vento"), ZoneShape::makeCylinder(PC, 22.6f, -140.f, -80.f), fw, 4);
        ZoneEffects fb; fb.areaName = Q("2.6 · O Fundo do Poço"); fb.subtitle = Q("Entulho, ossos e obsidiana"); fb.temperature = -17.f;
        fb.fogColor = V3(0.006f, 0.005f, 0.008f); fb.fogDensity = 0.016f;
        w.addZone(Q("fundo_poco"), ZoneShape::makeBox(V3(17.f, -179.f, -203.f), V3(63.f, -163.f, -157.f)), fb, 5);
        ZoneEffects fr; fr.areaName = Q("2.4 · Câmara do Roper Glacial"); fr.subtitle = Q("Caverna lateral · estalagmites e pingentes"); fr.temperature = -21.f;
        fr.fogColor = V3(0.006f, 0.009f, 0.014f); fr.fogDensity = 0.02f;
        w.addZone(Q("roper"), ZoneShape::makeBox(V3(0.f, -60.f, -217.f), V3(27.f, -34.f, -192.f)), fr, 5);
        ZoneEffects fn; fn.areaName = Q("2.5 · O Nicho do Eco"); fn.subtitle = Q("Único descanso curto razoável do poço"); fn.temperature = -12.f;
        fn.fogDensity = 0.03f;
        w.addZone(Q("nicho"), ZoneShape::makeBoxCenter(V3(nichoC.x(), -105.5f, nichoC.z()), V3(4.6f, 2.2f, 4.6f)), fn, 5);
        ZoneEffects ft; ft.areaName = Q("2.7 · Corredor do Contraste Térmico"); ft.subtitle = Q("Armadilha: gelo mágico sobre água escaldante"); ft.temperature = 34.f;
        ft.fogColor = V3(0.05f, 0.012f, 0.006f); ft.fogDensity = 0.024f; ft.ambientColor = V3(0.07f, 0.022f, 0.012f); ft.ambientGround = V3(0.05f, 0.012f, 0.006f);
        w.addZone(Q("corredor_termico"), ZoneShape::makeBox(V3(-25.f, -181.f, -185.f), V3(18.5f, -172.f, -177.f)), ft, 5);

        w.addLight(V3(PC.x(), -12.f, PC.z()), V3(0.35f, 0.55f, 0.9f), 1.6f, 34.f, LightAnim::Pulse, Q("gelo da cupula"))->animFreq = 0.08f;
        w.addLight(V3(PC.x() - 14.f, -22.f, PC.z() + 10.f), V3(0.4f, 0.6f, 1.f), 1.2f, 18.f, LightAnim::Pulse, Q("brilho de gelo"))->animFreq = 0.13f;
        w.addLight(V3(PC.x() + 12.f, -28.f, PC.z() - 12.f), V3(0.4f, 0.6f, 1.f), 1.0f, 16.f, LightAnim::Pulse, Q("brilho de gelo"))->animFreq = 0.1f;
        w.addLight(V3(24.f, -176.5f, -180.f), V3(1.f, 0.25f, 0.08f), 2.5f, 12.f, LightAnim::Pulse, Q("calor distante"))->animFreq = 0.3f;
        for (int i = 0; i < 6; ++i)
            w.addEmitter(ParticlePreset::Dust, V3(PC.x(), -30.f - i * 26.f, PC.z()), V3(16.f, 11.f, 16.f), 5.f, Q("poeira do poço"));

        // olho do Roper: abre quando a luz chega perto
        MeshBuilder em;
        em.ellipsoid(V3(0, 0, 0), V3(0.22f, 0.15f, 0.1f), QColor(255, 220, 90), MAT_EMISSIVE, 8);
        em.box(V3(0, 0, -0.095f), V3(0.02f, 0.14f, 0.012f), QColor(20, 12, 8), MAT_COAL);
        DynHandle eye = w.addDynamic(Q("olho_roper"), std::move(em), V3(12.f, -56.75f, -207.5f), QQuaternion::fromAxisAndAngle(0, 1, 0, 25.f));
        eye->visible = false;
        Light* el = w.addLight(V3(12.f, -56.7f, -207.4f), V3(1.f, 0.85f, 0.3f), 2.f, 6.f, LightAnim::Pulse, Q("olho do roper"));
        el->enabled = false;
        w.addUpdater([&w, eye, el](float, float) {
            Vec3 e(12.f, -56.75f, -207.5f);
            bool open = (w.playerEye() - e).length() < 8.f;
            if (open && !eye->visible) w.state.toast(Q("Uma das 'estalagmites' abre um olho amarelo…"), QColor(255, 220, 110), 3.f);
            eye->visible = open; el->enabled = open;
        });
    }

    // ============================================================== EVENTO U (rajada)
    w.registerEvent(Q("gust"), Q("Rajada de vento no Poço"), Qt::Key_U, [&w]() {
        w.state.screenShake(0.05f, 1.2f);
        if (w.state.torchLit) w.state.extinguishTorch(Q("Uma rajada ascendente apaga sua tocha! (Destreza CD 12 com escudo ou capa)"));
        else w.state.toast(Q("Uma rajada uiva pelo poço (a tocha já estava apagada)."), QColor(170, 200, 255), 3.f);
    });

    // ============================================================== POIs
    w.addPoi(V3(PC.x(), -90.f, PC.z()), PoiCategory::Area, Q("2 · O Poço do Eco (visão geral)"),
        Q("Abismo cilíndrico colossal: centro (40, ·, −180), raio 22 m, do fundo (y −178) à cúpula (y −12) — 166 m de vertical. Calcário úmido com franja de gelo no alto; nos últimos 25 m a rocha vira obsidiana negra.\n\n"
          "O que os jogadores percebem: escuridão absoluta (só a tocha), frio de −20 °C, o eco dos próprios passos voltando segundos depois, um zumbido psíquico baixo e a sensação de vertigem. A tocha só mostra ~9 m: não se vê o fundo nem o teto.\n\n"
          "Mecânicas: temperatura −20 °C (descanso longo nas cornijas = exposto a predadores verticais), vento ascendente entre y −80 e −140 (apaga tochas), queda 1d6 por 3 m (máx. 20d6). Corda de segurança e Escalada ajudam.\n\n"
          "Tecla [U] = rajada de vento imediata. Tecla [N] = restaurar degraus e gelo."), kSess, kGrp);
    w.addPoi(V3(27.f, -18.0f, -167.f), PoiCategory::Area, Q("2.1 · A Borda"),
        Q("O túnel da Fronteira dos Ossos chega à parede do poço em θ ≈ 133°, y −19. Patamar largo (5 m) nos primeiros ~10 m antes de a cornija estreitar para 3 m.\n\n"
          "Perceba: o ar sobe do abismo e o som dos passos 'cai' para dentro. Uma pedra jogada demora ~3 s para ecoar de volta. Últimos vestígios goblin: ossos, um totem de fetiches e arranhões de proteção.\n\n"
          "Dica ao Mestre: aqui é o momento certo para amarrar cordas e anunciar o frio (−20 °C): quem estiver sem roupas adequadas sofre 1 nível de exaustão a cada descanso longo."), kSess, kGrp);
    w.addPoi(V3(28.f, -22.f, -168.5f), PoiCategory::Lore, Q("2.1 · Escada de corda partida"),
        Q("Restos de uma escada de corda presa a estacas de madeira podre, pendendo 9 m no vazio, terminando em pontas esfiapadas e degraus faltando. Alguém tentou descer por aqui há muito tempo, sem usar a cornija.\n\n"
          "Investigação CD 12: a corda rompeu sob o peso, não há cortes — o dono caiu, e está no fundo, com a Lâmina da Brasa.\n"
          "Usar como ancoragem: as estacas aguentam um humano (Atletismo CD 10 para rapel), mas a corda velha cede com 1 em 1d6 a cada uso."), kSess, kGrp);
    w.addPoi(lp(0.30f, 20.5f, 1.3f), PoiCategory::Area, Q("2.2 · As Cornijas em Espiral"),
        Q("Rampa helicoidal natural colada à parede: começa em θ 133°, y −19 e desce 32 m por volta (≈ 14°), largura 3 m (r 19–22), até o fundo (≈ 4,97 voltas). Altura na volta t: y = −19 − 32·t; ângulo θ = 133° + 360°·t. Cerca de 645 m de caminho.\n\n"
          "Perceba: a beirada interna é lisa e molhada; sem corrimão. A cada volta você está exatamente acima do trecho anterior — o som cai 32 m.\n"
          "Regras: correr ou lutar na beirada: Destreza CD 10 em cada empurrão para não cair. Queda de uma volta = 32 m (10d6)."), kSess, kGrp);
    w.addPoi(lp(0.975f, 21.4f, 1.3f), PoiCategory::Trap, Q("2.2 · Passo do Desespero (t 0,90–1,05)"),
        Q("A cornija estreita para 1,0 m por ~19 m de caminho, com o vazio de um lado e a parede lisa do outro. Cabe uma pessoa Média de lado; criaturas Grandes não passam.\n\n"
          "Regras: Acrobacia CD 10 se estiver com armadura pesada, carga extra, vento ou correndo; falha = Destreza CD 12 para se agarrar na beirada, senão cai 32 m (10d6). Rajadas de vento (tecla U) pioram: CD 15.\n"
          "Tática: o Roper (volta seguinte, t 1,25) e a Ponte da Estalactite começam logo depois — um grupo desequilibrado aqui é alvo fácil."), kSess, kGrp);
    {
        const float tm[3] = {0.575f, 1.725f, 3.325f};
        const char* txt[3] = {
            "Primeiro Degrau Esfarelento (t 0,55–0,60, y ≈ −50, ~6 m de calcário podre).",
            "Segundo Degrau Esfarelento (t 1,70–1,75, y ≈ −75, depois da câmara do Roper).",
            "Terceiro Degrau Esfarelento (t 3,30–3,35, y ≈ −125, dentro da zona de vento)."};
        for (int i = 0; i < 3; ++i)
            w.addPoi(lp(tm[i], 20.6f, 1.2f), PoiCategory::Trap, Q("2.2 · Degraus Esfarelentos #%1").arg(i + 1),
                QString::fromUtf8(txt[i]) + Q("\n\nÉ uma laje de calcário podre (colisor) onde a cornija é interrompida: parece um pouco mais clara e rachada. Ao pisar, estala por ~0,9 s e depois desaba.\n"
                  "Percepção CD 14 para notar as rachaduras. Quando estala: Destreza CD 14 para pular para trás (ou para frente, se já estiver no meio). Falha = queda até a volta de baixo: 32 m, 10d6 contundente — ou agarrar a corda de segurança (Destreza CD 15, reação) se o grupo tiver amarrado uma.\n"
                  "Depois de desabada, a laje deixa um vão de ~6,4 m: exige cordas, Pulo Longo ou voltar pela Ponte. [N] restaura a laje para repetir a cena."), kSess, kGrp);
    }
    w.addPoi(lp(2.51f, 20.5f, 1.3f), PoiCategory::Trap, Q("2.2 · O Vão (t 2,50–2,52)"),
        Q("Um vão natural de 1,3 m (≈ 4 ft) interrompe a cornija em y ≈ −99, com 0,3 m de degrau para baixo do outro lado. Atrás dele, a rampa continua normalmente.\n\n"
          "Regras: salto em distância de 4 ft — Atletismo CD 8 com corrida (CD 12 sem corrida); falha por 5+ = cai 32 m (10d6). O vão fica na zona de vento (rajada = desvantagem). Dá para jogar uma tábua ou amarrar uma corda."), kSess, kGrp);
    w.addPoi(V3(PC.x(), -57.8f, PC.z()), PoiCategory::Area, Q("2.3 · A Ponte da Estalactite Caída"),
        Q("Uma estalactite de ~40 m (raio 1,7 → 1,15 m) caiu da cúpula e ficou atravessada no poço, passando pelo centro: liga a cornija em t 1,0 (θ 133°, y −51) à cornija em t 1,5 (θ 313°, y −67). Inclinação ~21°. Atalho dramático sobre o vazio (pula ~0,5 volta).\n\n"
          "Regras: Acrobacia CD 10 (CD 15 com vento). Falha = Destreza CD 12 para se agarrar ao tronco; senão cai (até 110 m se estiver no meio: 20d6 máx.).\n"
          "Tática: ponto perfeito para o Roper atacar com tentáculos (alcance 15 m): ele agarra alguém no meio do vão e iça para a escuridão. Os outros ficam presos no tronco, sem cobertura."), kSess, kGrp);
    w.addPoi(roperC + V3(0, 5.f, 0), PoiCategory::Area, Q("2.4 · A Câmara do Roper Glacial"),
        Q("Caverna lateral saindo da cornija em t 1,25 (θ 223°, y −59). Centro (13,7, −59, −204,6), ~26 × 22 m, 22 m de altura: alta e úmida. Teto coberto de pingentes de gelo (até 7 m), chão de estalagmites.\n\n"
          "Perceba: gotejar constante, ossos pequenos (goblins) espalhados, cheiro de mofo e carniça. Nenhuma luz, nenhum ruído.\n"
          "Descanso: péssimo lugar — o Roper caça aqui. Evite combate na cornija: o Roper também alcança a abertura (alcance 15 m)."), kSess, kGrp);
    w.addPoi(V3(12.f, -56.0f, -207.f), PoiCategory::Creature, Q("2.4 · O Roper Glacial (camuflado)"),
        Q("Predador de caverna sem ligação com a trama. Se camufla entre as estalagmites em (12, −59, −207); só abre o olho quando a luz chega a ≤ 8 m (o olho amarelo aparece no mundo 3D).\n\n"
          "Percepção CD 16 para distinguir de uma estalagmite. Roper padrão do Monster Manual (CR 5; no nível 7 use 2 Roper ou +1 capanga) — sugestão de variante glacial: imune a frio, vulnerável a fogo.\n"
          "Tática: 6 tentáculos (alcance 15 m; Força CD 15 ou fica agarrado e é puxado 7,5 m); iça heróis para a escuridão do teto. Fraco com luz forte (tocha na cara: desvantagem) e com fogo. Combate vertical: o grupo precisa de cordas. Ele não persegue para fora da caverna."), kSess, kGrp);
    w.addPoi(V3(15.f, -57.6f, -200.2f), PoiCategory::Treasure, Q("2.4 · Restos do batedor goblin"),
        Q("Goblin magro de olhos arregalados, com a bolsa de couro ainda no cinto: 2 Poções de Escalada (Climbing) e 45 po.\n"
          "Percepção/Investigação CD 12: veio sozinho; suas pegadas voltam para a cornija. A flecha em seu pescoço não é goblin (mais pesada): outro grupo já passou aqui.\n"
          "Local: (15, −59, −200)."), kSess, kGrp);
    w.addPoi(V3(nichoC.x(), -105.8f, nichoC.z()), PoiCategory::Rest, Q("2.5 · O Nicho do Eco (descanso curto)"),
        Q("Alcova de 6 × 4 m, 3 m de altura, saindo da cornija em t 2,75 (θ 43°, y −107). Centro (58,3, −107, −162,9). Único lugar razoável para um descanso curto no poço.\n\n"
          "Perceba: os sons ficam mais curtos, o vento some; há restos de uma fogueira antiga, uma bolsa vazia e um crânio humano.\n"
          "Regras: dá para barricar com pedras soltas (10 min, Força CD 12 para empilhar) e dormir um descanso curto em segurança. Descanso longo aqui: 1 encontro vertical (Roper ou aranhas de gelo) a critério do Mestre. Temperatura −12 °C (abrigo do vento)."), kSess, kGrp);
    w.addPoi(V3(PC.x() - 6.f, -110.f, PC.z() + 6.f), PoiCategory::Trap, Q("2.2 · Correntes de Vento (y −80 a −140)"),
        Q("Entre y −80 e −140, rajadas ascendentes a cada ~20 s (duram 2 s) apagam tochas desprotegidas (Shadowdark). No jogo o motor apaga a tocha automaticamente.\n\n"
          "Regras: escudo, capa ou a mão em concha protegem — Destreza CD 12. Sem luz, o grupo fica em escuridão total: visão no escuro [V] só vê 18 m em cinza. Sempre leve uma tocha reserva (5 no inventário, [R] reacende).\n"
          "No Passo do Desespero e no Vão, a rajada exige novo teste de equilíbrio. Tecla [U] dispara uma rajada imediata."), kSess, kGrp);
    w.addPoi(V3(PC.x(), -176.5f, PC.z()), PoiCategory::Area, Q("2.6 · O Fundo do Poço"),
        Q("Chão em y −178: entulho de rocha, lajes quebradas, ossos de tudo o que caiu (goblins, cabras, um explorador). A rocha muda de calcário para obsidiana negra; o piso vira cinza.\n\n"
          "Perceba: silêncio absoluto e um zumbido psíquico distante; o ar morno do corredor a oeste cheira a enxofre.\n"
          "Sai pelo oeste (θ 180°) para o Corredor do Contraste Térmico. Temperatura −17 °C."), kSess, kGrp);
    w.addPoi(V3(47.4f, -177.4f, -185.1f), PoiCategory::Treasure, Q("2.6 · Lâmina da Brasa Adormecida (arma +1)"),
        Q("Arma mágica +1, emite calor fraco e brilha e esquenta mais à medida que o grupo desce em direção ao Coração do Dragão — uma bússola narrativa. No programa: depois de pegá-la [E], uma barra 'Lâmina da Brasa' no HUD cresce com a proximidade do Coração em (100, −287, −505).\n\n"
          "Mecânica sugerida: +1d4 de fogo em um acerto por descanso longo; com ela embainhada, o portador resiste ao frio (−20 °C não causa exaustão por 1 h); ao chegar a 1,0 no medidor, esquenta a ponto de queimar mãos desprotegidas (1d4).\n"
          "Local: (48, −178, −186), nas mãos do Esqueleto Esquecido."), kSess, kGrp);
    w.addPoi(V3(48.f, -177.2f, -186.f), PoiCategory::Lore, Q("2.6 · O Esqueleto Esquecido"),
        Q("Um aventureiro antigo que caiu: roupas apodrecidas, mochila vazia, corda partida a 2 m. As pernas estão quebradas em vários pontos; o crânio está virado para o corredor oeste.\n\n"
          "Investigação CD 13: não morreu da queda — arrastou-se 4 m até a parede e morreu de frio com a lâmina ainda quente. História livre: pode ter sido um batedor de um grupo anterior que nunca voltou."), kSess, kGrp);
    w.addPoi(V3(0.f, -177.0f, -181.f), PoiCategory::Trap, Q("2.7 · O Corredor do Contraste Térmico"),
        Q("De (18, −178, −180) a (−24, −178, −182): 42 m de comprimento, 4,5 m de largura, 4,5 m de altura. Inundado por água vulcânica escaldante (superfície y −178,6, fundo y −180,5), coberta por uma camada fina de gelo mágico (topo y −178,0) criada pelo feitiço do Alhoon.\n\n"
          "Perceba: vapor saindo pelas bordas, brilho avermelhado vindo de baixo do gelo, calor de forno no rosto (34 °C) logo após −17 °C; o contraste estala o gelo ao longe.\n"
          "8 placas de gelo de ~5 m; as placas 3 e 6 são FINAS e racham sob peso.\n"
          "Regras: Acrobacia CD 13 por placa (desvantagem se correr). Falha: o gelo quebra, 2d6 cortante + frio na queda e 4d10 de fogo ao mergulhar (zona de perigo; o motor devolve o grupo ao último chão seguro).\n"
          "Soluções: Detectar Magia (aura do Alhoon), Resistência a Fogo, congelar a água com magia de frio, saltar as placas finas (3 m: Atletismo CD 10) ou passar rente à parede."), kSess, kGrp);
    {
        float xc[2] = {5.2f, -11.1f};
        for (int i = 0; i < 2; ++i)
            w.addPoi(V3(xc[i], -177.0f, -180.5f), PoiCategory::Trap, Q("2.7 · Placa de gelo fina (placa %1)").arg(i == 0 ? 3 : 6),
                Q("Placa fina (~0,2 m): translúcida demais, com rachaduras brancas. Percepção CD 14 revela o aspecto frágil; a luz da tocha encostada mostra a água vermelha por baixo.\n"
                  "Ao receber peso racha (~0,8 s de aviso) e cai na água escaldante: 2d6 cortante + frio e 4d10 de fogo. Dá para saltar de uma placa grossa para a seguinte ignorando a fina. [N] restaura."), kSess, kGrp);
    }

    // ============================================================== PONTOS DE VISTA
    w.addViewpoint(Q("2.1 A Borda"), V3(27.f, -17.35f, -167.f), -45.f, -14.f, false, kGrp);
    w.addViewpoint(Q("2.1 Escada de corda partida"), V3(29.4f, -17.35f, -168.6f), 10.f, -50.f, false, kGrp);
    w.addViewpoint(Q("2.2 As Cornijas em Espiral"), lp(0.30f, 20.5f, 1.65f), yawAlong(0.30f), -8.f, false, kGrp);
    w.addViewpoint(Q("2.2 Passo do Desespero"), lp(0.86f, 20.2f, 1.65f), yawAlong(0.86f), -10.f, false, kGrp);
    w.addViewpoint(Q("2.2 Degraus Esfarelentos (1º)"), lp(0.50f, 20.4f, 1.65f), yawAlong(0.50f), -12.f, false, kGrp);
    w.addViewpoint(Q("2.2 O Vão (t 2,50)"), lp(2.487f, 20.4f, 1.65f), yawAlong(2.487f), -27.f, false, kGrp);
    w.addViewpoint(Q("2.3 Ponte da Estalactite Caída"), lp(0.99f, 21.f, 1.65f), yawToward(lp(1.0f, 21.f), lp(1.5f, 21.f)), -10.f, false, kGrp);
    w.addViewpoint(Q("2.4 Câmara do Roper Glacial"), lp(1.25f, 20.f, 1.65f), yawToward(lp(1.25f, 20.f), roperC), 6.f, false, kGrp);
    w.addViewpoint(Q("2.5 Nicho do Eco"), lp(2.75f, 20.5f, 1.65f), yawToward(lp(2.75f, 20.5f), nichoC), -2.f, false, kGrp);
    w.addViewpoint(Q("2.6 Fundo do Poço (Esqueleto)"), V3(42.f, -176.35f, -179.f), yawToward(V3(42.f, 0, -179.f), V3(48.f, 0, -186.f)), -8.f, false, kGrp);
    w.addViewpoint(Q("2.7 Corredor do Contraste Térmico"), V3(21.5f, -176.35f, -180.f), 90.f, -5.f, false, kGrp);
    w.addViewpoint(Q("2.7 Placas de gelo (de perto)"), V3(14.f, -176.35f, -180.5f), 90.f, -24.f, false, kGrp);
    w.addViewpoint(Q("Voo · Escala do Poço (olhando para cima)"), V3(32.f, -165.f, -172.f), 130.f, 78.f, true, kGrp);
    w.addViewpoint(Q("Voo · Escala do Poço (olhando para baixo)"), V3(40.f, -40.f, -180.f), 0.f, -82.f, true, kGrp);
    w.addViewpoint(Q("Voo · Meio do Poço (parede e espiral)"), V3(44.f, -90.f, -176.f), 150.f, -4.f, true, kGrp);

    // ============================================================== ROTA
    w.addRoutePoint(Q("Borda do Poço"), V3(27.f, -18.4f, -167.f));
    const float tv[] = {0.12f, 0.30f, 0.575f, 0.80f, 0.975f, 1.25f, 1.50f, 1.725f, 2.0f, 2.49f, 2.75f, 3.0f, 3.325f, 3.8f, 4.4f, 4.85f};
    const char* rn[] = {"Cornija t 0,12", "Cornija t 0,30", "Degrau Esfarelento 1", "Cornija t 0,80", "Passo do Desespero", "Entrada da Câmara do Roper", "Ponte da Estalactite (extremo)",
                        "Degrau Esfarelento 2", "Cornija t 2,00", "Antes do Vão", "Nicho do Eco", "Cornija t 3,00", "Degrau Esfarelento 3", "Cornija t 3,80", "Cornija t 4,40", "Cornija t 4,85"};
    for (int i = 0; i < 16; ++i)
        w.addRoutePoint(QString::fromUtf8(rn[i]), lp(tv[i], i == 4 ? 21.5f : 20.4f, 0.6f));
    w.addRoutePoint(Q("Fundo do Poço"), V3(40.f, -177.4f, -180.f));
    w.addRoutePoint(Q("Boca do Corredor Térmico"), V3(18.f, -177.4f, -180.f));
    w.addRoutePoint(Q("Saída do Corredor Térmico"), V3(-24.f, -177.4f, -182.f));
}
