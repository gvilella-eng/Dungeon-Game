// Ponto de entrada do conteudo: monta todas as camadas e a Sala de Testes.
#pragma once
class World;

// Constroi o mundo. Se testLevelOnly, so a Sala de Testes (rapido).
void buildWorld(World& world, bool testLevelOnly);

// Uma funcao por camada (cada uma em seu proprio arquivo; veja docs/MASMORRA.md).
void buildLayer0Exterior(World& w);
void buildLayer1Goblins(World& w);
void buildLayer2Pit(World& w);
void buildLayer3Veins(World& w);
void buildLayer3bLabyrinth(World& w);
void buildLayer4Geode(World& w);
void buildLayer5Parasite(World& w);
void buildTestLevel(World& w);
void buildStressLevel(World& w);   // so com --stress (medicao de desempenho)
