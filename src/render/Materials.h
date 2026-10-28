// Materiais compartilhados: ids de rocha (SDF), props e liquidos.
// A tabela em Materials.cpp e a unica fonte da verdade (C++ e GLSL usam os mesmos dados).
#pragma once
#include <QVector3D>
#include <QString>

enum MatId : int {
    // ---- rocha (SDF) ----
    MAT_LIMESTONE = 0,     // calcario cinza-marrom umido
    MAT_WET_LIMESTONE = 1, // calcario molhado / coberto de gelo
    MAT_OBSIDIAN = 2,      // obsidiana negra vitrea
    MAT_GEODE = 3,         // ametista roxa com quartzo
    MAT_HOT_BASALT = 4,    // basalto quente com rachaduras brilhantes
    MAT_ALTERED = 5,       // rocha alterada (labirinto psiquico)
    MAT_ICE = 6,           // gelo (rocha de gelo, opaco)
    MAT_SNOW = 7,          // neve (exterior)
    MAT_ASH_FLOOR = 8,     // cinza petrificada
    MAT_ROCK_COUNT = 9,

    // ---- props ----
    MAT_WOOD = 16, MAT_ROTTEN_WOOD, MAT_ROPE, MAT_CLOTH, MAT_HIDE, MAT_BONE,
    MAT_RUSTY_IRON, MAT_IRON, MAT_BRONZE, MAT_GOLD,
    MAT_GLASS,        // transparente
    MAT_CLEAR_ICE,    // transparente
    MAT_CRYSTAL,      // translucido/emissivo
    MAT_FLESH, MAT_SKIN,
    MAT_FUNGUS,       // cogumelo brilhante
    MAT_EMISSIVE,     // cor pura emissiva
    MAT_COAL, MAT_PAPER, MAT_ADAMANTINE,

    // ---- liquidos (animados) ----
    MAT_WATER = 40,   // transparente
    MAT_BOILING_WATER,// transparente + emissivo
    MAT_LAVA,         // opaco, emissivo
    MAT_NECRO_LIQUID, // transparente, esverdeado, emissivo

    MAT_COUNT = 48
};

struct MaterialInfo {
    const char* name;      // nome (docs)
    float roughness;
    float metal;
    float emissive;        // 0..1+ (multiplica a cor)
    float wet;             // 0..1 (brilho de umidade)
    bool transparent;      // vai para o passe transparente
    QVector3D base;        // cor base padrao (rocha usa no shader)
};

const MaterialInfo& materialInfo(int id);
inline bool materialIsTransparent(int id) { return materialInfo(id).transparent; }
inline bool materialIsRock(int id) { return id >= 0 && id < MAT_ROCK_COUNT; }
