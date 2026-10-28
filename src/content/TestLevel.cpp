// Sala de Testes (x ~ +600): exercita TODAS as funcionalidades do motor.
// Serve tambem de exemplo de uso da API de World para os autores de camadas.
// Convencao dos tuneis: carveTunnel recebe pontos no EIXO; aqui o helper tun() recebe pontos no CHAO
// e sobe o eixo em raio*floorFrac para que o chao fique exatamente nos pontos dados.
#include "Dungeon.h"
#include "Prefabs.h"
#include "world/World.h"
#include <memory>

#define Q(s) QStringLiteral(s)
static const float OX = 600.f;
static const QString kGrp = QStringLiteral("Sala de Testes");

static void tun(World& w, std::vector<Vec3> floorPts, float r, int mat = MAT_LIMESTONE, float noise = 0.35f)
{
    for (auto& p : floorPts) p.setY(p.y() + r * 0.55f);
    TunnelOpts o; o.floorFrac = 0.55f; o.material = mat; o.noise = noise; o.k = 1.2f;
    w.carveTunnel(floorPts, r, o);
}

static ZoneEffects caveFx()
{
    ZoneEffects fx;
    fx.temperature = 10.f; fx.fogColor = Vec3(0.010f, 0.010f, 0.014f); fx.fogDensity = 0.030f;
    fx.ambientColor = Vec3(0.030f, 0.032f, 0.045f); fx.ambientGround = Vec3(0.012f, 0.011f, 0.012f); fx.ambientStrength = 1.f;
    return fx;
}

void buildTestLevel(World& w)
{
    const QString session = Q("Camada T · Sala de Testes");
    w.setDefaultZone(caveFx());

    // ======================================================== EXTERIOR (terreno + boca)
    {
        auto H = [](float x, float z) {
            float dx = std::fabs(x - OX);
            float dunes = 0.7f * std::sin(x * 0.21f) * std::cos(z * 0.17f) + 0.4f * std::sin(z * 0.37f + x * 0.11f);
            float cliff = smoothstepf(52.f, 41.f, z) * 60.f;
            float side = smoothstepf(20.f, 31.f, dx) * 60.f;
            return -0.04f * (z - 50.f) + dunes + cliff + side;
        };
        w.setTerrain(H, Aabb(Vec3(OX - 30, -8, 40), Vec3(OX + 30, 45, 125)));
        tun(w, {Vec3(OX, 0.f, 62), Vec3(OX, 0.f, 36), Vec3(OX, 0.f, 0)}, 2.7f, MAT_SNOW, 0.45f);

        ZoneEffects fx;
        fx.areaName = Q("Sala de Testes · Encosta"); fx.subtitle = Q("Exterior · neve e terreno procedural");
        fx.temperature = -15.f; fx.fogColor = Vec3(0.09f, 0.115f, 0.16f); fx.fogDensity = 0.014f;
        fx.ambientColor = Vec3(0.30f, 0.36f, 0.50f); fx.ambientGround = Vec3(0.16f, 0.18f, 0.24f);
        w.addZone(Q("encosta"), ZoneShape::makeBox(Vec3(OX - 31, -9, 38), Vec3(OX + 31, 46, 126)), fx, 1);
        // neve caindo (emissor fixo sobre a encosta)
        w.addEmitter(ParticlePreset::Snow, Vec3(OX, 16.f, 80.f), Vec3(30, 0.5f, 36), 320.f, Q("neve"));
    }

    // ======================================================== CAMARA PRINCIPAL (hub)
    w.carveChamber(Vec3(OX, 3.7f, -12), Vec3(16, 7.7f, 14), 0.f, MAT_LIMESTONE, 0.5f, 2.f, Q("hub"));
    w.scatterStalactites(Aabb(Vec3(OX - 13, 1, -24), Vec3(OX + 13, 11, -1)), 26, 1, 1.5f, 4.5f);
    w.scatterStalagmites(Aabb(Vec3(OX - 14, -1, -24), Vec3(OX - 5, 6, -2)), 8, 2, 0.8f, 2.2f, 0.18f, 0.5f);
    w.scatterStalagmites(Aabb(Vec3(OX + 5, -1, -24), Vec3(OX + 14, 6, -2)), 8, 3, 0.8f, 2.2f, 0.18f, 0.5f);
    {
        ZoneEffects fx = caveFx();
        fx.areaName = Q("Sala de Testes · Câmara principal"); fx.subtitle = session;
        w.addZone(Q("hub"), ZoneShape::makeBox(Vec3(OX - 17, -6, -27), Vec3(OX + 17, 13, 3)), fx, 1);
    }

    // ---- props e criaturas na camara ----
    {
        MeshBuilder mb;
        using namespace Prefabs;
        goblin(mb, Vec3(OX - 9, 0, -18), 30.f);
        goblin(mb, Vec3(OX - 7.4f, 0, -17.2f), -20.f, Pose::Crouch, 1.f, true);
        goblinChild(mb, Vec3(OX - 8.2f, 0, -16.2f), 0.f, Pose::Huddle);
        HumanoidParams hp; hp.hood = true; hp.cape = true; hp.cloth = QColor(70, 50, 90);
        humanoid(mb, Vec3(OX - 4.5f, 0, -19.f), 20.f, hp);
        HumanoidParams hp2; hp2.pose = Pose::Sit; hp2.skin = QColor(180, 120, 100);
        humanoid(mb, Vec3(OX + 11.f, 0, -9.f), -90.f, hp2);
        HumanoidParams hp3; hp3.pose = Pose::Lie; hp3.glowingEyes = true; hp3.eyeColor = QColor(120, 200, 255);
        humanoid(mb, Vec3(OX - 12.f, 0, -4.f), 70.f, hp3);
        wolf(mb, Vec3(OX + 9.5f, 0, -14.5f), -70.f, true);
        wolf(mb, Vec3(OX + 7.5f, 0, -15.8f), -20.f, false, Pose::Lie);
        brazier(mb, Vec3(OX - 8.f, 0, -6.f), 1.f, true);
        brazier(mb, Vec3(OX + 8.f, 0, -4.f), 0.9f, false);
        campfire(mb, Vec3(OX + 3.f, 0, -4.f));
        crate(mb, Vec3(OX + 10.f, 0, -6.f), 15.f); crate(mb, Vec3(OX + 11.2f, 0, -6.9f), 40.f, 0.7f);
        barrel(mb, Vec3(OX + 11.f, 0, -4.2f)); barrel(mb, Vec3(OX + 12.2f, 0, -5.f), 30.f, 0.9f);
        sack(mb, Vec3(OX - 12.f, 0, -9.f)); sack(mb, Vec3(OX - 11.2f, 0, -9.8f), 40.f, 0.8f);
        hideTent(mb, Vec3(OX - 12.5f, 0, -13.f), 20.f, 2.f, 1.8f);
        bonesPile(mb, Vec3(OX + 1.f, 0, -17.f), 1.1f, 4); skull(mb, Vec3(OX + 2.2f, 0.f, -16.f), 40.f);
        ribcage(mb, Vec3(OX + 12.5f, 0, -15.f), 100.f);
        giantRib(mb, Vec3(OX - 14.f, 0, -20.f), Vec3(OX - 11.f, 0, -23.f), 0.22f, 5.5f, 2);
        totemOfBones(mb, Vec3(OX - 3.f, 0, -3.f), 2.3f, 3);
        table(mb, Vec3(OX + 11.f, 0, -11.f), -90.f, Vec3(1.6f, 0.8f, 0.8f), true);
        chest(mb, Vec3(OX - 3.f, 0, -22.f), 0.f, true);
        chest(mb, Vec3(OX - 5.5f, 0, -22.f), 10.f, false);
        altar(mb, Vec3(OX + 1.5f, 0, -23.f), 0.f);
        chain(mb, Vec3(OX - 6.f, 9.5f, -8.f), Vec3(OX - 2.f, 9.5f, -9.f), 0.6f);
        hangingCage(mb, Vec3(OX + 4.f, 9.6f, -15.f), 1.5f, 4.f, true);
        glassCapsule(mb, Vec3(OX + 5.5f, 0, -21.8f), 2.4f, 0.7f, true, false);
        glassCapsule(mb, Vec3(OX + 8.2f, 0, -21.0f), 2.4f, 0.7f, true, true);
        pipe(mb, {Vec3(OX + 5.5f, 2.4f, -21.8f), Vec3(OX + 5.5f, 6.f, -21.8f), Vec3(OX + 4.f, 8.f, -18.f), Vec3(OX + 3.f, 9.f, -12.f)}, 0.09f);
        scaffoldSegment(mb, Vec3(OX - 14, 0, -6), Vec3(OX - 14, 0, -1), 4.f, 2.5f);
        gratedWalkway(mb, Vec3(OX - 14, 4.f, -6), Vec3(OX - 14, 4.f, -1), 2.f);
        ironStairs(mb, Vec3(OX - 12.5f, 0, -1.5f), Vec3(OX - 12.5f, 4.f, -5.5f), 1.2f);
        wallTorch(mb, Vec3(OX + 15.f, 3.f, -12.f), Vec3(-1, 0, 0));
        wallTorch(mb, Vec3(OX - 15.f, 3.f, -12.f), Vec3(1, 0, 0));
        crystalCluster(mb, Vec3(OX - 2.f, 0, -10.f), Vec3(0, 1, 0), 0.9f, 7, 5, QColor(160, 90, 255));
        crystalCluster(mb, Vec3(OX + 14.f, 0, -20.f), Vec3(0, 1, 0), 1.1f, 6, 9, QColor(90, 200, 255));
        mushroom(mb, Vec3(OX + 7.f, 0, -2.f), 0.5f); mushroom(mb, Vec3(OX + 7.6f, 0, -1.4f), 0.35f, QColor(255, 120, 200));
        iceFringe(mb, Vec3(OX - 4, 9.4f, -3), Vec3(OX + 4, 9.5f, -3), 9, 1.4f, 5);
        w.addStatic(std::move(mb));
    }
    // luzes: braseiro (alternavel), fogueira, tochas, cristais
    Light* hubLight = w.addLight(Vec3(OX - 8, 1.9f, -6), Vec3(1.0f, 0.55f, 0.2f), 6.f, 13.f, LightAnim::TorchFlicker, Q("braseiro"));
    w.addLight(Vec3(OX + 3, 1.2f, -4), Vec3(1.0f, 0.5f, 0.18f), 5.f, 11.f, LightAnim::TorchFlicker, Q("fogueira"));
    w.addLight(Vec3(OX + 14, 3.4f, -12), Vec3(1.0f, 0.6f, 0.25f), 4.5f, 10.f, LightAnim::TorchFlicker);
    w.addLight(Vec3(OX - 14, 3.4f, -12), Vec3(1.0f, 0.6f, 0.25f), 4.5f, 10.f, LightAnim::TorchFlicker);
    w.addLight(Vec3(OX - 2, 1.2f, -10), Vec3(0.6f, 0.3f, 1.0f), 4.f, 8.f, LightAnim::Pulse, Q("cristal roxo"))->animFreq = 0.4f;
    w.addLight(Vec3(OX + 6.8f, 1.5f, -20.5f), Vec3(0.3f, 1.0f, 0.5f), 3.5f, 7.f, LightAnim::Pulse, Q("capsula"));
    w.addEmitter(ParticlePreset::Embers, Vec3(OX - 8, 1.5f, -6), Vec3(0.4f, 0.1f, 0.4f), -1.f, Q("brasas"));
    w.addEmitter(ParticlePreset::Embers, Vec3(OX + 3, 0.8f, -4), Vec3(0.3f, 0.1f, 0.3f), -1.f, Q("brasas fogueira"));
    w.addEmitter(ParticlePreset::Dust, Vec3(OX, 5, -12), Vec3(10, 3, 10), -1.f, Q("poeira"));
    // interativo: alternar o braseiro
    w.addInteractable(Q("alavanca"), Vec3(OX - 10.5f, 1.1f, -2.f), 0.9f, Q("Alternar o braseiro"), [&w, hubLight]() {
        hubLight->enabled = !hubLight->enabled;
        w.state.toast(hubLight->enabled ? Q("O braseiro se acende.") : Q("O braseiro se apaga."), QColor(255, 210, 140), 2.f);
    });
    {
        MeshBuilder mb; Prefabs::lever(mb, Vec3(OX - 10.5f, 0, -2.f), 0.f, 35.f); w.addStatic(std::move(mb));
    }

    // ======================================================== ENGRENAGEM GIRATORIA (dinamico)
    {
        MeshBuilder mb; Prefabs::gear(mb, Vec3(0, 0, 0), Vec3(0, 0, 1), 1.5f, 14, 0.3f, MAT_BRONZE);
        DynHandle gear = w.addDynamic(Q("engrenagem"), std::move(mb), Vec3(OX + 8, 6.f, -23.8f));
        w.addUpdater([gear](float, float t) { gear->setRotation(QQuaternion::fromAxisAndAngle(0, 0, 1, t * 25.f)); });
    }

    // ======================================================== PISCINA (nado) + esporos (alucinacao)
    {
        w.carveChamber(Vec3(OX - 30, 0.5f, -12), Vec3(10, 6, 10), -3.5f, MAT_WET_LIMESTONE, 0.45f, 2.f, Q("piscina"));
        tun(w, {Vec3(OX - 13, 0, -12), Vec3(OX - 22, 0, -12)}, 2.f, MAT_WET_LIMESTONE);
        w.fill(Prim::box(Vec3(OX - 21.5f, -1.75f, -12), Vec3(2.5f, 1.75f, 3.5f), QQuaternion(), 0.3f).noise(0.12f).mat(MAT_WET_LIMESTONE));
        w.scatterStalactites(Aabb(Vec3(OX - 38, 0, -20), Vec3(OX - 22, 6, -4)), 10, 7, 1.f, 3.f, 0.2f, 0.5f, MAT_WET_LIMESTONE);
        ZoneEffects fx; fx.areaName = Q("Sala de Testes · Piscina"); fx.subtitle = Q("Nado, fôlego e tinta submersa");
        fx.temperature = 2.f; fx.fogColor = Vec3(0.008f, 0.02f, 0.03f); fx.fogDensity = 0.04f;
        fx.ambientColor = Vec3(0.03f, 0.06f, 0.08f); fx.ambientGround = Vec3(0.01f, 0.02f, 0.03f);
        w.addZone(Q("piscina_area"), ZoneShape::makeBox(Vec3(OX - 41, -6, -23), Vec3(OX - 20, 7, -1)), fx, 2);
        ZoneEffects wf; wf.water = true; wf.waterSurfaceY = -0.5f;
        w.addZone(Q("piscina_agua"), ZoneShape::makeBox(Vec3(OX - 41, -4, -23), Vec3(OX - 20.5f, -0.5f, -1)), wf, 3);
        ZoneEffects hf; hf.hallucination = 0.85f; hf.areaName = Q("Bosque de Esporos"); hf.subtitle = Q("Alucinação: a rocha vira carne");
        w.addZone(Q("esporos"), ZoneShape::makeSphere(Vec3(OX - 33, 3.f, -4.5f), 4.2f), hf, 4);
        MeshBuilder wm; wm.setFlow(-1, 0.3f);
        wm.boxMinMax(Vec3(OX - 41, -3.5f, -23), Vec3(OX - 20.5f, -0.5f, -1), QColor(40, 110, 140, 150), MAT_WATER);
        w.addStatic(std::move(wm));
        MeshBuilder pm; using namespace Prefabs;
        mushroom(pm, Vec3(OX - 33, 3.f, -7.f), 0.6f); mushroom(pm, Vec3(OX - 32.4f, 0.f, -4.f), 0.4f);  // (os de fundo ficam na parede)
        ropeBridge(pm, Vec3(OX - 40, 4.2f, -20), Vec3(OX - 26, 4.2f, -3), 1.4f, 0.8f, 18, true, 5);
        w.addStatic(std::move(pm));
        w.addLight(Vec3(OX - 30, -1.5f, -12), Vec3(0.2f, 0.7f, 1.0f), 6.f, 12.f, LightAnim::Pulse, Q("agua"))->animFreq = 0.25f;
        w.addLight(Vec3(OX - 30, 3.f, -12), Vec3(0.5f, 0.8f, 1.0f), 3.f, 10.f, LightAnim::None);
        w.addEmitter(ParticlePreset::Spores, Vec3(OX - 33, 2.5f, -5), Vec3(3, 1.5f, 3), -1.f, Q("esporos"));
    }

    // ======================================================== POCO HELICOIDAL + vento + gelo
    {
        const Vec3 pc(OX + 45, 0, -12);
        w.carve(Prim::cylinder(pc, 9.5f, -17.f, 9.f).noise(0.4f).named(Q("poco")));
        tun(w, {Vec3(OX + 14, 0, -12), Vec3(OX + 37, 0, -12)}, 2.f);
        HelixDesc hd; hd.center = pc; hd.rInner = 6.2f; hd.rOuter = 10.4f; hd.startY = 0.f; hd.dropPerTurn = 8.f;
        hd.startAngleDeg = 180.f; hd.t0 = 0.f; hd.t1 = 1.95f; hd.thickness = 2.4f;
        w.fill(Prim::helix(hd).noise(0.12f).smooth(0.3f).mat(MAT_WET_LIMESTONE));
        w.scatterStalactites(Aabb(Vec3(OX + 37, -2, -20), Vec3(OX + 53, 9, -4)), 8, 11, 1.5f, 4.f, 0.3f, 0.7f);
        ZoneEffects fx; fx.areaName = Q("Sala de Testes · Poço helicoidal"); fx.subtitle = Q("Rampa em espiral, vento e queda");
        fx.temperature = -5.f; fx.fogColor = Vec3(0.008f, 0.012f, 0.02f); fx.fogDensity = 0.02f;
        fx.wind = true; fx.windPeriod = 24.f; fx.windDuration = 2.5f;
        w.addZone(Q("poco"), ZoneShape::makeCylinder(pc, 10.f, -17.f, 9.f), fx, 2);
        ZoneEffects tf; tf.areaName = Q("Túnel de Gelo"); tf.subtitle = Q("Ar rarefeito: a chama encolhe"); tf.torchScale = 0.45f; tf.temperature = -12.f;
        w.addZone(Q("rarefeito"), ZoneShape::makeBox(Vec3(OX + 25.5f, -1, -15), Vec3(OX + 35, 4, -9)), tf, 3);
        w.addLight(Vec3(pc.x(), -8, pc.z()), Vec3(0.4f, 0.6f, 1.0f), 9.f, 20.f, LightAnim::Pulse, Q("poco"))->animFreq = 0.2f;
        w.addEmitter(ParticlePreset::Dust, Vec3(pc.x(), -4, pc.z()), Vec3(7, 5, 7), 14.f, Q("poeira do poco"));
        // muro de gelo (colisor "openable") + prisma + feixe com raycast
        MeshBuilder im; im.boxMinMax(Vec3(-0.5f, -1.7f, -2.1f), Vec3(0.5f, 1.7f, 2.1f), QColor(170, 215, 240, 120), MAT_CLEAR_ICE);
        DynHandle ice = w.addDynamic(Q("muro_gelo"), std::move(im), Vec3(OX + 24, 1.6f, -12));
        Collider* iceCol = w.addBoxCollider(Q("muro_gelo"), Vec3(), Vec3(0.5f, 1.7f, 2.1f), QQuaternion(), true);
        w.attachCollider(iceCol, ice);
        {
            MeshBuilder pm; pm.cylinder(Vec3(OX + 6, 0, -12), Vec3(OX + 6, 1.05f, -12), 0.45f, 0.35f, 10, QColor(95, 90, 100), MAT_IRON);
            w.addStatic(std::move(pm));
        }
        MeshBuilder pr;
        pr.cone(Vec3(0, 0, 0), Vec3(0, 0.45f, 0), 0.32f, 6, QColor(200, 150, 255, 150), MAT_CRYSTAL);
        pr.cone(Vec3(0, 0, 0), Vec3(0, -0.45f, 0), 0.32f, 6, QColor(200, 150, 255, 150), MAT_CRYSTAL);
        DynHandle prism = w.addDynamic(Q("prisma"), std::move(pr), Vec3(OX + 6, 1.55f, -12));
        Beam* beam = w.addBeam(Q("raio"), Vec3(OX + 6, 1.55f, -12), Vec3(OX + 6, 1.55f, -20), QColor(255, 220, 120), 0.14f);
        EmitterHandle sparks = w.addEmitter(ParticlePreset::Sparks, Vec3(OX + 6, 1.55f, -20), Vec3(0.05f, 0.05f, 0.05f), 25.f, Q("fagulhas do raio"));
        struct St { int step = 0; bool melted = false; float melt = -1.f; };
        auto st = std::make_shared<St>();
        w.addInteractable(Q("prisma"), Vec3(OX + 6, 1.4f, -12), 0.8f, Q("Girar o prisma (45°)"), [&w, st]() {
            st->step = (st->step + 1) % 8;
            w.state.toast(Q("Prisma girado: %1°").arg(st->step * 45), QColor(220, 200, 255), 1.6f);
        });
        w.registerEvent(Q("refreeze"), Q("Recongelar o muro de gelo"), 0, [&w, st, ice, iceCol]() {
            st->melted = false; st->melt = -1.f; ice->visible = true; ice->scl = Vec3(1, 1, 1); ice->pos.setY(1.6f); iceCol->enabled = true;
            w.state.toast(Q("O muro de gelo se refaz."), QColor(170, 220, 255), 2.5f);
        });
        w.addUpdater([&w, st, beam, sparks, prism, ice, iceCol](float dt, float t) {
            float a = st->step * 45.f * 0.0174533f;
            Vec3 dir(std::sin(a), 0, -std::cos(a)), o(OX + 6, 1.55f, -12);
            RayHit h = w.raycast(o + dir * 0.6f, dir, 60.f, true);
            Vec3 end = h.hit ? h.pos : o + dir * 60.f;
            beam->a = o + dir * 0.4f; beam->b = end;
            sparks->setPosition(end); sparks->setEnabled(h.hit);
            prism->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, -st->step * 45.f + t * 20.f));
            if (h.hit && h.collider == QLatin1String("muro_gelo") && !st->melted) {
                st->melted = true; st->melt = 0.f;
                w.state.toast(Q("O feixe derrete o muro de gelo!"), QColor(255, 230, 150), 3.f);
            }
            if (st->melt >= 0.f && st->melt < 1.6f) {
                st->melt += dt;
                float s = std::max(0.02f, 1.f - st->melt / 1.6f);
                ice->scl = Vec3(1, s, 1); ice->pos.setY(1.7f * s - 0.1f);
                if (st->melt >= 1.6f) { ice->visible = false; iceCol->enabled = false; }
            }
        });
    }

    // ======================================================== SALA DE LAVA + ponte que colapsa + vazio
    {
        w.carveChamber(Vec3(OX, 3.2f, -52), Vec3(11, 6.6f, 10), 0.f, MAT_HOT_BASALT, 0.4f, 2.f, Q("sala_lava"));
        tun(w, {Vec3(OX, 0, -24), Vec3(OX, 0, -44)}, 2.f, MAT_HOT_BASALT);
        w.carve(Prim::cylinder(Vec3(OX, 0, -52), 4.2f, -7.f, 1.f).noise(0.3f).mat(MAT_HOT_BASALT).named(Q("fosso_lava")));
        w.scatterStalactites(Aabb(Vec3(OX - 9, 1, -60), Vec3(OX + 9, 9, -44)), 14, 21, 1.f, 3.5f, 0.2f, 0.6f, MAT_HOT_BASALT);
        ZoneEffects fx; fx.areaName = Q("Sala de Testes · Lava"); fx.subtitle = Q("Perigo: lava e ponte que colapsa");
        fx.temperature = 38.f; fx.fogColor = Vec3(0.05f, 0.015f, 0.008f); fx.fogDensity = 0.03f;
        fx.ambientColor = Vec3(0.09f, 0.035f, 0.02f); fx.ambientGround = Vec3(0.05f, 0.018f, 0.01f);
        w.addZone(Q("sala_lava"), ZoneShape::makeBox(Vec3(OX - 12, -8, -63), Vec3(OX + 12, 11, -41)), fx, 2);
        ZoneEffects hz; hz.hazard = true; hz.hazardText = Q("Lava: 10d10 de dano de fogo!"); hz.temperature = 70.f;
        w.addZone(Q("lava"), ZoneShape::makeCylinder(Vec3(OX, 0, -52), 4.2f, -7.f, -0.9f), hz, 5);
        MeshBuilder lm; lm.setFlow(0.3f, 1.f);
        lm.cylinder(Vec3(OX, -1.3f, -52), Vec3(OX, -1.2f, -52), 4.5f, 4.5f, 28, QColor(255, 120, 20), MAT_LAVA);
        w.addStatic(std::move(lm));
        w.addLight(Vec3(OX, 0.8f, -52), Vec3(1.0f, 0.35f, 0.08f), 9.f, 18.f, LightAnim::Pulse, Q("lava"))->animFreq = 0.5f;
        w.addEmitter(ParticlePreset::Embers, Vec3(OX, -0.8f, -52), Vec3(3.5f, 0.1f, 3.5f), 40.f, Q("brasas da lava"));
        w.addEmitter(ParticlePreset::Steam, Vec3(OX + 6, 0.2f, -48), Vec3(0.4f, 0.1f, 0.4f), -1.f, Q("vapor"));
        // vazio giratorio + fluxo de calor subindo
        MeshBuilder vm; vm.cylinder(Vec3(0, 0, 0), Vec3(0, 0.12f, 0), 1.6f, 1.6f, 24, QColor(10, 5, 20), MAT_COAL);
        vm.torus(Vec3(0, 0.06f, 0), Vec3(0, 1, 0), 1.7f, 0.12f, QColor(190, 90, 255), MAT_EMISSIVE, 28, 6);
        DynHandle voidD = w.addDynamic(Q("vazio"), std::move(vm), Vec3(OX, 9.2f, -52));
        w.addUpdater([voidD](float, float t) { voidD->setRotation(QQuaternion::fromAxisAndAngle(0, 1, 0, t * 60.f)); });
        EmitterDesc vd; vd.preset = ParticlePreset::VoidStream; vd.pos = Vec3(OX, -0.5f, -52); vd.name = Q("fluxo para o vazio");
        ParticleEmitter::applyPreset(vd); vd.target = Vec3(OX, 9.2f, -52); vd.halfExtent = Vec3(3.f, 0.3f, 3.f);
        w.addEmitter(vd);
        // ponte de tabuas
        MeshBuilder bm;
        for (int i = 0; i < 24; ++i) {
            float x = -5.3f + i * 0.45f;
            bm.box(Vec3(x, 0, 0), Vec3(0.2f, 0.05f, 0.9f), QColor(110 + (i % 3) * 12, 72, 40), MAT_WOOD);
        }
        bm.box(Vec3(0, -0.12f, 0.6f), Vec3(5.4f, 0.07f, 0.07f), QColor(70, 50, 30), MAT_ROTTEN_WOOD);
        bm.box(Vec3(0, -0.12f, -0.6f), Vec3(5.4f, 0.07f, 0.07f), QColor(70, 50, 30), MAT_ROTTEN_WOOD);
        DynHandle bridge = w.addDynamic(Q("ponte"), std::move(bm), Vec3(OX, 0.15f, -52));
        Collider* bc = w.addBoxCollider(Q("ponte"), Vec3(), Vec3(5.4f, 0.1f, 0.95f));
        w.attachCollider(bc, bridge);
        struct Br { bool fall = false; float t = 0, vy = 0; };
        auto br = std::make_shared<Br>();
        w.registerEvent(Q("collapse"), Q("Colapsar a ponte de tábuas"), Qt::Key_Z, [&w, br, bc]() {
            if (br->fall) return;
            br->fall = true; br->t = 0; br->vy = 0; bc->enabled = false;
            w.state.screenShake(0.06f, 0.6f); w.state.toast(Q("As cordas se rompem! A ponte desaba."), QColor(255, 180, 120), 3.f);
        });
        w.registerEvent(Q("restore"), Q("Restaurar a ponte"), 0, [br, bc, bridge]() {
            br->fall = false; br->t = 0; bc->enabled = true; bridge->visible = true; bridge->pos = Vec3(OX, 0.15f, -52); bridge->rot = QQuaternion();
        });
        w.addUpdater([br, bridge](float dt, float) {
            if (!br->fall || br->t > 3.f) return;
            br->t += dt; br->vy -= 9.8f * dt;
            bridge->pos.setY(bridge->pos.y() + br->vy * dt);
            bridge->setRotation(QQuaternion::fromAxisAndAngle(Vec3(0.3f, 0.2f, 1), br->t * 55.f));
            if (br->t > 3.f) bridge->visible = false;
        });
    }

    // ======================================================== GRAVIDADE INVERTIDA
    {
        const Vec3 gc(OX + 45, 6, -52);
        w.carve(Prim::box(gc, Vec3(9, 6, 9), QQuaternion(), 1.5f).noise(0.3f).mat(MAT_GEODE).named(Q("sala_gravidade")));
        tun(w, {Vec3(OX + 12, 0, -20), Vec3(OX + 26, 0, -34), Vec3(OX + 26, 0, -50), Vec3(OX + 38, 0, -50)}, 2.f, MAT_GEODE);
        w.fill(Prim::box(Vec3(OX + 45, 1, -52), Vec3(3, 1, 3), QQuaternion(), 0.2f).noise(0.1f).mat(MAT_GEODE));      // plataforma (topo y=2)
        w.fill(Prim::box(Vec3(OX + 45, 12.5f, -52), Vec3(3, 2, 3), QQuaternion(), 0.2f).noise(0.1f).mat(MAT_GEODE));  // mesas no teto
        w.fill(Prim::box(Vec3(OX + 40, 12.5f, -47), Vec3(2, 2, 2), QQuaternion(), 0.2f).noise(0.1f).mat(MAT_GEODE));
        ZoneEffects fx; fx.areaName = Q("Sala de Testes · Gravidade"); fx.subtitle = Q("Pule da plataforma: a gravidade aponta para o teto");
        fx.temperature = 14.f; fx.fogColor = Vec3(0.02f, 0.008f, 0.03f); fx.fogDensity = 0.022f;
        fx.ambientColor = Vec3(0.06f, 0.03f, 0.09f); fx.ambientGround = Vec3(0.03f, 0.015f, 0.05f);
        w.addZone(Q("sala_gravidade"), ZoneShape::makeBox(Vec3(OX + 35, -1, -62), Vec3(OX + 55, 13, -42)), fx, 2);
        ZoneEffects gf; gf.gravity = Vec3(0, 9.81f, 0);
        w.addZone(Q("gravidade_invertida"), ZoneShape::makeBox(Vec3(OX + 36, 3.2f, -61), Vec3(OX + 54, 12.5f, -43)), gf, 6);
        MeshBuilder cm; using namespace Prefabs;
        crystalCluster(cm, Vec3(OX + 45, 2, -52), Vec3(0, 1, 0), 1.0f, 6, 31, QColor(190, 100, 255));
        crystalCluster(cm, Vec3(OX + 45, 10.5f, -52), Vec3(0, -1, 0), 1.0f, 6, 32, QColor(140, 100, 255));
        crystalCluster(cm, Vec3(OX + 40, 10.5f, -47), Vec3(0, -1, 0), 0.8f, 5, 33, QColor(255, 120, 220));
        crystalCluster(cm, Vec3(OX + 50, 0, -57), Vec3(0, 1, 0), 1.3f, 8, 34, QColor(120, 180, 255));
        w.addStatic(std::move(cm));
        w.addLight(Vec3(OX + 45, 4.5f, -52), Vec3(0.7f, 0.35f, 1.0f), 8.f, 18.f, LightAnim::Pulse, Q("cristais"))->animFreq = 0.3f;
    }

    // ======================================================== CHAMINE (escalada) + topo + parede mutante
    {
        const Vec3 sc(OX - 36, 0, -46);
        tun(w, {Vec3(OX - 12, 0, -22), Vec3(OX - 24, 0, -34), Vec3(OX - 33, 0, -43.5f), Vec3(sc.x(), 0, sc.z() + 1.f)}, 2.f);
        // poco vertical: cilindro de chao plano ligado ao fim do tunel (escalavel nas paredes)
        w.carve(Prim::cylinder(sc, 1.9f, 0.f, 20.f).noise(0.25f).smooth(0.8f).named(Q("chamine")));
        w.carveChamber(Vec3(sc.x(), 21.7f, sc.z()), Vec3(7, 4.2f, 7), 19.f, MAT_LIMESTONE, 0.4f, 2.f, Q("topo_chamine"));
        tun(w, {Vec3(OX - 31, 19, -44), Vec3(OX - 20, 10, -32), Vec3(OX - 8, 0, -17)}, 2.f);   // rampa-tunel de volta ao hub
        ZoneEffects cf; cf.climb = true;
        w.addZone(Q("chamine_escalada"), ZoneShape::makeBox(Vec3(sc.x() - 2.6f, 0.2f, sc.z() - 2.6f), Vec3(sc.x() + 2.6f, 19.6f, sc.z() + 2.6f)), cf, 4);
        ZoneEffects bf; bf.areaName = Q("Sala de Testes · Chaminé"); bf.subtitle = Q("Escalada, queda e zona psíquica"); bf.temperature = 6.f;
        w.addZone(Q("chamine_area"), ZoneShape::makeBox(Vec3(OX - 44, -1, -54), Vec3(OX - 27, 27, -38)), bf, 2);
        ZoneEffects pf; pf.psychic = 0.8f; pf.torchScale = 0.55f; pf.areaName = Q("Topo da Chaminé"); pf.subtitle = Q("Efeito psíquico + ar rarefeito");
        w.addZone(Q("topo_psiquico"), ZoneShape::makeBox(Vec3(OX - 43, 19, -53), Vec3(OX - 29, 27, -39)), pf, 5);
        MeshBuilder lm; Prefabs::ladder(lm, Vec3(sc.x() + 1.45f, 0.f, sc.z()), Vec3(sc.x() + 1.45f, 19.f, sc.z()), 90.f, 0.55f);
        w.addStatic(std::move(lm));
        w.addLight(Vec3(sc.x() + 0.5f, 3, sc.z() + 0.5f), Vec3(1, 0.7f, 0.4f), 4.f, 9.f, LightAnim::TorchFlicker);
        w.addLight(Vec3(sc.x(), 21, sc.z()), Vec3(0.7f, 0.5f, 1.f), 5.f, 12.f, LightAnim::Pulse)->animFreq = 0.6f;

        // parede mutante: some e reaparece quando nao esta no campo de visao
        MeshBuilder wm; wm.box(Vec3(0, 0, 0), Vec3(2.3f, 1.8f, 0.3f), QColor(110, 100, 90), MAT_LIMESTONE);
        wm.box(Vec3(0, 0, 0.31f), Vec3(2.0f, 1.5f, 0.02f), QColor(70, 60, 120), MAT_ALTERED);
        const Vec3 wpos(OX - 18, 1.8f, -28);
        DynHandle wall = w.addDynamic(Q("parede_mutante"), std::move(wm), wpos, QQuaternion::fromAxisAndAngle(0, 1, 0, 45.f));
        Collider* wc = w.addBoxCollider(Q("parede_mutante"), Vec3(), Vec3(2.3f, 1.8f, 0.3f), QQuaternion(), true);
        w.attachCollider(wc, wall);
        struct Sh { bool closed = true; float cd = 0.f; };
        auto sh = std::make_shared<Sh>();
        w.addUpdater([&w, sh, wall, wc, wpos](float dt, float) {
            sh->cd -= dt;
            Vec3 pp = w.playerPos();
            float d = (pp - wpos).length();
            if (sh->cd > 0.f || d > 22.f || d < 3.5f) return;
            Vec3 c = sh->closed ? wpos : wpos + Vec3(0, 4.6f, 0);
            if (!w.isVisibleToPlayer(c, 40.f)) {
                sh->closed = !sh->closed; sh->cd = 2.5f;
                wall->pos = sh->closed ? wpos : wpos + Vec3(0, 4.6f, 0);
                wc->enabled = sh->closed;
            }
        });
    }

    // ======================================================== EVENTO: emboscada goblin (dinamicos em tempo de execucao)
    {
        struct Amb { std::vector<DynHandle> gs; float t = -1.f; };
        auto amb = std::make_shared<Amb>();
        w.registerEvent(Q("ambush"), Q("Emboscada goblin"), Qt::Key_H, [&w, amb]() {
            Vec3 p = w.playerPos();
            amb->gs.clear();
            for (int i = 0; i < 4; ++i) {
                MeshBuilder mb; Prefabs::goblin(mb, Vec3(0, 0, 0), 0.f, Prefabs::Pose::Stand, 1.f, i % 2 == 0);
                float a = i * 1.5708f + 0.4f;
                Vec3 pos = p + Vec3(std::cos(a) * 6.f, 0.f, std::sin(a) * 6.f);
                if (w.solidDistance(pos + Vec3(0, 0.9f, 0)) < 0.4f) pos = p + Vec3(std::cos(a) * 3.f, 0.f, std::sin(a) * 3.f);
                amb->gs.push_back(w.addDynamic(Q("goblin_emboscada_%1").arg(i), std::move(mb), pos));
            }
            amb->t = 0.f;
            w.state.toast(Q("Goblins surgem das sombras!"), QColor(255, 170, 90), 3.f);
        });
        w.addUpdater([&w, amb](float dt, float) {
            if (amb->t < 0.f) return;
            amb->t += dt;
            Vec3 p = w.playerPos();
            for (DynHandle g : amb->gs) {
                Vec3 d = p - g->pos; d.setY(0);
                float l = d.length();
                if (l > 1.2f) g->pos += d / l * 3.2f * dt;
                g->setYaw(std::atan2(-d.x(), -d.z()) * 57.2957795f);
                g->pos.setY(g->pos.y() + std::sin(amb->t * 14.f) * 0.004f);
            }
            if (amb->t > 8.f) { for (DynHandle g : amb->gs) g->visible = false; amb->t = -1.f; }
        });
    }

    // ======================================================== POIs (um por categoria + extras)
    w.addPoi(Vec3(OX, 1.6f, -12), PoiCategory::Area, Q("Câmara principal de testes"),
             Q("Hub da Sala de Testes. Saídas: oeste = piscina (nado), leste = poço helicoidal (atrás do muro de gelo), norte = lava e ponte, nordeste = gravidade invertida, noroeste = chaminé de escalada.\n\nUse esta sala para conferir o visual da rocha, das criaturas e dos adereços sob a luz da tocha."), session);
    w.addPoi(Vec3(OX + 6, 1.7f, -52), PoiCategory::Trap, Q("Ponte de tábuas que colapsa"),
             Q("Colisor de tábuas ligado a um objeto dinâmico. O evento \"Colapsar a ponte\" (tecla Z ou aba Eventos) desativa o colisor e derruba o objeto sobre a lava (zona de perigo: dano + respawn).\n\nMostra: colisor attachável, evento do Mestre, zona de hazard."), session);
    w.addPoi(Vec3(OX + 6, 2.3f, -12), PoiCategory::Puzzle, Q("Prisma e muro de gelo"),
             Q("Interaja com o prisma [E] para girar o feixe em passos de 45°. O feixe é calculado por raycast sobre rocha + colisores; ao atingir o colisor \"muro_gelo\" (flag openable) ele derrete.\n\nSolução: duas rotações (90°) apontam o feixe para o leste."), session);
    w.addPoi(Vec3(OX - 17.5f, 2.5f, -27.5f), PoiCategory::Secret, Q("Parede que muda de lugar"),
             Q("Usa isVisibleToPlayer(): quando o jogador não vê a parede (fora do frustum ou ocluída) ela alterna entre fechar e abrir o túnel. É o mecanismo do Labirinto Alienígena."), session);
    w.addPoi(Vec3(OX - 8, 1.4f, -18), PoiCategory::Creature, Q("Goblins de teste"),
             Q("Prefabs: goblin (de pé e agachado, com arco), filhote goblin encolhido, humanoide encapuzado, lobo e hell hound. Olhos emissivos aparecem no escuro."), session);
    w.addPoi(Vec3(OX - 3, 1.2f, -22), PoiCategory::Treasure, Q("Baú de teste"),
             Q("Prefab chest (aberto com ouro / fechado). Posicione POIs de tesouro sobre o item para facilitar a leitura dos rótulos em modo Mestre."), session);
    w.addPoi(Vec3(OX + 1.5f, 1.6f, -23), PoiCategory::Lore, Q("Altar de obsidiana"),
             Q("Exemplo de nota de Lore. Textos podem ter várias linhas.\n\nLinha 3: a rolagem do painel funciona com a roda do mouse quando o texto é longo.\n\n(texto de preenchimento)\nLorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."), session);
    w.addPoi(Vec3(OX - 36, 20.8f, -43), PoiCategory::Rest, Q("Descanso no topo da chaminé"),
             Q("Alcova alta e protegida. Zona psíquica (distorção + aberração cromática) e ar rarefeito (a tocha encolhe)."), session);
    w.addPoi(Vec3(OX - 30, 1.2f, -12), PoiCategory::Area, Q("Piscina (nado)"),
             Q("Zona de água: flutuação, fôlego (barra azul), Espaço sobe e Ctrl desce. A tocha apaga debaixo d'água.\nPerto do fundo há uma zona de alucinação (esporos)."), session);
    w.addPoi(Vec3(OX + 45, 3.4f, -52), PoiCategory::Puzzle, Q("Gravidade invertida"),
             Q("Pule da plataforma de cristal: ao entrar na zona a gravidade aponta para o teto e a câmera rola suavemente. É possível andar no teto sobre as mesas."), session);
    w.addPoi(Vec3(OX + 38, 1.6f, -12), PoiCategory::Trap, Q("Poço helicoidal"),
             Q("HelixRamp (Fill) colada à parede, 17 m de profundidade. Rajadas de vento a cada 24 s apagam a tocha; queda causa dano (1d6 por 3 m)."), session);

    // ======================================================== PONTOS DE VISTA (posicao = OLHOS) e ROTA
    w.addViewpoint(Q("Teste · Encosta e boca da caverna"), Vec3(OX, 1.65f, 88), 0.f, 8.f, false, kGrp);
    w.addViewpoint(Q("Teste · Túnel de entrada"), Vec3(OX, 1.65f, 34), 0.f, 0.f, false, kGrp);
    w.addViewpoint(Q("Teste · Câmara principal"), Vec3(OX + 7, 1.65f, -3), -22.f, 4.f, false, kGrp);
    w.addViewpoint(Q("Teste · Criaturas e adereços"), Vec3(OX + 3, 1.65f, -6), 35.f, 0.f, false, kGrp);
    w.addViewpoint(Q("Teste · Goblins e hell hound (perto)"), Vec3(OX - 5.5f, 1.5f, -12.5f), 28.f, 0.f, false, kGrp);
    w.addViewpoint(Q("Teste · Cápsulas de vidro e tubos"), Vec3(OX + 4.f, 1.65f, -14.f), -22.f, 2.f, false, kGrp);
    w.addViewpoint(Q("Teste · Fosso de cristais e capsulas"), Vec3(OX + 4, 1.65f, -9), -75.f, 3.f, false, kGrp);
    w.addViewpoint(Q("Teste · Piscina (nado)"), Vec3(OX - 23, 1.65f, -12), 90.f, -10.f, false, kGrp);
    w.addViewpoint(Q("Teste · Prisma e muro de gelo"), Vec3(OX + 11, 1.65f, -12), -90.f, 0.f, false, kGrp);
    w.addViewpoint(Q("Teste · Poço helicoidal"), Vec3(OX + 36, 1.65f, -12), -90.f, -38.f, false, kGrp);
    w.addViewpoint(Q("Teste · Lava e ponte"), Vec3(OX, 1.65f, -44), 0.f, -12.f, false, kGrp);
    w.addViewpoint(Q("Teste · Gravidade invertida"), Vec3(OX + 38, 1.65f, -50), -90.f, 18.f, false, kGrp);
    w.addViewpoint(Q("Teste · Chaminé (escalada)"), Vec3(OX - 33.5f, 1.65f, -43.f), 0.f, 62.f, false, kGrp);
    w.addViewpoint(Q("Teste · Topo da chaminé"), Vec3(OX - 36, 20.65f, -42), 0.f, -8.f, false, kGrp);
    w.addViewpoint(Q("Teste · Vista aérea (voo livre)"), Vec3(OX + 8, 26.f, 112), 0.f, -22.f, true, kGrp);

    w.addRoutePoint(Q("encosta"), Vec3(OX, 1.f, 80));
    w.addRoutePoint(Q("boca"), Vec3(OX, 1.f, 36));
    w.addRoutePoint(Q("hub"), Vec3(OX, 1.f, -12));
    w.addRoutePoint(Q("beira do poço (atrás do gelo)"), Vec3(OX + 31, 1.f, -12));
    w.addRoutePoint(Q("sala de lava"), Vec3(OX, 1.f, -48));
    w.addRoutePoint(Q("sala de gravidade"), Vec3(OX + 40, 1.f, -50));
    w.addRoutePoint(Q("base da chaminé"), Vec3(OX - 36, 1.f, -43));
    w.addRoutePoint(Q("topo da chaminé"), Vec3(OX - 36, 20.2f, -43));

    if (w.testLevelOnly || !w.hasSpawn()) w.setSpawn(Vec3(OX, 1.65f, 84), 0.f, 6.f);   // nao sobrescreve o spawn do dungeon
    if (w.testLevelOnly) w.setOverview(Vec3(OX + 5, 10, -20), 85.f);
}
