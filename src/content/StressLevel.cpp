// Teste de carga (--stress): geometria sintetica na escala da masmorra real (~1,6 km de tuneis,
// poco de 166 m, geodo de 160 m, ~300x370x625 m) para medir a malhagem. NAO e conteudo do jogo.
#include "Dungeon.h"
#include "world/World.h"

void buildStressLevel(World& w)
{
    auto H = [](float x, float z) {
        float dx = std::fabs(x);
        return -0.04f * z + 0.7f * std::sin(x * 0.21f) * std::cos(z * 0.17f) + smoothstepf(35.f, 45.f, dx) * 60.f + smoothstepf(10.f, -6.f, z) * 70.f;
    };
    w.setTerrain(H, Aabb(Vec3(-60, -8, -8), Vec3(60, 70, 110)));
    const Vec3 route[] = {{0, 0, 40}, {0, -1, -17}, {0, -3, -50}, {0, -4, -77}, {0, -5, -98}, {0, -6, -120}, {6, -8, -140}, {18, -13, -157}, {27, -19, -167}};
    std::vector<Vec3> p1(std::begin(route), std::end(route));
    for (auto& p : p1) p.setY(p.y() + 1.6f);
    TunnelOpts o; o.noise = 0.5f;
    w.carveTunnel(p1, 3.f, o);
    w.carveChamber(Vec3(0, 4, -14), Vec3(11, 5, 14), -1.f);
    w.carveChamber(Vec3(0, -1, -98), Vec3(18, 5, 14), -5.f);
    // poco
    w.carve(Prim::cylinder(Vec3(40, 0, -180), 22.f, -178.f, -12.f).mat(MAT_LIMESTONE));
    HelixDesc hd; hd.center = Vec3(40, 0, -180); hd.rInner = 19.f; hd.rOuter = 23.f; hd.startY = -19.f; hd.dropPerTurn = 32.f; hd.startAngleDeg = 133.f; hd.t1 = 4.9f; hd.thickness = 3.f;
    w.fill(Prim::helix(hd).noise(0.3f));
    w.scatterStalactites(Aabb(Vec3(18, -60, -202), Vec3(62, -12, -158)), 120, 5, 2.f, 6.f);
    // veias, labirinto, geodo, parasita
    std::vector<Vec3> p2 = {{18, -176, -180}, {-24, -176.5f, -182}, {-62, -180, -220}, {-75, -184, -255}, {-82, -186, -292}, {-90, -192, -335}, {-92, -198, -345}, {-92, -200, -370}, {-70, -203, -392}, {-36, -212, -383}};
    w.carveTunnel(p2, 3.5f, o);
    for (int i = 0; i < 12; ++i) {   // labirinto
        float x = -125 + (i % 4) * 18, z = -350 - (i / 4) * 15;
        w.carveTunnel({Vec3(x, -198, z), Vec3(x + 18, -198, z), Vec3(x + 18, -198, z - 15)}, 1.8f, o);
    }
    w.carveChamber(Vec3(-75, -176, -255), Vec3(22, 12, 22), -186.f);
    w.carveChamber(Vec3(20, -232, -370), Vec3(80, 26, 55), -255.f, MAT_GEODE, 0.6f, 4.f);
    w.scatterStalactites(Aabb(Vec3(-50, -255, -420), Vec3(90, -210, -320)), 250, 9, 3.f, 10.f, 0.4f, 1.2f, MAT_GEODE);
    w.scatterStalagmites(Aabb(Vec3(-50, -256, -420), Vec3(90, -230, -320)), 250, 10, 3.f, 12.f, 0.5f, 1.6f, MAT_GEODE);
    std::vector<Vec3> p3 = {{0, -255, -355}, {35, -255, -395}, {80, -255, -405}, {98, -255, -408}, {115, -265, -430}, {110, -290, -470}, {100, -300, -480}};
    for (auto& p : p3) p.setY(p.y() + 1.6f);
    w.carveTunnel(p3, 3.5f, o);
    w.carveChamber(Vec3(100, -275, -505), Vec3(48, 28, 45), -300.f, MAT_HOT_BASALT, 0.5f, 4.f);
    w.scatterStalactites(Aabb(Vec3(60, -300, -540), Vec3(140, -250, -470)), 100, 11, 3.f, 9.f, 0.4f, 1.1f, MAT_HOT_BASALT);
    // tuneis extra para alcancar ~1,6 km
    for (int i = 0; i < 24; ++i) {
        float a = i * 0.9f;
        Vec3 s(-60 + 120 * std::sin(a), -100 - i * 4.f, -250 - 100 * std::cos(a));
        w.carveTunnel({s, s + Vec3(30, -3, 25), s + Vec3(60, -2, 0), s + Vec3(70, 4, -30)}, 2.2f, o);
    }
    w.setSpawn(Vec3(0, 1.65f, 40), 0.f);
    w.addViewpoint(QStringLiteral("Stress · boca"), Vec3(0, 1.65f, 60), 0.f, 5.f, false);
    w.addViewpoint(QStringLiteral("Stress · geodo"), Vec3(-36, -210, -383), 90.f, -10.f, true);
    w.addViewpoint(QStringLiteral("Stress · poço"), Vec3(40, -15, -160), 0.f, -50.f, true);
    w.addViewpoint(QStringLiteral("Stress · câmara do coração"), Vec3(100, -296, -470), 0.f, 0.f, true);
}
