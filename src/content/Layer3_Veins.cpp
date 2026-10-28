// CAMADA 3 — As Veias Vitreas (dominio de Kaelen): geometria dos tubos, zonas, pontos de vista e rota.
// Os adereços e a jogabilidade de cada area ficam em Layer3_Camp.cpp, Layer3_Vents.cpp, Layer3_Fungus.cpp e Layer3_Fissure.cpp.
// Ver docs/MASMORRA.md (CAMADA 3). Usa so a API de World.
#include "Dungeon.h"
#include "world/World.h"
#include "Prefabs.h"
#include "Layer2_Util.h"
#include "Layer3_Parts.h"

#define Q(s) QStringLiteral(s)
using namespace L23;

namespace L3 {
float domeCeilingY(float x, float z)
{
    float rho = std::hypot(x - DC.x(), z - DC.z());
    float f = std::max(0.f, 1.f - (rho / kDomeR) * (rho / kDomeR));
    return kDomeFloor + kDomeH * std::sqrt(f);
}
} // namespace L3

void buildLayer3Veins(World& w)
{
    using namespace L3;
    const QString kSess = Q("Sessão 3 · Nível 7→8");
    const QString kGrp = Q("Camada 3 · As Veias Vítreas");

    // ============================================================== GEOMETRIA
    // ---- 3.1 Tubo A: do fim do corredor termico ate o portao do entroncamento ----
    tun(w, {V3(-22.f, -178.f, -181.6f), V3(-48.f, -180.f, -196.f), V3(-62.f, -182.f, -220.f), V3(-69.f, -185.2f, -233.f), V3(-70.6f, -186.f, -241.f)},
        3.6f, MAT_OBSIDIAN, 0.4f, 1.5f);
    // ---- 3.2 O Entroncamento: cupula colossal (r 22, teto 20 m) ----
    w.carveChamber(DC, V3(kDomeR, kDomeH, kDomeR), kDomeFloor, MAT_OBSIDIAN, 0.65f, 2.f, Q("entroncamento"));
    w.scatterStalactites(Aabb(V3(DC.x() - 18.f, kDomeFloor + 12.f, DC.z() - 18.f), V3(DC.x() + 18.f, kDomeFloor + 17.f, DC.z() + 18.f)), 38, 77, 2.f, 7.f, 0.3f, 0.9f, MAT_OBSIDIAN);
    // ---- 3.3 Tubo D (leste, beco de magma colapsado) + chamine 1 + duto de respiro + chamine 2 ----
    tun(w, {V3(-55.f, -186.f, -257.5f), V3(-47.f, -186.f, -260.f), V3(-38.f, -186.f, -262.f)}, 3.6f, MAT_OBSIDIAN, 0.4f, 1.5f);
    w.fill(Prim::ellipsoid(V3(-33.8f, -184.5f, -263.f), V3(2.0f, 3.2f, 3.4f)).smooth(0.8f).noise(0.4f).mat(MAT_HOT_BASALT));    // magma colapsado
    w.carve(Prim::cylinder(V3(-36.f, 0.f, -263.f), 1.9f, -186.f, -169.5f).smooth(0.8f).noise(0.28f).mat(MAT_OBSIDIAN).named(Q("chamine_1")));
    w.carveChamber(V3(-36.f, -167.8f, -263.f), V3(3.2f, 3.f, 3.2f), -170.f, MAT_OBSIDIAN, 0.3f, 1.f, Q("topo_chamine_1"));
    tunR(w, {V3(-36.f, -170.f, -263.f), V3(-55.f, -164.f, -258.f), V3(-75.f, -163.f, -255.f), V3(-86.f, -163.5f, -252.5f), V3(-95.f, -165.f, -262.f), V3(-95.f, -165.f, -275.f), V3(-92.f, -165.f, -281.f)},
         {1.05f}, MAT_OBSIDIAN, 0.12f, 0.6f);
    {   // Olhos do Respiro: tres buracos verticais do duto ate o teto da cupula
        const float ex[3] = {-62.f, -75.f, -86.f}, ez[3] = {-257.f, -255.f, -252.f};
        for (int i = 0; i < 3; ++i)
            w.carve(Prim::capsule(V3(ex[i], -163.3f, ez[i]), V3(ex[i], domeCeilingY(ex[i], ez[i]) - 1.5f, ez[i]), 0.55f, 0.55f).smooth(0.3f).noise(0.1f).mat(MAT_OBSIDIAN).named(Q("olho_respiro")));
    }
    w.carve(Prim::cylinder(V3(-92.f, 0.f, -283.f), 1.6f, -188.f, -163.8f).smooth(0.8f).noise(0.25f).mat(MAT_OBSIDIAN).named(Q("chamine_2")));
    tun(w, {V3(-92.f, -188.f, -283.f), V3(-84.f, -187.6f, -285.5f)}, 2.2f, MAT_OBSIDIAN, 0.3f, 1.f);
    // ---- 3.4 Tubo B (oeste) + Bosque Miconida ----
    tun(w, {V3(-95.f, -186.f, -255.f), V3(-115.f, -188.f, -250.f), V3(-132.f, -189.f, -258.f), V3(-141.f, -190.f, -263.f)}, 3.4f, MAT_OBSIDIAN, 0.45f, 1.5f);
    w.carveChamber(V3(-145.f, -186.5f, -265.f), V3(10.f, 7.5f, 10.f), -190.f, MAT_OBSIDIAN, 0.8f, 2.f, Q("bosque_miconida"));
    // ---- 3.5 Tubo C (norte) e a Fenda da Guerra Elemental ----
    tun(w, {V3(-77.f, -186.f, -274.f), V3(-82.f, -188.f, -292.f), V3(-85.f, -189.f, -300.f)}, 3.6f, MAT_OBSIDIAN, 0.4f, 1.5f);
    {
        Vec3 S = kFissS + V3(0, 0, 1.5f), E = kFissE + V3(0, 0, -1.5f);
        Vec3 d = E - S; float horiz = std::hypot(d.x(), d.z());
        float yawF = yawDir(d.x(), d.z()), pitchF = deg(std::atan2(d.y(), horiz));
        QQuaternion q = QQuaternion::fromAxisAndAngle(0, 1, 0, yawF) * QQuaternion::fromAxisAndAngle(1, 0, 0, pitchF);
        Vec3 up = q.rotatedVector(V3(0, 1, 0)), right = q.rotatedVector(V3(1, 0, 0));
        Vec3 mid = (S + E) * 0.5f;
        float halfLen = d.length() * 0.5f + 0.5f;
        w.carve(Prim::box(mid + up * 15.f - right * 1.0f, V3(2.1f, 15.f, halfLen), q, 1.2f).smooth(0.6f).noise(0.45f).mat(MAT_HOT_BASALT).named(Q("fenda_oeste")));
        w.carve(Prim::box(mid + up * 15.f + right * 1.0f, V3(2.1f, 15.f, halfLen), q, 1.2f).smooth(0.6f).noise(0.45f).mat(MAT_ICE).named(Q("fenda_leste")));
    }
    // ---- descida para o labirinto: fim em (-92,-200,-345), penetra ~3 m mais ao norte (-Z) ----
    tun(w, {V3(-90.f, -194.f, -335.f), V3(-94.f, -197.f, -339.f), V3(-92.f, -200.f, -345.f), V3(-92.f, -200.f, -348.f)}, 2.1f, MAT_OBSIDIAN, 0.3f, 1.2f);

    // ============================================================== PROPS, CRIATURAS, ZONAS DE CADA AREA
    buildKaelenCamp(w);
    buildVents(w);
    buildFungus(w);
    buildFissure(w);

    // ============================================================== ZONAS DE AREA (banners, temperatura, nevoa)
    {
        ZoneEffects base;
        base.areaName = Q("Camada 3 · As Veias Vítreas"); base.subtitle = kSess; base.temperature = 8.f;
        base.fogColor = V3(0.006f, 0.008f, 0.014f); base.fogDensity = 0.018f;
        base.ambientColor = V3(0.024f, 0.030f, 0.050f); base.ambientGround = V3(0.012f, 0.012f, 0.016f);
        w.addZone(Q("veias_base"), ZoneShape::makeBox(V3(-152.f, -204.f, -350.f), V3(-20.f, -160.f, -176.f)), base, 1);
        ZoneEffects a = base;
        a.areaName = Q("3.1 · Tubo A"); a.subtitle = Q("Tubo de magma de obsidiana · marcas de arrasto");
        const Vec3 ac[3] = {V3(-35.f, -179.f, -189.f), V3(-55.f, -181.f, -208.f), V3(-65.f, -184.f, -228.f)};
        for (int i = 0; i < 3; ++i) w.addZone(Q("tubo_a_%1").arg(i), ZoneShape::makeSphere(ac[i], 13.f), a, 4);
        ZoneEffects d = base;
        d.areaName = Q("3.2 · O Entroncamento de Kaelen"); d.subtitle = Q("Cúpula colossal · chama fria e jaulas"); d.temperature = 6.f;
        d.fogColor = V3(0.008f, 0.014f, 0.030f); d.fogDensity = 0.014f;
        d.ambientColor = V3(0.030f, 0.050f, 0.090f); d.ambientGround = V3(0.014f, 0.022f, 0.040f);
        w.addZone(Q("entroncamento"), ZoneShape::makeCylinder(V3(DC.x(), 0.f, DC.z()), 22.6f, -187.f, -164.f), d, 5);
        ZoneEffects tc = base;
        tc.areaName = Q("3.5 · Tubo C"); tc.subtitle = Q("Para o norte, em direção à fenda");
        w.addZone(Q("tubo_c"), ZoneShape::makeBox(V3(-92.f, -192.f, -297.f), V3(-70.f, -181.f, -272.f)), tc, 4);
        ZoneEffects ds = base;
        ds.areaName = Q("Descida para o Labirinto"); ds.subtitle = Q("Zumbido psíquico · fim da Camada 3"); ds.psychic = 0.22f;
        ds.fogColor = V3(0.012f, 0.008f, 0.020f);
        w.addZone(Q("descida"), ZoneShape::makeSphere(V3(-92.f, -197.f, -341.f), 8.f), ds, 6);
    }

    // ============================================================== POI do Tubo A
    {
        float yA = groundY(w, -46.f, -179.5f, -195.f);
        w.addPoi(V3(-46.f, yA + 1.4f, -195.f), PoiCategory::Area, Q("3.1 · Tubo A (de chegada)"),
            Q("De (−24, −178, −182) a (−48, −180, −196), (−62, −182, −220) e o portão do entroncamento (−70, −184, −236). Antigo tubo de magma de obsidiana negra polida, seção circular de ~7 m, chão plano coberto de cinzas petrificadas. Reflexos vítreos na luz da tocha: tudo parece espelho escuro.\n\n"
              "Perceba: ar quente e seco depois do poço (8 °C), cheiro de ferro e cinza. A luz da tocha vai longe nas paredes lisas — quem está na frente é visto de muito longe.\n"
              "Primeiros sinais de Kaelen: marcas de arrasto no chão de cinza, um sapato infantil, correntes cortadas. Sobrevivência CD 11 (rastreamento): 4–6 pessoas foram arrastadas, uma delas pequena e descalça.\n"
              "Percepção passiva 13: um fio de arame com latas, esticado entre as paredes a 8 m do portão — é o alarme de Kaelen."), kSess, kGrp);
    }

    // ============================================================== PONTOS DE VISTA
    auto eye = [&](float x, float yHint, float z) { return V3(x, groundY(w, x, yHint, z) + 1.65f, z); };
    w.addViewpoint(Q("3.1 Tubo A (chegada)"), eye(-40.f, -179.f, -191.f), yawToward(V3(-40, 0, -191), V3(-62, 0, -220)), 0.f, false, kGrp);
    w.addViewpoint(Q("3.1 Marcas de arrasto e sapato"), eye(-48.5f, -180.f, -196.5f), yawToward(V3(-48.5f, 0, -196.5f), V3(-62, 0, -220)), -14.f, false, kGrp);
    w.addViewpoint(Q("3.1 Portão do Entroncamento"), eye(-68.5f, -184.f, -231.f), yawToward(V3(-68.5f, 0, -231), V3(-72, 0, -255)), 4.f, false, kGrp);
    w.addViewpoint(Q("3.2 O Entroncamento de Kaelen"), eye(-71.f, -186.f, -243.f), yawToward(V3(-71, 0, -243), V3(-75, 0, -256)), 12.f, false, kGrp);
    w.addViewpoint(Q("3.2 Jaulas penduradas"), eye(-73.f, -186.f, -245.f), yawToward(V3(-73, 0, -245), V3(-76.5f, 0, -248.5f)), 24.f, false, kGrp);
    w.addViewpoint(Q("3.2 Braseiro de chama fria"), eye(-75.f, -186.f, -249.f), yawToward(V3(-75, 0, -249), V3(-75, 0, -255)), 2.f, false, kGrp);
    w.addViewpoint(Q("3.2 Posto de comando e Caderneta"), eye(-77.f, -186.f, -255.5f), yawToward(V3(-77, 0, -255.5f), V3(-80.5f, 0, -262.5f)), 4.f, false, kGrp);
    w.addViewpoint(Q("3.2 Kaelen (de frente)"), eye(-76.f, -186.f, -257.f), yawToward(V3(-76, 0, -257), V3(-81, 0, -262.5f)), 6.f, false, kGrp);
    w.addViewpoint(Q("3.2 Canil dos hell hounds"), eye(-71.5f, -186.f, -259.f), yawToward(V3(-71.5f, 0, -259), V3(-65.5f, 0, -266)), 2.f, false, kGrp);
    w.addViewpoint(Q("3.2 Baú de Kaelen"), eye(-80.f, -186.f, -254.f), yawToward(V3(-80, 0, -254), V3(-84, 0, -258)), -6.f, false, kGrp);
    w.addViewpoint(Q("3.2 Paliçada e costelas (de fora)"), eye(-70.f, -186.f, -236.5f), yawToward(V3(-70, 0, -236.5f), V3(-90, 0, -246)), 6.f, false, kGrp);
    w.addViewpoint(Q("3.3 Beco de magma (Tubo D)"), eye(-47.f, -186.f, -260.f), yawToward(V3(-47, 0, -260), V3(-37, 0, -262)), 0.f, false, kGrp);
    w.addViewpoint(Q("3.3 Base da chaminé (16 m)"), eye(-36.f, -186.f, -261.f), yawToward(V3(-36, 0, -261), V3(-36, 0, -263)), 72.f, false, kGrp);
    w.addViewpoint(Q("3.3 Topo da chaminé e boca do duto"), V3(-36.f, -170.f + 1.65f, -261.5f), yawToward(V3(-36, 0, -261.5f), V3(-55, 0, -258)), -2.f, false, kGrp);
    w.addViewpoint(Q("3.3 Duto de respiro (rastejando)"), V3(-56.f, -164.f + 0.85f, -257.8f), yawToward(V3(-56, 0, -257.8f), V3(-75, 0, -255)), 0.f, false, kGrp);
    w.addViewpoint(Q("3.3 Olho do Respiro (olhando o acampamento)"), V3(-75.f, -163.f + 0.85f, -255.f), 0.f, -88.f, false, kGrp);
    w.addViewpoint(Q("3.4 Tubo B (fungos)"), eye(-112.f, -188.f, -250.5f), yawToward(V3(-112, 0, -250.5f), V3(-132, 0, -258)), 0.f, false, kGrp);
    w.addViewpoint(Q("3.4 Bosque Micônida"), eye(-135.f, -189.5f, -259.f), yawToward(V3(-135, 0, -259), V3(-146, 0, -267)), 2.f, false, kGrp);
    w.addViewpoint(Q("3.5 Tubo C"), eye(-80.f, -187.f, -283.f), yawToward(V3(-80, 0, -283), V3(-85, 0, -300)), 0.f, false, kGrp);
    w.addViewpoint(Q("3.5 Início da Fenda da Guerra Elemental"), eye(-85.5f, -189.f, -300.5f), yawToward(V3(-85.5f, 0, -300.5f), V3(-90, 0, -335)), 28.f, false, kGrp);
    w.addViewpoint(Q("3.5 Fenda (meio, mephits no alto)"), eye(-87.f, -191.f, -317.f), yawToward(V3(-87, 0, -317), V3(-90, 0, -335)), 38.f, false, kGrp);
    w.addViewpoint(Q("3.5 Descida para o Labirinto"), eye(-88.5f, -193.f, -327.f), yawToward(V3(-88.5f, 0, -327), V3(-92, 0, -343)), -16.f, false, kGrp);
    w.addViewpoint(Q("Voo · Cúpula e Olhos do Respiro (por baixo)"), V3(-78.f, -178.f, -246.f), yawToward(V3(-78, 0, -246), V3(-75, 0, -256)), 52.f, true, kGrp);
    w.addViewpoint(Q("Voo · Olhos do Respiro no teto da cúpula"), V3(-66.f, -180.f, -242.f), yawToward(V3(-66, 0, -242), V3(-75, 0, -255)), 48.f, true, kGrp);
    w.addViewpoint(Q("Voo · Entroncamento visto do alto"), V3(-75.f, -177.f, -238.f), yawToward(V3(-75, 0, -238), V3(-75, 0, -262)), -12.f, true, kGrp);

    // ============================================================== ROTA (continua a partir de "Saida do Corredor Termico")
    w.addRoutePoint(Q("Tubo A (meio)"), V3(-48.f, -179.4f, -196.f));
    w.addRoutePoint(Q("Tubo A (curva)"), V3(-62.f, -181.4f, -220.f));
    w.addRoutePoint(Q("Portão do Entroncamento"), V3(-70.f, -185.4f, -236.f));
    w.addRoutePoint(Q("Entroncamento · portão da paliçada"), V3(-71.5f, -185.4f, -243.f));
    w.addRoutePoint(Q("Entroncamento · pátio (jaulas)"), V3(-73.f, -185.4f, -249.f));
    w.addRoutePoint(Q("Entroncamento · braseiro frio"), V3(-71.5f, -185.4f, -258.f));
    w.addRoutePoint(Q("Entroncamento · portão norte"), V3(-77.f, -185.4f, -270.f));
    w.addRoutePoint(Q("Tubo C"), V3(-79.f, -185.4f, -279.f));
    w.addRoutePoint(Q("Tubo C (curva)"), V3(-82.f, -187.4f, -292.f));
    w.addRoutePoint(Q("Início da Fenda"), V3(-85.f, -188.4f, -300.f));
    w.addRoutePoint(Q("Fenda da Guerra Elemental"), V3(-87.5f, -190.9f, -317.5f));
    w.addRoutePoint(Q("Fim da Fenda"), V3(-90.f, -193.4f, -334.5f));
    w.addRoutePoint(Q("Entrada do Labirinto"), V3(-92.f, -199.4f, -345.f));
}
