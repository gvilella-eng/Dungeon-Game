// CAMADA 1 — A Garganta Erodida (goblins): 1.1 a 1.8. Orquestra as fases em Layer1_*.cpp.
#include "Dungeon.h"
#include "world/World.h"
#include "Prefabs.h"
#include "Layer1_Common.h"

namespace L1 {
static std::unique_ptr<State> g_state;
State& st() { if (!g_state) g_state.reset(new State()); return *g_state; }
void resetState() { g_state.reset(new State()); }
}

void buildLayer1Goblins(World& w)
{
    L1::buildGeometry(w);
    L1::buildProps(w);
    L1::buildGameplay(w);
    L1::buildPois(w);
}
