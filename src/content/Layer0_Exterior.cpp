// CAMADA 0 — A Encosta Gelada (exterior)
// Encosta nevada, penhasco de ~60 m, boca da caverna em z ~ 0, pistas na neve (pegadas, cobertor, carvao),
// nevasca e neblina azulada. Este arquivo tambem define a zona padrao "caverna escura" do jogo inteiro.
#include "Dungeon.h"
#include "world/World.h"
#include "Prefabs.h"
#include "Layer1_Common.h"

namespace {

// ---------------------------------------------------------------- terreno
float terrainH(float x, float z)
{
    const float ax = std::fabs(x);
    // inclinacao suave para o sul: 0 ate z=35, desce a -4 em z=100
    float base = -4.f * smoothstepf(35.f, 100.f, z);
    // dunas de neve (+-0.8 m), apagadas perto da boca e do spawn
    float dunes = 0.45f * std::sin(x * 0.19f + z * 0.07f) * std::cos(z * 0.15f)
                + 0.30f * std::sin(z * 0.31f + x * 0.13f)
                + 0.18f * std::sin(x * 0.52f + z * 0.43f);
    float dx = x, dz = z - 40.f;
    float nearSpawn = smoothstepf(3.f, 14.f, std::sqrt(dx * dx + dz * dz));
    float mask = nearSpawn * smoothstepf(8.f, 24.f, z);
    // paredoes do vale (sobem a partir de |x| > 33) com rugosidade
    float side = 70.f * smoothstepf(33.f, 58.f, ax)
               + smoothstepf(33.f, 50.f, ax) * (3.2f * std::sin(z * 0.27f + x * 0.05f) + 2.f * std::sin(z * 0.61f + x * 0.21f));
    // penhasco da montanha: base em z ~ +4,5, borda (a vila) a 60 m em z ~ -6, depois sobe fechando a cena
    float t = smoothstepf(4.5f, -6.f, z);
    float cliff = 60.f * t + t * (1.f - t) * 4.f * (2.5f * std::sin(x * 0.31f) + 1.5f * std::sin(x * 0.83f + z * 0.2f))
                + 14.f * smoothstepf(-9.5f, -12.f, z);
    // fim sul do vale
    float farw = 70.f * smoothstepf(100.f, 112.f, z);
    float wall = std::max(std::max(side, cliff), farw);
    return base + dunes * mask + wall;
}

} // namespace

void buildLayer0Exterior(World& w)
{
    L1::resetState();

    // ---------------------------------------------------------------- zona padrao: caverna escura
    {
        ZoneEffects fx;
        fx.temperature = 8.f;
        fx.fogColor = Vec3(0.006f, 0.006f, 0.009f); fx.fogDensity = 0.030f;
        fx.ambientColor = Vec3(0.016f, 0.017f, 0.024f); fx.ambientGround = Vec3(0.006f, 0.006f, 0.008f); fx.ambientStrength = 1.f;
        w.setDefaultZone(fx);
    }

    // ---------------------------------------------------------------- terreno
    w.setTerrain(terrainH, Aabb(Vec3(-60.f, -8.f, -12.f), Vec3(60.f, 62.f, 112.f)));

    // boca da caverna: arco de 12 m de largura por 9 m de altura, chao em y = 0
    w.carve(Prim::ellipsoid(Vec3(0, 0, -1.f), Vec3(6.3f, 9.f, 8.f)).floorAt(-0.12f).smooth(2.f).noise(0.55f).mat(MAT_WET_LIMESTONE).named(Q("boca")));
    // ombreiras irregulares da boca (reentrancias na rocha)
    w.carve(Prim::ellipsoid(Vec3(-5.6f, 1.f, 2.f), Vec3(1.8f, 4.f, 2.5f)).floorAt(-0.12f).smooth(1.5f).noise(0.5f).mat(MAT_WET_LIMESTONE));
    w.carve(Prim::ellipsoid(Vec3(5.4f, 1.f, 1.5f), Vec3(1.6f, 3.4f, 2.f)).floorAt(-0.12f).smooth(1.5f).noise(0.5f).mat(MAT_WET_LIMESTONE));

    // rochas e blocos caidos do penhasco ao redor da boca (preenchimentos)
    w.fill(Prim::ellipsoid(Vec3(-9.f, 0.2f, 5.5f), Vec3(2.6f, 1.6f, 2.2f)).noise(0.1f).smooth(0.3f).mat(MAT_LIMESTONE));
    w.fill(Prim::ellipsoid(Vec3(8.5f, 0.1f, 6.f), Vec3(2.2f, 1.3f, 2.f)).noise(0.1f).smooth(0.3f).mat(MAT_LIMESTONE));
    w.fill(Prim::ellipsoid(Vec3(-14.f, 0.f, 12.f), Vec3(3.f, 1.8f, 2.4f)).noise(0.1f).smooth(0.3f).mat(MAT_LIMESTONE));
    w.fill(Prim::ellipsoid(Vec3(13.f, 0.f, 15.f), Vec3(2.4f, 1.4f, 2.6f)).noise(0.1f).smooth(0.3f).mat(MAT_LIMESTONE));
    w.fill(Prim::ellipsoid(Vec3(-24.f, 0.f, 30.f), Vec3(3.4f, 2.2f, 2.8f)).noise(0.12f).smooth(0.3f).mat(MAT_LIMESTONE));
    w.fill(Prim::ellipsoid(Vec3(22.f, -0.2f, 52.f), Vec3(3.f, 2.0f, 3.f)).noise(0.12f).smooth(0.3f).mat(MAT_LIMESTONE));
    // montes de neve acumulada junto a boca
    w.fill(Prim::ellipsoid(Vec3(-4.2f, -0.2f, 7.5f), Vec3(2.8f, 0.9f, 2.2f)).noise(0.12f).smooth(0.6f).mat(MAT_SNOW));
    w.fill(Prim::ellipsoid(Vec3(4.8f, -0.2f, 9.f), Vec3(3.2f, 0.8f, 2.4f)).noise(0.12f).smooth(0.6f).mat(MAT_SNOW));

    // ---------------------------------------------------------------- props: pegadas, cobertor, carvao, arvores, vila
    {
        MeshBuilder mb;
        auto trailX = [](float z) { return 3.f + 2.2f * std::sin(0.12f * (z - 18.f)); };
        // pegadas pequenas (goblins) da boca para o sul
        float z = 0.5f;
        int k = 0;
        while (z < 96.f) {
            float x0 = trailX(z), x1 = trailX(z + 1.f);
            float dirYaw = std::atan2(-(x1 - x0), -1.f) * 57.29578f;
            float side = (k % 2 == 0) ? -1.f : 1.f;
            float px = x0 + side * 0.11f + (L1::hrand(k, 1) - 0.5f) * 0.08f;
            float pz = z + (L1::hrand(k, 2) - 0.5f) * 0.1f;
            float gy = terrainH(px, pz);
            mb.push(); mb.translate(Vec3(px, gy + 0.012f, pz)); mb.rotateY(dirYaw + 180.f + side * 8.f);
            mb.box(Vec3(0, 0, 0), Vec3(0.055f, 0.012f, 0.1f), QColor(118, 132, 158), MAT_CLOTH);
            mb.box(Vec3(0, 0, -0.12f), Vec3(0.04f, 0.012f, 0.03f), QColor(118, 132, 158), MAT_CLOTH);   // dedos
            mb.pop();
            z += 0.5f + (L1::hrand(k, 3) - 0.5f) * 0.12f; ++k;
        }
        // cobertor da vila caido em (3,0,18): manta xadrez meio coberta de neve
        {
            const Vec3 c(3.f, terrainH(3.f, 18.f), 18.f);
            mb.push(); mb.translate(c); mb.rotateY(24.f);
            const QColor red(142, 40, 36), cream(196, 176, 140);
            mb.box(Vec3(0, 0.05f, 0), Vec3(0.78f, 0.045f, 0.55f), red, MAT_CLOTH);
            for (int i = 0; i < 4; ++i) mb.box(Vec3(-0.55f + i * 0.37f, 0.098f, 0), Vec3(0.06f, 0.006f, 0.55f), cream, MAT_CLOTH);
            for (int i = 0; i < 3; ++i) mb.box(Vec3(0, 0.099f, -0.35f + i * 0.35f), Vec3(0.78f, 0.006f, 0.05f), cream, MAT_CLOTH);
            mb.box(Vec3(0.5f, 0.17f, 0.3f), Vec3(0.3f, 0.06f, 0.22f), red, MAT_CLOTH);   // dobra levantada
            mb.ellipsoid(Vec3(-0.25f, 0.1f, 0.1f), Vec3(0.5f, 0.06f, 0.32f), QColor(232, 238, 248), MAT_SNOW, 8);   // neve por cima
            mb.pop();
        }
        // pedacos de carvao espalhados entre z 5 e 15
        for (int i = 0; i < 38; ++i) {
            float x = (L1::hrand(i, 11) - 0.5f) * 11.f + (i < 20 ? trailX(8.f + i * 0.35f) * 0.4f : 0.f);
            float zz = 5.f + L1::hrand(i, 12) * 10.5f;
            float s = 0.04f + L1::hrand(i, 13) * 0.07f;
            mb.ellipsoid(Vec3(x, terrainH(x, zz) + s * 0.4f, zz), Vec3(s, s * 0.7f, s * 0.9f), QColor(26, 24, 26), MAT_COAL, 6);
        }
        // saco furado com carvao derramado
        Prefabs::sack(mb, Vec3(5.3f, terrainH(5.3f, 12.4f), 12.4f), 30.f, 0.9f);
        for (int i = 0; i < 9; ++i) mb.ellipsoid(Vec3(5.5f + L1::hrand(i, 21) * 1.2f, terrainH(5.5f, 12.4f) + 0.04f, 12.7f + L1::hrand(i, 22) * 0.9f), Vec3(0.07f, 0.05f, 0.06f), QColor(24, 22, 24), MAT_COAL, 6);
        // abetos nevados e troncos mortos nas margens do vale
        for (int i = 0; i < 34; ++i) {
            float side = (i % 2 == 0) ? -1.f : 1.f;
            float x = side * (14.f + L1::hrand(i, 31) * 26.f);
            float zz = 14.f + L1::hrand(i, 32) * 84.f;
            if (std::fabs(x - trailX(zz)) < 9.f) continue;
            float gy = terrainH(x, zz);
            if (gy > 6.f) continue;
            float h = 3.f + L1::hrand(i, 33) * 4.2f;
            mb.cylinder(Vec3(x, gy - 0.1f, zz), Vec3(x, gy + h * 0.45f, zz), 0.14f, 0.09f, 6, QColor(56, 40, 30), MAT_WOOD);
            if (i % 5 == 0) {   // tronco morto
                mb.cylinder(Vec3(x, gy + h * 0.4f, zz), Vec3(x + 0.8f, gy + h * 0.75f, zz + 0.3f), 0.08f, 0.04f, 5, QColor(60, 46, 36), MAT_ROTTEN_WOOD);
                continue;
            }
            for (int c = 0; c < 4; ++c) {
                float cy = gy + h * (0.2f + 0.2f * c), cr = (1.5f - 0.3f * c) * (0.8f + h * 0.07f);
                mb.cone(Vec3(x, cy, zz), Vec3(x, cy + h * 0.38f, zz), cr, 8, QColor(34, 52, 50), MAT_CLOTH);
                mb.cone(Vec3(x, cy + h * 0.15f, zz), Vec3(x, cy + h * 0.4f, zz), cr * 0.62f, 8, QColor(226, 234, 246), MAT_SNOW);
            }
        }
        // a vila la no alto do penhasco (borda a 60 m): telhados nevados e janelas quentes
        for (int i = 0; i < 9; ++i) {
            float hx = -17.f + i * 4.3f + (L1::hrand(i, 41) - 0.5f) * 2.f;
            float hz = -7.f - L1::hrand(i, 42) * 1.5f;
            float gy = terrainH(hx, hz);
            mb.box(Vec3(hx, gy + 0.8f, hz), Vec3(1.1f, 0.8f, 0.9f), QColor(52, 46, 44), MAT_LIMESTONE);
            mb.box(Vec3(hx, gy + 1.7f, hz), Vec3(1.3f, 0.22f, 1.1f), QColor(220, 228, 240), MAT_SNOW);
            mb.box(Vec3(hx, gy + 0.9f, hz + 0.91f), Vec3(0.55f, 0.45f, 0.02f), QColor(255, 190, 100), MAT_EMISSIVE);
            if (i % 2 == 0) mb.box(Vec3(hx + 0.55f, gy + 0.7f, hz + 0.91f), Vec3(0.35f, 0.32f, 0.02f), QColor(255, 160, 70), MAT_EMISSIVE);
        }
        w.addStatic(std::move(mb));
        w.addLight(Vec3(0.f, 62.f, -3.f), Vec3(1.0f, 0.6f, 0.28f), 1.6f, 30.f, LightAnim::Pulse, Q("brilho da vila"))->animFreq = 0.13f;
    }

    // ---------------------------------------------------------------- luzes e particulas
    w.addLight(Vec3(0.f, 4.f, 5.5f), Vec3(0.40f, 0.55f, 1.0f), 6.0f, 20.f, LightAnim::Pulse, Q("luz do dia na boca"))->animFreq = 0.11f;
    w.addLight(Vec3(0.f, 2.5f, -3.f), Vec3(0.35f, 0.48f, 0.9f), 3.0f, 12.f, LightAnim::None, Q("luz azul entrando"));
    {   // nevasca em volta do jogador (so no exterior)
        EmitterDesc d; d.preset = ParticlePreset::Snow; ParticleEmitter::applyPreset(d);
        d.name = Q("nevasca"); d.followPlayer = true; d.followOffset = Vec3(0, 7.5f, 0);
        d.halfExtent = Vec3(22.f, 0.5f, 22.f); d.rate = 520.f; d.velocity = Vec3(1.9f, -1.7f, 0.5f); d.velJitter = 0.5f; d.turbulence = 0.9f;
        EmitterHandle e = w.addEmitter(d);
        w.addUpdater([e, &w](float, float) {
            Vec3 p = w.playerPos();
            e->setEnabled(p.z() > 1.f && std::fabs(p.x()) < 62.f && p.y() < 64.f);
        });
        EmitterDesc m; m.preset = ParticlePreset::Snow; ParticleEmitter::applyPreset(m);
        m.name = Q("neve na boca"); m.pos = Vec3(0.f, 7.f, 5.f); m.halfExtent = Vec3(6.f, 0.5f, 3.f); m.rate = 70.f;
        m.velocity = Vec3(0.4f, -1.2f, -1.4f); m.lifeMin = 3.f; m.lifeMax = 5.f; m.activeRange = 70.f;
        w.addEmitter(m);
        EmitterDesc g; g.preset = ParticlePreset::Snow; ParticleEmitter::applyPreset(g);
        g.name = Q("ventania no penhasco"); g.pos = Vec3(0.f, 28.f, 14.f); g.halfExtent = Vec3(26.f, 20.f, 10.f); g.rate = 160.f;
        g.velocity = Vec3(3.6f, -0.9f, -0.3f); g.sizeMin = 0.05f; g.sizeMax = 0.1f; g.lifeMin = 5.f; g.lifeMax = 8.f; g.activeRange = 110.f;
        w.addEmitter(g);
    }

    // ---------------------------------------------------------------- zonas
    {
        ZoneEffects fx;
        fx.areaName = Q("Camada 0 · A Encosta Gelada"); fx.subtitle = Q("Crepúsculo de nevasca · −15 °C");
        fx.temperature = -15.f;
        fx.fogColor = Vec3(0.105f, 0.130f, 0.180f); fx.fogDensity = 0.025f;
        fx.ambientColor = Vec3(0.30f, 0.37f, 0.52f); fx.ambientGround = Vec3(0.17f, 0.20f, 0.27f); fx.ambientStrength = 1.f;
        w.addZone(Q("encosta"), ZoneShape::makeBox(Vec3(-61.f, -9.f, -2.f), Vec3(61.f, 63.f, 113.f)), fx, 1);
        ZoneEffects bf = fx;
        bf.areaName = Q("0.2 A Boca Congelada"); bf.subtitle = Q("Franja de pingentes · −15 °C");
        bf.fogColor = Vec3(0.05f, 0.065f, 0.10f); bf.fogDensity = 0.028f;
        bf.ambientColor = Vec3(0.12f, 0.15f, 0.24f); bf.ambientGround = Vec3(0.06f, 0.07f, 0.10f);
        w.addZone(Q("boca_congelada"), ZoneShape::makeBox(Vec3(-9.f, -3.f, -4.f), Vec3(9.f, 10.f, 8.f)), bf, 2);
    }

    // ---------------------------------------------------------------- pingentes de gelo na franja da boca
    {
        MeshBuilder mb;
        Prefabs::iceFringe(mb, Vec3(-5.4f, 8.2f, 2.8f), Vec3(5.4f, 8.2f, 2.8f), 34, 2.2f, 7);
        Prefabs::iceFringe(mb, Vec3(-6.0f, 6.2f, 0.2f), Vec3(-3.0f, 8.6f, 1.4f), 9, 1.5f, 8);
        Prefabs::iceFringe(mb, Vec3(3.0f, 8.6f, 1.4f), Vec3(6.0f, 6.0f, 0.2f), 9, 1.5f, 9);
        w.addStatic(std::move(mb));
    }

    // ---------------------------------------------------------------- spawn
    w.setSpawn(Vec3(0.f, 1.65f, 40.f), 0.f, 4.f);

    // ---------------------------------------------------------------- POIs
    const QString S = Q("Sessão 1 · Nível 6");
    const QString A0 = Q("Camada 0 · A Encosta Gelada");
    w.addPoi(Vec3(1.5f, 0.7f, 14.f), PoiCategory::Lore, Q("Trilha dos Ladrões"),
             Q("O QUE É: uma trilha de pegadas pequenas (descalças e com trapos) que sai da boca da caverna e segue para o sul, para o vale, e some sob a neve nova. Entre z 5 e 15 há pedaços de carvão perdidos, um saco furado com carvão derramado e, em (3, 0, 18), um COBERTOR DA VILA (xadrez vermelho e creme, com a marca de tecelagem do mercado) caído e já meio coberto de neve.\n\n"
               "O QUE OS JOGADORES PERCEBEM: Sobrevivência/Rastreamento CD 10: as pegadas são de criaturas Pequenas ou Médias muito leves (goblinoides), umas 10 a 15 indo e voltando, algumas arrastando peso. Investigação CD 12: o carvão é de lareira doméstica, não de mineração: veio da vila. O cobertor tem o selo do mercado; dá para saber que foi roubado nesta estação.\n\n"
               "A HISTÓRIA POR TRÁS: é a PISTA de que os goblins roubam da vila. Mas as pegadas são de quem desce à noite para furtar e VOLTA correndo; nenhuma pegada foi até a vila com intenção de ataque, nem há sangue. Planta no jogador a dúvida: ladrões famintos ou invasores? (O padre da vila vai ler isto como 'ritual' de roubo de calor.)\n\n"
               "DICA DE MESA: deixe o grupo achar sozinho que o rastro é pequeno e hesitante. Quem tem Furtividade ou Natureza percebe que as pegadas pararam várias vezes olhando para trás: eles têm medo de algo atrás deles, não à frente."),
             S, A0);
    w.addPoi(Vec3(0.f, 3.2f, 3.f), PoiCategory::Area, Q("A Boca Congelada"),
             Q("O QUE É: a boca da caverna, em z ≈ 0, um arco de 12 m de largura (x −6..6) por 9 m de altura aberto na base de um penhasco de ~60 m. Uma franja de pingentes de gelo, alguns do tamanho de um braço, pende do lintel. Temperatura: −15 °C (Constituição CD 10 a cada hora ao ar livre sem roupas de frio, a critério do Mestre).\n\n"
               "O QUE OS JOGADORES PERCEBEM: um vento cortante que entra e sai da gruta como respiração; neve entrando em espirais; lá dentro, escuridão total. A luz azulada do crepúsculo morre uns 10 m depois da boca. Bem no alto do penhasco, entre a neve, um brilho alaranjado e muito fraco: a vila.\n\n"
               "MECÂNICA: a tocha começa APAGADA (use [R]). Dentro da boca, abrigada, a chama aguenta. Visão no escuro [V] ajuda pouco: só tons de cinza até 18 m.\n\n"
               "AVISO AO MESTRE: o interior imediato (1.1) parece uma fogueira de ritual: veja a nota sobre o Falso Ritual antes de deixar os jogadores entrarem. A entrada é a única saída para o exterior; um grupo em fuga sempre pode voltar por aqui."),
             S, A0);
    w.addPoi(Vec3(0.f, 56.f, 4.f), PoiCategory::Lore, Q("O brilho da vila"),
             Q("Muito acima, na borda do penhasco, há um brilho quente e muito fraco: as janelas da vila, que antes era a cidade mais quente da região e hoje vive de lareiras racionadas. De baixo, a vila parece segura e acolhedora; é uma imagem para o contraste que virá: a vila é o motivo de tudo, e os jogadores a veem pelas costas, a partir do fundo do inferno que a sustenta.\n\n"
               "Use para lembrar à mesa: a montanha é MUITO alta e a vila está literalmente sobre as suas cabeças durante toda a masmorra."),
             S, A0);

    // ---------------------------------------------------------------- pontos de vista
    const QString G0 = Q("Camada 0 · A Encosta Gelada");
    w.addViewpoint(Q("0.1 Encosta gelada (spawn)"), Vec3(0.f, 1.65f, 40.f), 0.f, 4.f, false, G0);
    w.addViewpoint(Q("0.2 Trilha dos Ladrões e cobertor"), Vec3(1.2f, 1.65f, 25.f), -12.f, -16.f, false, G0);
    w.addViewpoint(Q("0.3 A Boca Congelada"), Vec3(0.f, 1.65f, 12.f), 0.f, 14.f, false, G0);
    w.addViewpoint(Q("0.4 O penhasco e a vila"), Vec3(0.f, 1.65f, 26.f), 0.f, 61.f, false, G0);
    w.addViewpoint(Q("0.5 Vista aérea da encosta (voo livre)"), Vec3(0.f, 34.f, 104.f), 0.f, -24.f, true, G0);

    // ---------------------------------------------------------------- rota (inicio)
    w.addRoutePoint(Q("Spawn · Encosta"), Vec3(0.f, 0.5f, 40.f));
    w.addRoutePoint(Q("Trilha dos Ladrões"), Vec3(2.5f, 0.5f, 20.f));
    w.addRoutePoint(Q("Boca da Caverna"), Vec3(0.f, 0.5f, 4.f));
}
