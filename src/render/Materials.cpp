#include "Materials.h"

static MaterialInfo g_mats[MAT_COUNT];
static bool g_init = false;

static void set(int id, const char* name, float rough, float metal, float emi, float wet, bool tr, float r, float g, float b)
{
    g_mats[id] = MaterialInfo{name, rough, metal, emi, wet, tr, QVector3D(r, g, b)};
}

static void init()
{
    for (int i = 0; i < MAT_COUNT; ++i) set(i, "unused", 0.8f, 0, 0, 0, false, 0.5f, 0.5f, 0.5f);
    set(MAT_LIMESTONE,     "limestone",     0.85f, 0, 0, 0.25f, false, 0.36f, 0.32f, 0.27f);
    set(MAT_WET_LIMESTONE, "wet_limestone", 0.35f, 0, 0, 0.8f,  false, 0.30f, 0.32f, 0.33f);
    set(MAT_OBSIDIAN,      "obsidian",      0.12f, 0.1f, 0, 0.5f, false, 0.035f, 0.035f, 0.045f);
    set(MAT_GEODE,         "geode",         0.30f, 0, 0.10f, 0.3f, false, 0.34f, 0.14f, 0.52f);
    set(MAT_HOT_BASALT,    "hot_basalt",    0.7f,  0, 0.6f, 0,   false, 0.10f, 0.045f, 0.04f);
    set(MAT_ALTERED,       "altered_rock",  0.18f, 0, 0.15f, 0.3f, false, 0.20f, 0.17f, 0.24f);
    set(MAT_ICE,           "ice",           0.15f, 0, 0, 0.9f,  false, 0.55f, 0.72f, 0.82f);
    set(MAT_SNOW,          "snow",          0.9f,  0, 0, 0,     false, 0.85f, 0.88f, 0.93f);
    set(MAT_ASH_FLOOR,     "ash_floor",     0.95f, 0, 0, 0,     false, 0.16f, 0.15f, 0.15f);

    set(MAT_WOOD,        "wood",         0.8f, 0, 0, 0, false, 0.45f, 0.30f, 0.17f);
    set(MAT_ROTTEN_WOOD, "rotten_wood",  0.9f, 0, 0, 0.1f, false, 0.26f, 0.22f, 0.16f);
    set(MAT_ROPE,        "rope",         0.95f, 0, 0, 0, false, 0.55f, 0.46f, 0.30f);
    set(MAT_CLOTH,       "cloth",        0.95f, 0, 0, 0, false, 0.5f, 0.3f, 0.25f);
    set(MAT_HIDE,        "hide",         0.85f, 0, 0, 0, false, 0.40f, 0.28f, 0.20f);
    set(MAT_BONE,        "bone",         0.7f, 0, 0, 0, false, 0.80f, 0.76f, 0.66f);
    set(MAT_RUSTY_IRON,  "rusty_iron",   0.8f, 0.5f, 0, 0, false, 0.36f, 0.20f, 0.12f);
    set(MAT_IRON,        "iron",         0.5f, 0.8f, 0, 0, false, 0.30f, 0.30f, 0.33f);
    set(MAT_BRONZE,      "bronze",       0.4f, 0.9f, 0, 0, false, 0.72f, 0.45f, 0.20f);
    set(MAT_GOLD,        "gold",         0.25f, 1.0f, 0, 0, false, 0.95f, 0.75f, 0.25f);
    set(MAT_GLASS,       "glass",        0.05f, 0, 0, 0, true,  0.7f, 0.85f, 0.9f);
    set(MAT_CLEAR_ICE,   "clear_ice",    0.08f, 0, 0, 0.5f, true, 0.7f, 0.88f, 0.95f);
    set(MAT_CRYSTAL,     "crystal",      0.1f, 0, 0.5f, 0, true,  0.6f, 0.4f, 0.9f);
    set(MAT_FLESH,       "flesh",        0.5f, 0, 0.08f, 0.5f, false, 0.55f, 0.15f, 0.15f);
    set(MAT_SKIN,        "skin",         0.7f, 0, 0, 0, false, 0.75f, 0.55f, 0.45f);
    set(MAT_FUNGUS,      "fungus",       0.6f, 0, 1.2f, 0.3f, false, 0.25f, 0.9f, 0.7f);
    set(MAT_EMISSIVE,    "emissive",     1.0f, 0, 2.0f, 0, false, 1.0f, 1.0f, 1.0f);
    set(MAT_COAL,        "coal",         0.9f, 0, 0, 0, false, 0.05f, 0.05f, 0.05f);
    set(MAT_PAPER,       "paper",        0.9f, 0, 0, 0, false, 0.85f, 0.80f, 0.65f);
    set(MAT_ADAMANTINE,  "adamantine",   0.3f, 0.9f, 0.05f, 0, false, 0.12f, 0.25f, 0.18f);

    set(MAT_WATER,         "water",         0.05f, 0, 0, 1, true, 0.10f, 0.30f, 0.40f);
    set(MAT_BOILING_WATER, "boiling_water", 0.1f, 0, 0.35f, 1, true, 0.55f, 0.45f, 0.35f);
    set(MAT_LAVA,          "lava",          0.6f, 0, 2.2f, 0, false, 1.0f, 0.35f, 0.05f);
    set(MAT_NECRO_LIQUID,  "necro_liquid",  0.1f, 0, 0.8f, 1, true, 0.25f, 0.9f, 0.4f);
    g_init = true;
}

const MaterialInfo& materialInfo(int id)
{
    if (!g_init) init();
    if (id < 0 || id >= MAT_COUNT) id = 0;
    return g_mats[id];
}
