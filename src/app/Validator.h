// --validate: conectividade da rota (grade de ar ~0.5 m) e pontos de vista/POIs dentro da rocha.
#pragma once
#include "world/World.h"
#include <QString>

// Retorna 0 se tudo OK, 1 se houver falhas. Escreve o relatorio em reportPath e no stdout.
int runValidation(World& world, const QString& reportPath);
