# Masmorra — Referência do motor para autores de conteúdo

Explorador 3D em primeira pessoa da masmorra de `docs/MASMORRA.md` (Qt 5.15 + OpenGL 3.3 core, sem Qt3D).
Cada camada é **uma função** em `src/content/LayerN_*.cpp` que recebe `World&` e usa **só esta API**
(mais `content/Prefabs.h`). Nunca edite `Dungeon.cpp`, o motor ou arquivos de outra camada.
Exemplo vivo de tudo: `src/content/TestLevel.cpp` (Sala de Testes, em x ≈ +600).

## 1. Convenções

- Unidade **metro**; **Y para cima**; **−Z = norte**; +X = leste (igual à bíblia). 1 quadrado D&D = 1,5 m.
- **Yaw** em graus: 0 = olhando para −Z (norte); positivo gira para a **esquerda** (oeste). Pitch positivo = olhar para cima.
  Direção para frente = `(−sin yaw, 0, −cos yaw)`.
- **SDF**: `d > 0` = ar, `d < 0` = rocha. A malha visual e a colisão usam exatamente a mesma função.
- **Pontos de vista (`addViewpoint`) e `setSpawn` usam a posição dos OLHOS** (pés = olhos − 1,65 m).
  POIs e pontos de rota ficam em qualquer ponto de ar (a rota deve ser ar livre a ≥ 0,4 m da rocha).
- Todo texto ao usuário em **pt-BR com acentos**; fontes UTF-8; identificadores em inglês.
- Teclas **G K J U Y N B X** são reservadas para eventos de conteúdo (`registerEvent`). O motor não as usa.
- Limites do mundo (grade de aceleração): x −300..780, y −420..140, z −740..280. Fora disso é rocha maciça.

## 2. Geometria (SDF)

```cpp
void World::carve(const Prim&);   void World::fill(const Prim&);   // ar / rocha
```
`Prim` (src/world/Sdf.h) tem fábricas e modificadores encadeados:

| Fábrica | Uso |
|---|---|
| `Prim::sphere(c, r)` | bolsão |
| `Prim::ellipsoid(c, radii)` | câmaras, geodo |
| `Prim::box(c, half, quat, round)` | caixa rotacionada e arredondada |
| `Prim::capsule(a, b, r0, r1)` | **cavalo de batalha dos túneis** (round-cone a→b) |
| `Prim::cone(apex, base, baseR, tipR)` | estalactites/estalagmites (use `Fill`) |
| `Prim::cylinder(centro, r, yMin, yMax)` | poços verticais (fundo/topo planos) |
| `Prim::helix(HelixDesc)` | cornijas em espiral (Fill). `startY` = altura da **superfície de cima** em t=0; `t0..t1` em voltas; ângulo `x=cos, z=sin` |
| `Prim::annulus(c, rIn, rOut, yMin, yMax)` | fossos/anéis |

Modificadores: `.smooth(k)` (mistura suave), `.noise(amp_m)` (0,2–0,6 típico; mantenha ≤ ~1,2),
`.mat(MAT_*)`, `.floorAt(y)` (corta a cavidade num chão plano em y), `.floorRel(f)` (capsule: chão em `eixo.y − raio·f`,
interpolado ao longo do segmento), `.named("tag")`. O ruído é reduzido perto do chão.

Composição: `air = smin(carves)`; `rock = −air`; terreno (`max(M, rock)`); `rock = smin(rock, fills)`.
(Fills entram **depois** do terreno, então funcionam também no exterior.)

Helpers de `World`:
```cpp
TunnelOpts o; o.floorFrac = 0.55f; o.material = MAT_LIMESTONE; o.noise = 0.4f; o.k = 1.5f;
w.carveTunnel(pts, {raio0, raio1, ...} /*ou 1 raio*/, o);        // pts são pontos do EIXO
w.carveChamber(centro, radii, floorY, MAT_GEODE, noise, k, "tag");
w.stalactite(pontoNoTeto, comprimento, raioBase); w.stalagmite(pontoNoChao, comprimento, raioBase);
w.scatterStalactites(Aabb regiao, count, seed, minLen, maxLen, minR, maxR, mat);   // devolve quantas criou
w.scatterStalagmites(...);
w.setTerrain([](float x, float z){ return altura; }, Aabb exteriorBox);            // vários terrenos permitidos
```
- **Chão em túneis**: o chão fica em `eixo.y − raio·floorFrac`. Para o chão ficar em `y_chão`, use `eixo.y = y_chão + raio·floorFrac`
  (0,55 por padrão). O helper `tun()` em `TestLevel.cpp` faz isso. Túneis para rastejar: raio ≈ 0,7–0,8.
- **Terreno**: `h(x,z)` vale só dentro de `exteriorBox`; fora dela é montanha sólida (e acima do topo da caixa também).
  Garanta `h` ≥ topo da caixa nas bordas para fechar a cena. Material automático = neve.
- Estalactites/estalagmites **escavam depois** que a caverna foi escavada: chame `scatter*` *após* os `carve`.
- Ordem importa só para materiais/mistura; o resultado é determinístico (o cache de malha depende de um hash de todas as primitivas).

**Materiais de rocha** (`MAT_*`): `LIMESTONE`, `WET_LIMESTONE`, `OBSIDIAN` (chão vira cinza petrificada), `GEODE` (ametista + quartzo),
`HOT_BASALT` (rachaduras que pulsam), `ALTERED` (labirinto psíquico: liso, veios roxos), `ICE`, `SNOW`, `ASH_FLOOR`.
Faces voltadas para cima ganham aparência de chão (terra/cinza/neve) automaticamente.

## 3. Props, criaturas e objetos dinâmicos

`MeshBuilder` (src/render/MeshBuilder.h): pilha `push/pop/translate/rotate/rotateY/scale/place`, e primitivas
`box, boxMinMax, cylinder(a,b,r0,r1,seg), cone, sphere, ellipsoid, capsule, torus, hexPrism, quad, triangle,
tube(polilinha, raios), catenary(a,b,flecha)` — cada uma com `QColor` + `MAT_*`. `setFlow(dx,dz)` define o fluxo de líquidos.
Materiais transparentes (vidro, gelo, cristal, água…) vão automaticamente para o passe transparente (alpha = alfa da cor).

**Materiais de prop**: `WOOD, ROTTEN_WOOD, ROPE, CLOTH, HIDE, BONE, RUSTY_IRON, IRON, BRONZE, GOLD, GLASS*, CLEAR_ICE*, CRYSTAL*,
FLESH, SKIN, FUNGUS (brilha), EMISSIVE (cor pura), COAL, PAPER, ADAMANTINE`; **líquidos animados**: `WATER*, BOILING_WATER*, LAVA, NECRO_LIQUID*`
(* = transparente).

```cpp
MeshBuilder mb; Prefabs::goblin(mb, pos, yaw); Prefabs::brazier(mb, pos);
w.addStatic(std::move(mb));                         // estático: buckets de 24 m com frustum culling
DynHandle d = w.addDynamic("nome", std::move(mb), pos, quat, scale);   // d->setPosition/setRotation/setYaw/setTransform/setVisible, d->scl
```
**Desempenho**: rocha = SDF; muitos cristais/ossos/objetos pequenos = props (`addStatic`); evite centenas de primitivas SDF
pequenas (cada uma custa em toda amostra de malha próxima). Orçamento de referência: ~1,7 M de vértices ≈ 160 fps na RTX 3050.

**Prefabs** (`content/Prefabs.h`, base no chão, yaw em graus): `humanoid(HumanoidParams{height,build,skin,cloth,pose,cape,hood,glowingEyes,axe})`
(poses `Stand, Crouch, Huddle, Lie, Hang, Sit`), `goblin`, `goblinChild`, `wolf(hellHound)`, `brazier`, `wallTorch`, `campfire`, `crate`, `barrel`, `sack`,
`hideTent`, `bonesPile`, `skull`, `ribcage`, `giantRib`, `chain`, `hangingCage`, `ropeBridge`, `table`, `chest`, `glassCapsule`, `pipe`,
`scaffoldSegment`, `gratedWalkway`, `ironStairs`, `ladder`, `lever`, `gear`, `crystalCluster`, `mushroom`, `iceFringe`, `altar`, `totemOfBones`.
Estalactite/estalagmite de **rocha** = `World::stalactite/stalagmite` (Fill SDF), não prop.

## 4. Colisores
```cpp
Collider* c = w.addBoxCollider("ponte", centro, half, quat, /*openable*/ false);   // também Cylinder (eixo Y), Capsule, Sphere
w.attachCollider(c, dynHandle);   // center/rot viram locais ao objeto e seguem sua transformação
c->enabled = false;               // desativa
```
`openable = true` (portas, muros de gelo, paredes mutantes) → ignorado por `--validate`. Colisores são **somados ao SDF** (jogador e `raycast`).

## 5. Luzes, partículas, feixes
```cpp
Light* l = w.addLight(pos, Vec3(1,.55,.2) /*cor*/, 6.f /*intensidade*/, 13.f /*alcance*/, LightAnim::TorchFlicker);
l->enabled = false; l->intensity = ...;   // alterável em tempo de execução. Anim: None, TorchFlicker, Pulse(animFreq, animDepth), Custom(l->custom = [](float t){...})
w.addEmitter(ParticlePreset::Embers, pos, halfExtent, rate);   // Snow, Embers, Spores, Steam, Dust, Sparks, VoidStream (EmitterDesc::target); h->setEnabled/setPosition
Beam* b = w.addBeam("raio", a, b, cor, largura);                // faixa aditiva; atualize b->a / b->b / b->visible
```
Até ~32 luzes por quadro (as mais relevantes). Atenuação suave com janela; a tocha do jogador (5,5; forte ~9 m, fraca até ~15 m) é separada. Teto de 20 k partículas.

## 6. Zonas, gatilhos, interativos
```cpp
ZoneEffects fx; fx.areaName = "Camada 1 · A Garganta"; fx.subtitle = "Sessão 1 · Nível 6";   // banner ao entrar
fx.temperature = -20; fx.fogColor = Vec3(...); fx.fogDensity = 0.03f; fx.ambientColor = ...; fx.ambientGround = ...; fx.ambientStrength = 1;
fx.gravity = Vec3(0, 9.81f, 0); fx.water = true; fx.waterSurfaceY = -18.5f; fx.climb = true;
fx.wind = true; fx.windPeriod = 20; fx.windDuration = 2;      // rajada periódica; apaga a tocha
fx.torchScale = 0.5f; fx.hallucination = .8f; fx.psychic = .6f; fx.hazard = true; fx.hazardText = "Lava: 10d10 de dano de fogo!";
w.addZone("nome", ZoneShape::makeBox(mn, mx) /*ou makeSphere, makeCylinder*/, fx, /*prioridade*/ 1);
```
Por propriedade, vence a zona de maior prioridade que contém o centro do corpo; valores contínuos se misturam em ~0,7 s. `w.setDefaultZone(fx)` = base.
`addTrigger(nome, shape, onEnter, onExit, once)`; `addInteractable(nome, pos, raio, "prompt", callback)` (olhar ≤ 3 m, com linha de visada; **E**).
Água: nada quando a profundidade > 1,35 m (Espaço sobe, Ctrl desce, barra de fôlego, tinta submersa; a tocha apaga). Escalada: `climb` + parede à frente.
Gravidade qualquer: a câmera rola em ~0,6 s para −gravidade. Perigo: flash + texto + retorno ao último chão seguro em ~1,5 s.
Queda: 1d6 por 3 m (máx. 20d6) em toast vermelho.

## 7. Notas do Mestre, pontos de vista, rota, eventos
```cpp
w.addPoi(pos, PoiCategory::Trap /*Area,Trap,Puzzle,Secret,Creature,Treasure,Lore,Rest*/, "Título", "texto\nmulti-linha", "Sessão 1 · Nível 6", "Área opcional");
w.addViewpoint("Nome", posOlhos, yaw, pitch, /*voo*/ false, "Grupo");   // aba Áreas do Tab e --shots
w.addRoutePoint("nome", pos);                                          // --validate: pares consecutivos devem estar conectados
w.setSpawn(posOlhos, yaw, pitch);
w.registerEvent("colapso", "Colapso", Qt::Key_K, [&]{ ... });          // aba Eventos + tecla
w.addUpdater([&](float dt, float t){ ... });                           // todo quadro
w.setOverview(centro, raio);                                           // vista geral (O)
```
POI a ≤ 4 m (olhando): `[E] Notas do Mestre: <título>`; no Modo Mestre (F2/F4/O) todos viram rótulos coloridos.

## 8. Consultas
`w.raycast(origem, dir, maxDist, includeOpenable=true)` → `RayHit{hit,t,pos,normal,collider}` (sphere tracing sobre rocha + colisores);
`w.isVisibleToPlayer(p)` (no frustum e sem oclusão — paredes que mudam de lugar); `w.lineOfSight(a,b)`; `w.solidDistance(p)`;
`w.playerPos()/playerEye()`; `w.teleportPlayer(pés, yaw, pitch)`; `w.zoneState()`; `w.time()`; `w.areaAt(p)`.
`w.state` (`GameState`): `torchLit/torchSeconds` (tocha dura **3600 s**, 5 no inventário), `exhaustion`, `hitDiceSpent`,
`addItem(nome)`, `setFlag/flag/hasFlag`, `toast(texto, cor)`, `damageText`, `screenShake`, `fade`, `setMeter(nome, 0..1, cor)` (ex.: bússola de calor da Lâmina), `extinguishTorch`.

## 9. Teclas (também na ajuda F1)
WASD, mouse, Espaço, Ctrl (agachar), Shift, **E**, **R** (nova tocha), **V** (visão no escuro), F1 ajuda, **F2** Modo Mestre (voo, roda = velocidade),
**F3** Luz do Mestre, **F4** rótulos, **F5** zonas/gatilhos/colisores, **F6** wireframe, **F7** plano de corte (`[` `]`, F8 eixo), **F9** esconder terreno / raio-x,
**O** vista geral (arrastar gira, roda dá zoom, clique no rótulo teleporta), **Tab** menu (Áreas, Notas do Mestre, Eventos), F10 estatísticas, **F12** captura, Esc.

## 10. Build, testes e revisão
Build (PowerShell; só no diretório de build do scratchpad): ver `CMakeLists.txt` (MinGW 8.1, `-O2` em todas as configurações). Executável: `Masmorra`.
```
Masmorra.exe                                   # jogo (carrega a masmorra; cache de malha em <exe>/cache)
Masmorra.exe --test-level                      # só a Sala de Testes
Masmorra.exe --start "Poço"                    # começa no ponto de vista cujo nome contém o texto
Masmorra.exe --validate                        # rota conectada? viewpoints/POIs dentro da rocha? -> validate_report.txt
Masmorra.exe --shots out --filter "Camada 1" [--shots-mode explore|dm|both] [--size 1600x900]   # <NN>_<nome>.png e _mestre.png
Masmorra.exe --no-cache | --bench 8 | --stress | --selftest
```
`--selftest` simula o jogador sem GL (andar, pular, nadar, escalar, gravidade, queda, lava, ponte) na Sala de Testes.
`--stress` carrega geometria sintética na escala da masmorra real (mede malhagem). Ao mudar o algoritmo do mesher, incremente `Mesher::kVersion`.
Dica: teste uma camada com `--validate` e `--shots <dir> --filter "<nome da sua camada>"`, e **olhe** os PNGs.
