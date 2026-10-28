#include "Dungeon.h"
#include "world/World.h"

void buildWorld(World& w, bool testLevelOnly)
{
    if (!testLevelOnly) {
        // ordem importa: camadas posteriores podem consultar o SDF das anteriores
        buildLayer0Exterior(w);
        buildLayer1Goblins(w);
        buildLayer2Pit(w);
        buildLayer3Veins(w);
        buildLayer3bLabyrinth(w);
        buildLayer4Geode(w);
        buildLayer5Parasite(w);
    }
    if (!testLevelOnly) w.setOverview(Vec3(20, -150, -250), 340.f);   // vista geral da masmorra (exclui a Sala de Testes)
    w.addRouteBreak();   // a rota da Sala de Testes e um trecho separado
    buildTestLevel(w);
}
