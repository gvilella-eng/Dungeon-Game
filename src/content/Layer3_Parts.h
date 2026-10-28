// Partes da Camada 3 (As Veias Vitreas), divididas em arquivos para manter cada um legivel.
#pragma once
#include "world/World.h"
#include "Layer2_Util.h"

namespace L3 {
using L23::V3;
// Centro da cupula de Kaelen e constantes compartilhadas
const Vec3 DC(-75.f, -186.f, -255.f);
const float kDomeR = 22.f, kDomeFloor = -186.f, kDomeH = 20.f;

// Fissura: eixo (ida) do inicio ao fim
const Vec3 kFissS(-85.f, -189.f, -299.f);
const Vec3 kFissE(-90.f, -194.f, -336.f);

void buildKaelenCamp(World& w);      // 3.2 O Entroncamento de Kaelen + Tubo A (props)
void buildVents(World& w);           // 3.3 Dutos de Respiro (props, POIs, zonas)
void buildFungus(World& w);          // 3.4 Fungos da Memoria
void buildFissure(World& w);         // 3.5 Fenda da Guerra Elemental
float domeCeilingY(float x, float z);
} // namespace L3
