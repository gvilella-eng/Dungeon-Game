// Camada 1 — geometria de rocha (SDF): antecamara, corredor, abismo, toca, labirinto, sifao, cascata, fronteira.
#include "Layer1_Common.h"

namespace L1 {

// nos do Labirinto dos Leitos Secos (pontos no chao)
static const Vec3 N[11] = {
    Vec3(0, -5, -112), Vec3(0, -6, -120), Vec3(-15, -7, -126), Vec3(-26, -7, -132), Vec3(12, -7, -127), Vec3(22, -8, -122),
    Vec3(-6, -8, -136), Vec3(-16, -9, -146), Vec3(6, -8, -140), Vec3(12, -10, -150), Vec3(18, -13, -157)};

// Liga dois nos por um leito seco sinuoso (pontos intermediarios deslocados lateralmente).
static void labEdge(World& w, int a, int b, int seed, float r0 = 2.2f, float r1 = 2.3f)
{
    Vec3 A = N[a], B = N[b];
    Vec3 d = B - A; d.setY(0);
    float L = d.length();
    Vec3 side(d.z() / L, 0, -d.x() / L);
    int mids = L > 11.f ? 2 : 1;
    std::vector<Vec3> pts; std::vector<float> rad;
    pts.push_back(A); rad.push_back(r0);
    for (int i = 1; i <= mids; ++i) {
        float t = (float)i / (mids + 1);
        Vec3 p = A + (B - A) * t;
        float off = (hrand(seed, i) - 0.5f) * 5.2f;
        p += side * off;
        pts.push_back(p); rad.push_back(2.0f + hrand(seed, i, 9) * 0.6f);
    }
    pts.push_back(B); rad.push_back(r1);
    tun(w, pts, rad, MAT_LIMESTONE, 0.5f, 0.8f);
}

static void nodeBulge(World& w, int n, Vec3 radii, int mat = MAT_LIMESTONE)
{
    Vec3 c = N[n] + Vec3(0, radii.y() * 0.55f, 0);
    w.carveChamber(c, radii, N[n].y(), mat, 0.5f, 1.0f, Q("no_%1").arg(n));
}

void buildGeometry(World& w)
{
    const int LS = MAT_LIMESTONE, WET = MAT_WET_LIMESTONE;

    // ================================================================= 1.1 ANTECAMARA DO FALSO RITUAL
    // x -11..11, z 0..-28, altura 9 m, chao de 0 a -2
    room(w, V(0, -0.25f, -4.5f), V(0, -2.0f, -23.5f), 11.f, 9.f, WET, 0.55f, 11, 0.8f);
    tun(w, {V(-9.5f, -0.6f, -8.f), V(-13.f, -0.7f, -8.6f)}, 2.4f, WET, 0.5f, 1.0f);      // nichos laterais
    tun(w, {V(9.5f, -0.8f, -12.5f), V(12.5f, -0.9f, -13.f)}, 2.2f, WET, 0.5f, 1.0f);
    tun(w, {V(-9.f, -1.5f, -20.f), V(-12.5f, -1.5f, -21.f)}, 2.6f, WET, 0.5f, 1.0f);

    // ================================================================= 1.2 CORREDOR DAS FENDAS
    room(w, V(0, -2.0f, -28.f), V(0, -4.0f, -69.f), 2.5f, 8.f, LS, 0.42f, 21, 0.6f);
    // bolsoes laterais (reentrancias) para o corredor nao parecer um tubo
    tun(w, {V(-2.2f, corrFloor(-36), -36.f), V(-3.6f, corrFloor(-36), -36.f)}, 1.5f, LS, 0.4f, 1.f);
    tun(w, {V(2.2f, corrFloor(-47.f), -50.f), V(3.4f, corrFloor(-47.f), -49.f)}, 1.3f, LS, 0.4f, 1.f);
    tun(w, {V(-2.2f, corrFloor(-66.f), -66.f), V(-3.6f, corrFloor(-66.f), -66.5f)}, 1.4f, LS, 0.4f, 1.f);
    // Fosso de Estacas Coberto: z -46..-49, x -1.5..1.5, 3 m de fundo
    {
        float fy = corrFloor(-47.5f);
        w.carve(Prim::box(V(0, fy - 1.5f, -47.5f), V(1.5f, 1.5f, 1.5f), QQuaternion(), 0.08f).noise(0.05f).mat(LS).named(Q("fosso_estacas")));
    }
    // Seteiras do corredor (fendas 0,6 x 0,5 m a 5 m do chao): oeste x -2,5 em z -34,-46,-58; leste x +2,5 em z -40,-52,-64
    for (float z : {-34.f, -46.f, -58.f}) {
        float y = corrFloor(z) + 4.8f + 0.55f;
        w.carve(Prim::box(V(-3.3f, y, z), V(1.3f, 0.25f, 0.3f), QQuaternion(), 0.04f).noise(0.f).mat(LS).named(Q("seteira")));
    }
    for (float z : {-40.f, -52.f, -64.f}) {
        float y = corrFloor(z) + 4.8f + 0.55f;
        w.carve(Prim::box(V(3.3f, y, z), V(1.3f, 0.25f, 0.3f), QQuaternion(), 0.04f).noise(0.f).mat(LS).named(Q("seteira")));
    }
    // Seteiras da antecamara (parede leste x +11): (11,+4,-14) e (11,+4,-22)
    for (float z : {-14.f, -22.f})
        w.carve(Prim::box(V(11.9f, 4.05f, z), V(1.5f, 0.25f, 0.3f), QQuaternion(), 0.04f).noise(0.f).mat(LS).named(Q("seteira")));

    // Tuneis de rastejo goblin (1,0 m de altura, ~0,9 m de largura)
    {
        std::vector<Vec3> east = {V(13.2f, 3.55f, -12.f), V(13.2f, 3.55f, -24.5f), V(9.5f, 3.2f, -28.8f), V(4.2f, 2.66f, -31.f),
                                  V(4.2f, 2.23f, -40.f), V(4.2f, 1.66f, -52.f), V(4.2f, 1.09f, -64.f), V(4.2f, 0.9f, -68.f),
                                  V(11.f, -0.2f, -67.4f), V(18.f, -1.2f, -68.2f), V(24.f, -2.f, -69.6f), V(24.f, -2.f, -70.9f)};
        tun(w, east, 0.62f, LS, 0.06f, 0.4f);
        std::vector<Vec3> west = {V(-4.2f, 2.66f, -30.f), V(-4.2f, 2.51f, -34.f), V(-4.2f, 1.94f, -46.f), V(-4.2f, 1.37f, -58.f),
                                  V(-4.2f, 0.9f, -68.f), V(-6.f, 0.4f, -69.2f)};
        tun(w, west, 0.62f, LS, 0.06f, 0.4f);
        // Ninho de atirador (-8,0,-70): bolsao com janela para o abismo e a ponte
        w.carveChamber(V(-7.8f, 0.85f, -69.2f), V(1.5f, 1.1f, 1.4f), 0.0f, LS, 0.1f, 0.4f, Q("ninho"));
        w.carve(Prim::box(V(-8.f, 0.8f, -70.3f), V(1.0f, 0.55f, 0.9f), QQuaternion(), 0.05f).noise(0.f).mat(LS));
        // alcova de dormir no inicio do tunel oeste
        w.carveChamber(V(-4.6f, 3.3f, -29.2f), V(1.3f, 0.9f, 1.3f), 2.62f, LS, 0.08f, 0.4f, Q("alcova_rastejo"));
    }

    // ================================================================= 1.3 ABISMO DO RIO VIOLENTO
    // x -32..28, z -70..-84, do chao (y -4) ao leito (y -21); sumidouro em x 28
    w.carve(Prim::box(V(-2.f, -8.f, -77.f), V(30.f, 13.f, 7.f), QQuaternion(), 1.6f).floorAt(-21.f).smooth(1.5f).noise(0.85f).mat(WET).named(Q("abismo")));
    // reentrancias nas paredes do abismo (sem tocar a ponte, o tronco ou o rastejo)
    w.carve(Prim::ellipsoid(V(-24.f, -13.f, -69.4f), V(5.f, 6.5f, 2.2f)).floorAt(-21.f).smooth(2.f).noise(0.6f).mat(WET));
    w.carve(Prim::ellipsoid(V(15.f, -14.f, -69.2f), V(6.f, 7.f, 2.2f)).floorAt(-21.f).smooth(2.f).noise(0.6f).mat(WET));
    w.carve(Prim::ellipsoid(V(8.f, -16.f, -84.8f), V(6.f, 5.f, 1.8f)).floorAt(-21.f).smooth(2.f).noise(0.6f).mat(WET));
    // sumidouro: buraco na base da parede leste
    w.carve(Prim::capsule(V(26.f, -19.2f, -77.f), V(32.f, -19.6f, -77.f), 2.2f, 2.4f).smooth(1.f).noise(0.4f).mat(WET).named(Q("sumidouro")));
    // patamares junto das cabeceiras da ponte (chao y -4)
    w.fill(Prim::box(V(0, -5.5f, -70.4f), V(3.1f, 1.5f, 1.5f), QQuaternion(), 0.25f).noise(0.1f).mat(LS));
    w.fill(Prim::box(V(0, -5.5f, -83.4f), V(3.1f, 1.5f, 1.5f), QQuaternion(), 0.25f).noise(0.1f).mat(LS));
    // leito raso sob a ponte (onde ficam as estacas): topo em y -19.2
    w.fill(Prim::box(V(0, -20.5f, -77.f), V(5.8f, 1.3f, 5.6f), QQuaternion(), 0.8f).noise(0.2f).mat(WET));
    // matacoes no leito
    w.fill(Prim::ellipsoid(V(-20.f, -20.6f, -75.f), V(2.6f, 1.6f, 2.f)).noise(0.2f).smooth(0.8f).mat(WET));
    w.fill(Prim::ellipsoid(V(-12.f, -20.8f, -80.5f), V(2.f, 1.3f, 1.8f)).noise(0.2f).smooth(0.8f).mat(WET));
    w.fill(Prim::ellipsoid(V(11.f, -20.7f, -73.5f), V(2.4f, 1.5f, 1.9f)).noise(0.2f).smooth(0.8f).mat(WET));
    w.fill(Prim::ellipsoid(V(18.f, -20.8f, -81.f), V(2.f, 1.4f, 1.8f)).noise(0.2f).smooth(0.8f).mat(WET));
    // trilha de cabras: rampa estreita junto a parede norte, do leito oeste ate a cabeceira norte da ponte
    {
        Vec3 a(-31.f, -20.0f, -82.4f), b(-6.f, -4.6f, -82.4f);
        Vec3 dir = (b - a).normalized();
        QQuaternion q = QQuaternion::rotationTo(Vec3(1, 0, 0), dir);
        float len = (b - a).length();
        Vec3 up = q.rotatedVector(Vec3(0, 1, 0));
        w.fill(Prim::box((a + b) * 0.5f - up * 0.8f, V(len * 0.5f + 0.5f, 0.8f, 1.35f), q, 0.4f).noise(0.25f).mat(WET).named(Q("trilha_cabras")));
    }

    // ================================================================= tronco-estalactite (secao 1.5) e Linha Goblin
    // tronco (raio ~0,6) de (-20,-4,-70) a (-20,-5,-84): topo andavel em y -4 -> -5
    w.fill(Prim::cone(V(-20.f, -5.55f, -84.4f), V(-20.f, -4.65f, -69.6f), 0.85f, 0.48f).noise(0.08f).smooth(0.3f).mat(LS).named(Q("tronco")));

    // ================================================================= 1.4 A TOCA
    // x -18..18 (lobo oeste ate -24), z -86..-112, chao -5, altura 7
    room(w, V(0, -5.f, -89.5f), V(0, -5.f, -108.5f), 18.f, 7.f, WET, 0.5f, 41, 0.8f);
    // lobo oeste (por baixo da galeria dos arqueiros), conectado ao abismo
    room(w, V(-20.5f, -5.f, -88.f), V(-20.5f, -5.f, -86.f), 3.5f, 5.f, WET, 0.45f, 43, 0.7f);
    // cabeceira norte da ponte -> toca
    tun(w, {V(0, -4.f, -83.5f), V(0, -4.5f, -85.5f), V(0, -5.f, -88.f)}, 2.2f, LS, 0.35f, 0.8f);
    // galeria alta oeste: plataforma x -18..-12, topo y -1, z -88..-108
    w.fill(Prim::box(V(-15.f, -2.2f, -98.f), V(3.2f, 1.3f, 10.2f), QQuaternion(), 0.35f).noise(0.12f).mat(LS).named(Q("galeria")));
    // rampa de acesso (rochas caidas) da galeria: do chao da toca em x -6 ate a plataforma em x -12
    {
        Vec3 a(-5.5f, -5.2f, -95.8f), b(-12.5f, -1.0f, -95.8f);
        Vec3 dir = (b - a).normalized();
        QQuaternion q = QQuaternion::rotationTo(Vec3(1, 0, 0), dir);
        Vec3 up = q.rotatedVector(Vec3(0, 1, 0));
        w.fill(Prim::box((a + b) * 0.5f - up * 0.7f, V((b - a).length() * 0.5f, 0.7f, 1.2f), q, 0.35f).noise(0.2f).mat(LS).named(Q("rampa_galeria")));
    }
    // saida norte (0,-5,-112) -> N1 ; saida leste (18,-5,-100) -> sifao ; alcova da Linha Goblin (24,-3,-86)
    tun(w, {V(0, -5.f, -110.f), V(0, -5.1f, -113.5f), V(0, -5.5f, -116.5f), V(0, -6.f, -120.f)}, 2.3f, LS, 0.4f, 0.8f);
    tun(w, {V(16.5f, -5.f, -100.f), V(21.f, -5.f, -99.f), V(26.5f, -5.25f, -96.f)}, 2.0f, WET, 0.4f, 0.8f);
    tun(w, {V(24.f, -3.f, -85.2f), V(24.f, -3.1f, -88.8f), V(20.5f, -4.2f, -92.2f), V(16.5f, -5.f, -94.f)}, 1.9f, WET, 0.4f, 0.8f);
    // bolsoes de dormir e cantos da toca
    w.carveChamber(V(15.5f, -3.4f, -108.f), V(4.f, 3.2f, 4.f), -5.f, WET, 0.5f, 1.0f, Q("toca_ne"));
    w.carveChamber(V(-9.f, -3.6f, -111.5f), V(4.f, 3.f, 3.5f), -5.f, WET, 0.5f, 1.0f, Q("toca_nw"));
    w.carveChamber(V(14.f, -3.6f, -90.5f), V(3.6f, 3.f, 3.f), -5.f, WET, 0.5f, 1.0f, Q("toca_se"));

    // ================================================================= 1.5 A CASCATA CONGELADA
    // abertura de 3 m na parede oeste do corredor em z -40 -> camara lateral (-12,-3,-40), 10 x 8 m, 6 m de altura
    slot(w, {V(-1.8f, corrFloor(-40.f), -40.f), V(-4.5f, -2.8f, -40.f), V(-7.5f, -3.0f, -40.f)}, 1.5f, 3.4f, LS, 0.3f, 0.5f);
    w.carve(Prim::box(V(-12.f, 0.f, -40.2f), V(5.f, 3.f, 4.2f), QQuaternion(), 1.2f).floorAt(-3.f).smooth(1.5f).noise(0.4f).mat(WET).named(Q("camara_cascata")));
    // nicho da cascata (norte) e fenda natural atras do gelo
    w.carve(Prim::box(V(-12.f, 0.3f, -45.2f), V(2.05f, 3.4f, 1.25f), QQuaternion(), 0.3f).floorAt(-3.f).smooth(0.5f).noise(0.2f).mat(WET).named(Q("nicho_gelo")));
    slot(w, {V(-12.f, -3.f, -45.6f), V(-12.6f, -3.05f, -49.f), V(-15.f, -3.3f, -53.f), V(-17.5f, -3.4f, -57.f), V(-15.5f, -3.5f, -61.f),
             V(-17.5f, -3.7f, -65.f), V(-19.5f, -3.9f, -68.5f), V(-20.f, -4.f, -70.8f)}, 0.5f, 2.2f, LS, 0.18f, 0.3f);

    // ================================================================= 1.6 O SIFAO TURVO
    // abertura na parede leste do corredor em z -62 -> poco de agua turva (30,-5,-62)
    tun(w, {V(1.9f, corrFloor(-62.f), -62.f), V(9.f, -3.8f, -62.f), V(15.f, -3.9f, -62.5f), V(21.f, -4.4f, -62.2f), V(25.8f, -5.2f, -62.f)}, 1.55f, WET, 0.4f, 1.2f);
    w.carve(Prim::ellipsoid(V(30.f, -5.f, -62.f), V(3.3f, 5.f, 3.3f)).floorAt(-9.5f).smooth(1.2f).noise(0.4f).mat(WET).named(Q("poco_sifao")));
    // tubo totalmente submerso
    {
        std::vector<Vec3> pts = {V(30.f, -7.5f, -62.f), V(33.5f, -9.f, -64.f), V(36.f, -9.f, -67.f), V(36.5f, -9.2f, -75.f), V(35.5f, -9.f, -83.f),
                                 V(36.f, -9.f, -90.f), V(33.5f, -8.f, -92.5f), V(30.f, -7.5f, -94.f)};
        std::vector<float> r = {1.7f, 1.6f, 1.5f, 1.9f, 1.5f, 1.7f, 1.6f, 1.7f};
        TunnelOpts o; o.floorFrac = 1.f; o.material = WET; o.noise = 0.45f; o.k = 1.5f;
        w.carveTunnel(pts, r, o);
    }
    w.carve(Prim::ellipsoid(V(30.f, -5.f, -94.f), V(3.3f, 5.f, 3.3f)).floorAt(-9.5f).smooth(1.2f).noise(0.4f).mat(WET).named(Q("poco_toca")));

    // ================================================================= 1.7 LABIRINTO DOS LEITOS SECOS
    nodeBulge(w, 1, V(3.6f, 2.4f, 3.6f));
    nodeBulge(w, 2, V(3.6f, 2.5f, 3.6f));
    nodeBulge(w, 3, V(5.4f, 2.9f, 4.8f), WET);       // O Lixao
    nodeBulge(w, 4, V(3.6f, 2.5f, 3.6f));
    nodeBulge(w, 5, V(4.6f, 2.6f, 3.9f), WET);       // A Despensa Congelada
    nodeBulge(w, 6, V(3.4f, 2.4f, 3.4f));
    nodeBulge(w, 7, V(4.6f, 3.0f, 4.8f));            // Galeria Desabada
    nodeBulge(w, 8, V(3.4f, 2.4f, 3.4f));
    nodeBulge(w, 9, V(3.4f, 2.4f, 3.4f));
    nodeBulge(w, 10, V(3.4f, 2.5f, 3.4f));
    labEdge(w, 1, 2, 101); labEdge(w, 2, 3, 102); labEdge(w, 1, 4, 103); labEdge(w, 4, 5, 104);
    labEdge(w, 2, 6, 105); labEdge(w, 6, 7, 106); labEdge(w, 6, 8, 107); labEdge(w, 4, 8, 108);
    labEdge(w, 8, 9, 109); labEdge(w, 9, 10, 110);
    // Esconderijo do Chefe (secreto): bolsao atras de pedras soltas, a oeste do Lixao
    tun(w, {V(-29.5f, -7.05f, -133.3f), V(-32.f, -7.1f, -135.f)}, 1.6f, LS, 0.3f, 1.f);
    w.carveChamber(V(-34.f, -5.6f, -136.5f), V(2.9f, 2.0f, 2.7f), -7.1f, LS, 0.3f, 1.2f, Q("esconderijo"));
    // Galeria Desabada: passagem soterrada por desabamento ao norte do no 7
    tun(w, {V(-16.5f, -9.f, -150.f), V(-17.5f, -9.2f, -155.f), V(-19.f, -9.5f, -161.f)}, 2.1f, LS, 0.4f, 0.8f);
    w.fill(Prim::ellipsoid(V(-17.3f, -8.f, -152.3f), V(2.4f, 2.1f, 1.9f)).noise(0.35f).smooth(0.8f).mat(LS));
    w.fill(Prim::ellipsoid(V(-16.1f, -8.6f, -151.4f), V(1.7f, 1.4f, 1.5f)).noise(0.35f).smooth(0.8f).mat(LS));
    w.fill(Prim::ellipsoid(V(-18.6f, -8.6f, -153.5f), V(1.5f, 1.6f, 1.4f)).noise(0.35f).smooth(0.8f).mat(LS));

    // ================================================================= 1.8 A FRONTEIRA DOS OSSOS
    tun(w, {V(18.f, -13.f, -157.f), V(20.f, -14.5f, -159.5f), V(22.f, -16.f, -162.f), V(24.5f, -17.6f, -164.8f), V(27.f, -19.f, -167.f), V(29.1f, -19.f, -169.1f)},
        {2.4f, 2.3f, 2.5f, 2.4f, 2.4f, 2.3f}, LS, 0.4f, 0.8f);

    // ================================================================= estalactites e estalagmites (depois de todas as escavacoes)
    w.scatterStalactites(Aabb(V(-10, 1, -26), V(10, 9, -2)), 20, 101, 1.5f, 4.5f, 0.25f, 0.7f, WET);
    w.scatterStalagmites(Aabb(V(-10.5f, -3, -26), V(-5, 2, -4)), 6, 102, 0.6f, 1.8f, 0.2f, 0.5f, WET);
    w.scatterStalagmites(Aabb(V(5, -3, -26), V(10.5f, 2, -4)), 6, 103, 0.6f, 1.8f, 0.2f, 0.5f, WET);
    w.scatterStalactites(Aabb(V(-2, 0, -69), V(2, 8, -29)), 12, 104, 0.8f, 2.6f, 0.2f, 0.5f, LS);
    w.scatterStalactites(Aabb(V(-30, -3, -83), V(26, 6, -71)), 16, 105, 1.5f, 5.f, 0.3f, 0.8f, WET);
    w.scatterStalagmites(Aabb(V(-30, -19.5f, -83), V(26, -13, -71)), 8, 106, 1.2f, 3.4f, 0.3f, 0.7f, WET);
    w.scatterStalactites(Aabb(V(-17, -4, -111), V(17, 3, -87)), 16, 107, 0.8f, 2.6f, 0.2f, 0.6f, WET);
    w.scatterStalagmites(Aabb(V(-17, -5.5f, -111), V(17, -1, -87)), 8, 108, 0.5f, 1.4f, 0.2f, 0.5f, WET);
    w.scatterStalactites(Aabb(V(-17, -2, -45), V(-7, 4, -36)), 12, 109, 0.8f, 2.4f, 0.2f, 0.5f, WET);
    w.scatterStalagmites(Aabb(V(-17, -4, -45), V(-7, 1, -36)), 6, 110, 0.5f, 1.3f, 0.2f, 0.45f, WET);
    for (int n = 1; n <= 10; ++n) {
        Vec3 c = N[n];
        w.scatterStalactites(Aabb(c + V(-6, 1.f, -6), c + V(6, 5.5f, 6)), 3, 120 + n, 0.4f, 1.5f, 0.12f, 0.35f, LS);
        w.scatterStalagmites(Aabb(c + V(-6, -1.f, -6), c + V(6, 1.2f, 6)), 2, 140 + n, 0.3f, 0.9f, 0.12f, 0.3f, LS);
    }
    w.scatterStalactites(Aabb(V(18, -12, -170), V(30, -6, -158)), 5, 115, 0.5f, 1.4f, 0.12f, 0.3f, LS);
}

} // namespace L1
