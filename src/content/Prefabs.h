// Biblioteca compartilhada de prefabs (props e criaturas low-poly).
// Todas as funcoes desenham em um MeshBuilder, respeitando a transformacao atual da pilha;
// 'pos' e o ponto no CHAO (base do objeto), 'yaw' em graus em torno de Y (0 = olhando para -Z).
// Olhos brilhantes usam MAT_EMISSIVE (leem no escuro).
#pragma once
#include "render/MeshBuilder.h"
#include "render/Materials.h"

namespace Prefabs {

enum class Pose { Stand, Crouch, Huddle, Lie, Hang, Sit };

struct HumanoidParams {
    float height = 1.75f;                 // altura em pe (m)
    float build = 1.0f;                   // 0.8 magro .. 1.3 robusto
    QColor skin = QColor(200, 150, 120);
    QColor cloth = QColor(90, 70, 60);    // tronco/casaco
    QColor cloth2 = QColor(55, 50, 50);   // calcas/botas
    QColor hair = QColor(50, 35, 25);
    Pose pose = Pose::Stand;
    bool cape = false;                    // capa/manto nas costas
    bool hood = false;                    // capuz
    bool glowingEyes = false;
    QColor eyeColor = QColor(255, 255, 255);
    bool axe = false;                     // machado na mao (Kaelen)
};

void humanoid(MeshBuilder& mb, const Vec3& pos, float yawDeg, const HumanoidParams& p = HumanoidParams());
void goblin(MeshBuilder& mb, const Vec3& pos, float yawDeg, Pose pose = Pose::Stand, float scale = 1.f, bool withBow = false);
void goblinChild(MeshBuilder& mb, const Vec3& pos, float yawDeg, Pose pose = Pose::Huddle);
// lobo da montanha; hellHound=true: pelo escuro, olhos de brasa azul, rachaduras brilhantes.
void wolf(MeshBuilder& mb, const Vec3& pos, float yawDeg, bool hellHound = false, Pose pose = Pose::Stand, float scale = 1.f);

void brazier(MeshBuilder& mb, const Vec3& pos, float scale = 1.f, bool lit = true);      // bronze + bacia de brasas
void wallTorch(MeshBuilder& mb, const Vec3& pos, const Vec3& awayFromWall);             // tocha presa na parede
void campfire(MeshBuilder& mb, const Vec3& pos, float scale = 1.f);
void crate(MeshBuilder& mb, const Vec3& pos, float yawDeg, float size = 0.8f);
void barrel(MeshBuilder& mb, const Vec3& pos, float yawDeg = 0.f, float scale = 1.f);
void sack(MeshBuilder& mb, const Vec3& pos, float yawDeg = 0.f, float scale = 1.f);
void hideTent(MeshBuilder& mb, const Vec3& pos, float yawDeg, float radius, float height, const QColor& cloth = QColor(120, 70, 60));
void bonesPile(MeshBuilder& mb, const Vec3& pos, float radius, int seed = 1);
void skull(MeshBuilder& mb, const Vec3& pos, float yawDeg = 0.f, float scale = 1.f);
void ribcage(MeshBuilder& mb, const Vec3& pos, float yawDeg, float scale = 1.f);        // scale >> 1: costelas gigantes
void giantRib(MeshBuilder& mb, const Vec3& base, const Vec3& tip, float radius, float archHeight, int seed = 1); // costela arqueada
void chain(MeshBuilder& mb, const Vec3& a, const Vec3& b, float sag = 0.3f, float linkSize = 0.09f, int material = MAT_RUSTY_IRON);
void hangingCage(MeshBuilder& mb, const Vec3& attachTop, float size = 1.6f, float chainLength = 3.f, bool withPrisoner = false);
// ponte de corda: tabuas + corrimaos em catenaria; 'rotten' deixa tabuas tortas/faltando.
void ropeBridge(MeshBuilder& mb, const Vec3& a, const Vec3& b, float width = 1.5f, float sag = 0.6f, int planks = 20, bool rotten = true, int seed = 3);
void table(MeshBuilder& mb, const Vec3& pos, float yawDeg, const Vec3& size = Vec3(1.6f, 0.8f, 0.8f), bool withBooksAndPapers = true);
void chest(MeshBuilder& mb, const Vec3& pos, float yawDeg, bool open = false, float scale = 1.f);
// capsula de vidro e ferro com liquido necromantico; 'figure' poe um humano com tubos no peito; 'depleted': vidro rachado, escuro.
void glassCapsule(MeshBuilder& mb, const Vec3& pos, float height = 2.4f, float radius = 0.7f, bool figure = true, bool depleted = false, bool open = false);
void pipe(MeshBuilder& mb, const std::vector<Vec3>& pts, float radius, int material = MAT_BRONZE, bool clamps = true);
// segmento de andaime: postes + travessas + diagonais entre a e b (a/b = extremos da base; height = altura).
void scaffoldSegment(MeshBuilder& mb, const Vec3& a, const Vec3& b, float height = 3.f, float width = 3.f);
void gratedWalkway(MeshBuilder& mb, const Vec3& a, const Vec3& b, float width = 3.f, bool rails = true);
void ironStairs(MeshBuilder& mb, const Vec3& bottom, const Vec3& top, float width = 1.4f);
void ladder(MeshBuilder& mb, const Vec3& bottom, const Vec3& top, float yawDeg = 0.f, float width = 0.5f);   // yaw: direcao em que a escada encara
void lever(MeshBuilder& mb, const Vec3& pos, float yawDeg, float angleDeg = 35.f);
void gear(MeshBuilder& mb, const Vec3& center, const Vec3& axis, float radius, int teeth = 12, float thickness = 0.25f, int material = MAT_IRON);
void crystalCluster(MeshBuilder& mb, const Vec3& pos, const Vec3& up, float radius, int count, int seed, const QColor& color, bool emissive = true);
void mushroom(MeshBuilder& mb, const Vec3& pos, float height, const QColor& cap = QColor(80, 255, 200), bool glowing = true);
void iceFringe(MeshBuilder& mb, const Vec3& from, const Vec3& to, int count, float maxLen, int seed = 1); // pingentes de gelo ao longo de um segmento (props)
void altar(MeshBuilder& mb, const Vec3& pos, float yawDeg, const Vec3& size = Vec3(2.f, 1.0f, 1.f), int material = MAT_OBSIDIAN);
void totemOfBones(MeshBuilder& mb, const Vec3& pos, float height = 2.2f, int seed = 1);   // fetiche goblin

} // namespace Prefabs
