# A Chama Primordial — Bíblia da Masmorra

Documento de design da caverna navegável. É a fonte da verdade para o conteúdo do programa:
cada área, armadilha, enigma, segredo, criatura e tesouro aqui listado deve existir no mundo 3D,
nas coordenadas indicadas, com uma nota de Mestre (POI) explicando a mecânica.

## Premissa (resumo da campanha)

- **A vila**: cidade próspera no topo de uma montanha gelada, mas estranhamente quente — solo fértil,
  águas termais. De repente começa a esfriar: plantações morrem, a fome chega.
- **A verdade**: o calor vem do **Coração de um Dragão de Fogo** morto há milênios, no âmago da montanha.
  O Coração está **incubando** — vai ressuscitar o dragão ou causar uma erupção que destrói o continente.
- **O vilão**: um **Alhoon** (illithid lich, Caótico Neutro, utilitarista frio). Construiu a **Máquina do Frio**,
  que drena o calor do Coração e o dissipa no vazio, usando **conduítes de carne humana** (pessoas sequestradas
  em cápsulas de vidro e ferro, tubos cravados no peito, mantidas vivas por necromancia). Ele sabe que a vila
  vai congelar e não hesita: "a agonia de cinquenta é o custo de operação para a paz de cinquenta milhões".
- **O intermediário**: **Kaelen**, lenhador/guia da vila. O Alhoon invadiu sua mente se passando pelo
  "Espírito da Montanha"/voz do Dragão. Kaelen sequestra pessoas acreditando que são sacrifícios. É o mid-boss.
- **O bode expiatório**: os **goblins** vivem nos primeiros 2% da caverna, encurralados entre o frio de fora e o
  horror de baixo. Roubaram carvão, cobertores e os **braseiros sagrados de bronze** do padre para não congelar.
  O padre carrancudo vê aquilo empilhado na entrada e conclui que é um ritual para sugar o calor da montanha.
- **Tom**: horror de sobrevivência (como Curse of Strahd), claustrofobia, escuridão absoluta (tochas de
  Shadowdark: 1 hora real), exaustão, frio. Do nível 6 ao 9, 6 sessões quinzenais de 3h.
- **O dilema final**: o maquinário está colapsando; o Alhoon pede que **um aventureiro entre voluntariamente
  na cápsula vazia**. Se o grupo destruir a máquina, a vila se salva, mas a montanha treme: contagem regressiva
  para o apocalipse.

## Convenções do mundo 3D

- Unidade: **metro**. **1 quadrado de D&D (5 ft) = 1,5 m**. 30 ft = 9 m. 60 ft = 18 m.
- Eixo **Y para cima**. **-Z = norte** (para dentro da montanha). +X = leste.
- Origem `(0, 0, 0)`: chão da boca da caverna.
- Altura dos olhos do personagem: 1,65 m. Goblin: ~1,0 m de altura. Túneis goblin: 1,0 m de altura (personagens Médios
  precisam se agachar/rastejar).
- Notação de pontos: `(x, y, z)`. Quando y for do chão, está indicado "chão".
- Ângulo θ no Poço do Eco: ponto = centro + r·(cos θ, ·, sin θ) (x = cos, z = sin), em graus.

## Mapa macro

| Camada | Nome | Faixa Y (m) | Região aproximada | Sessão / Nível |
|---|---|---|---|---|
| 0 | A Encosta Gelada (exterior) | 0 a +60 | x −60..60, z 0..+110 | início |
| 1 | A Garganta Erodida (goblins, "2% iniciais") | 0 a −22 | x −35..40, z 0..−170 | S1 / Nv 6 |
| 2 | O Poço do Eco | −19 a −178 | centro (40, ·, −180), raio 22 | S2 / Nv 7 |
| 2b | Corredor do Contraste Térmico | −178 | x 18..−24, z −180..−182 | S2 / Nv 7 |
| 3 | As Veias Vítreas (domínio de Kaelen) | −178 a −196 | x −150..−20, z −180..−340 | S3 / Nv 7→8 |
| 3b | O Labirinto Alienígena | −200 a −206 | x −125..−65, z −345..−395 | S4 / Nv 8 |
| 4 | O Geodo Primordial | −255 a −206 | centro (20, −232, −370) | S5 / Nv 8 |
| 5 | O Parasita de Ferro (covil do Alhoon) | −300 a −247 | centro (100, −275, −505) | S6 / Nv 9 |

Profundidade total ≈ 300 m. Comprimento do caminho principal ≈ 1,6 km. Equivale a uns 6 Templos de Âmbar
distribuídos entre horizontal e vertical.

**Caminho principal (rota de validação)** — a sequência de pontos que precisa estar conectada por espaço livre:
`(0,0,40)` → `(0,−1,−17)` → `(0,−3,−50)` → ponte `(0,−4,−77)` → `(0,−5,−98)` → `(0,−6,−120)` → `(6,−8,−140)`
→ `(18,−13,−157)` → borda do poço `(27,−19,−167)` → espiral → fundo `(40,−178,−180)` → `(18,−178,−180)`
→ `(−24,−178,−182)` → `(−62,−182,−220)` → `(−75,−186,−255)` → `(−82,−188,−292)` → `(−90,−194,−335)`
→ `(−92,−200,−345)` → `(−92,−202,−370)` → `(−78,−203,−386)` → `(−70,−205,−392)` → balcão `(−36,−214,−383)`
→ escadaria de cristal → `(0,−255,−355)` → `(5,−255,−365)` → `(35,−255,−395)` → `(80,−255,−405)` → porta `(98,−255,−408)`
→ `(115,−265,−430)` → `(110,−290,−470)` → `(100,−300,−480)` → altar `(100,−296,−532)`.

---

## CAMADA 0 — A Encosta Gelada (exterior)

- Terreno nevado. Chão ≈ y 0 junto à boca, descendo suave para o sul (≈ −4 em z = 100). Dunas de neve ±0,8 m.
  Paredões de vale sobem a partir de |x| > 35 (até +30 m) para fechar a cena. Acima da boca, um **penhasco**
  de ~60 m (a montanha), com a base em z ≈ +3 e o topo em z ≈ −8.
- **Spawn**: `(0, 0, 40)`, olhando para o norte (para a boca). Crepúsculo de nevasca: luz azul-acinzentada fraca,
  neve caindo, névoa azulada densa (visibilidade ~60 m). A tocha começa **apagada** ([R] acende).
- **Boca da caverna** em z ≈ 0: 12 m de largura (x −6..6), 9 m de altura. Franja de pingentes de gelo.
- Pistas na neve: trilha de pegadas pequenas (goblins) da boca para o sul; **cobertor da vila** caído em `(3, 0, 18)`;
  pedaços de carvão espalhados entre z 5 e 15.
- Bônus de atmosfera: um brilho quente, muito fraco, lá no alto do penhasco (a vila), visível entre a neve.

POIs: **Trilha dos Ladrões** (pegadas + cobertor: pista de que goblins roubam da vila),
**A Boca Congelada** (descrição da entrada; temperatura −15 °C).

---

## CAMADA 1 — A Garganta Erodida (Sessão 1, Nível 6)

Calcário cinza-marrom úmido, labirintos horizontais apertados moldados por rios subterrâneos (secos e um violento).
Escala opressiva, não vasta. Os goblins **apagam as próprias luzes antes de atacar** e atiram **flechas de barro cheias
de água** para apagar as tochas. Estão desnutridos, desesperados, lutam como ratos encurralados.

### 1.1 Antecâmara do Falso Ritual
- Câmara x −11..11, z 0..−28, chão de y 0 (z 0) descendo a −2 (z −28), altura 9 m. Neve acumulada nos primeiros 10 m.
- **O Falso Ritual** no centro `(0, −1.5, −17)`: 5 **braseiros sagrados de bronze** em círculo de raio 3,5 m, acesos com
  brasas fracas (luz laranja). No centro, sobre uma pedra, o **Relicário do Sol** do templo (disco dourado).
  Pilhas de carvão `(−6, chão, −22)`, lenha `(7, chão, −24)`, cobertores roubados `(−7, chão, −10)`.
  Espirais de ocre vermelho pintadas nas paredes (tentativa goblin de "chamar o calor").
- Duas **seteiras** no alto da parede leste: `(11, +4, −14)` e `(11, +4, −22)`, ligadas ao túnel de rastejo leste.
- Nota do Mestre: é a "prova" do padre. Investigação CD 13 revela que os braseiros estão sendo usados só para
  aquecimento (restos de comida, peles de rato, marcas de dormir em volta). Não há magia.

### 1.2 Corredor das Fendas (Murder Holes)
- De `(0, −2, −28)` a `(0, −4, −70)`, 5 m de largura, 8 m de altura, chão plano.
- **Seteiras** (fendas 0,6 × 0,5 m) a 5 m do chão: parede oeste (x −2,5) em z −34, −46, −58; parede leste (x +2,5)
  em z −40, −52, −64.
- **Túneis de rastejo goblin** paralelos atrás das paredes: oeste em x −4,2 e leste em x +4,2, com o piso no nível das
  seteiras, 1,0 m de altura, 0,9 m de largura, de z −30 a z −68. O leste liga, ao sul, às seteiras da antecâmara;
  ao norte, segue para leste até a Linha Goblin (1.3). O oeste termina num **ninho de atirador** que dá para o abismo
  e para a ponte, em `(−8, 0, −70)`.
- Nota: goblins usam *Nimble Escape* (esconder como ação bônus); atirar das seteiras dá cobertura de 3/4. Personagens
  Médios cabem nos túneis só rastejando (meio deslocamento).

**Armadilhas goblin do corredor**
- **Fosso de Estacas Coberto** — z −46..−49, x −1,5..1,5. 3 m de profundidade, coberto por lona suja e folhas.
  Percepção passiva 14 para notar. Falha: cai 3 m (1d6 contundente) + estacas (2d6 perfurante).
- **Fio de Tropeço** — z −59, na altura do tornozelo. Derruba potes de barro com óleo e pedras de uma prateleira no teto
  (y +4). Percepção CD 13; Destreza CD 13 ou 2d10 contundente e fica coberto de óleo (vulnerável a fogo até se limpar).

### 1.3 O Abismo do Rio Violento e A Ponte Podre
- Abismo leste–oeste: x −32..28, z −70..−84; do chão (y −4) até o leito (y −21). Superfície da água em y −18,5,
  correnteza para leste, sumindo num sumidouro em x 28. **Estacas** fincadas no leito raso sob a ponte
  (x −4..4, z −73..−81).
- **A Ponte Podre**: de `(0, −4, −70)` a `(0, −4, −84)`, 1,5 m de largura, 20 tábuas, 2 postes de cada lado, cordas
  de corrimão, cedendo 0,6 m no meio. Tábuas tortas, algumas faltando.
  Nota: quando alguém de armadura pesada chega ao meio, os goblins **cortam as cordas**. Destreza CD 15 para se agarrar;
  falha: queda de 16 m (5d6) na água gelada, correnteza (Atletismo CD 14 ou é arrastado para o sumidouro).
- **Linha Goblin**: uma corda única esticada sobre o abismo em x +24, entre a saída do túnel de rastejo leste
  `(24, −2, −70)` e uma alcova `(24, −3, −86)` ligada ao acampamento. Goblins (Pequenos) atravessam pendurados;
  Médios: Atletismo CD 15.

### 1.4 A Toca (Acampamento Goblin)
- Câmara x −18..18 (lobo oeste até x −24), z −84..−112, chão y −5, altura 7 m.
- **Galeria alta oeste**: plataforma de rocha x −18..−12, y −1 (4 m acima do chão), z −88..−108; posição de arqueiros.
- Conteúdo: 6 tendas feitas de **cobertores roubados da vila**; braseiro central roubado `(0, −5, −98)`; peles de dormir,
  ossos de rato, restos de comida; ~10 goblins magros encolhidos; **3 filhotes goblin** e **uma anciã** perto do braseiro;
  o chefe **Grukk Dente-Quebrado** (goblin boss esquelético) com o **cálice dourado** do templo.
- Saídas: norte `(0, −5, −112)` para o Labirinto dos Leitos Secos; leste para o Sifão Turvo `(18, −5, −100)`.
- Nota de horror: depois da luta, o silêncio. Os filhotes e a anciã não lutam; só tremem. É o momento de a mesa perceber
  que os goblins não são os vilões.

### 1.5 A Cascata Congelada (passagem secreta)
- Abertura de 3 m na parede oeste do corredor em z −40 → câmara lateral `(−12, −3, −40)`, 10 × 8 m, 6 m de altura.
- **Cascata congelada** na parede norte da câmara `(−12, −3, −44.5)`: lâmina de gelo de 4 m × 7 m, translúcida.
  Atrás dela, uma **fenda natural** (1,0 m de largura, 2,2 m de altura) serpenteia até a borda sul do abismo
  `(−20, −4, −70)`. Lá, uma **estalactite caída** atravessa o abismo como um tronco (raio ~0,6 m) até `(−20, −5, −84)`,
  chegando ao lobo oeste da Toca, por baixo da galeria dos arqueiros.
- Nota: Percepção CD 15 (com luz de tocha encostada no gelo vê-se a sombra da fenda). Quebrar o gelo faz barulho
  (goblins alertados). Atravessar o tronco: Acrobacia CD 12. Rota de flanco que evita a ponte.

### 1.6 O Sifão Turvo (túnel submerso)
- Abertura na parede leste do corredor em z −62 → túnel até um poço de água turva `(30, −5, −62)` (6 × 6 m,
  superfície y −5,5).
- O sifão desce e segue para o norte **totalmente submerso**: `(30,−5,−62)` → `(36,−9,−66)` → `(36,−9,−90)` →
  `(30,−5.5,−94)`, emergindo num poço na alcova leste da Toca. ~28 m debaixo d'água.
- Nota: prender a respiração (1 + mod. CON minutos), água a 2 °C (Constituição CD 12 ou 1 nível de exaustão).
  Sai pelas costas do acampamento: emboscada pelo flanco.

### 1.7 O Labirinto dos Leitos Secos
- z −112..−160, corredores sinuosos de 3–4 m de largura, 3,5 m de altura, chão descendo de −6 a −13.
- Nós: N0 `(0,−5,−112)`, N1 `(0,−6,−120)`, N2 `(−15,−7,−126)`, N3 `(−26,−7,−132)` beco **O Lixão**,
  N4 `(12,−7,−127)`, N5 `(22,−8,−122)` beco **A Despensa Congelada** (carcaças congeladas de cabras e um urso),
  N6 `(−6,−8,−136)`, N7 `(−16,−9,−146)` beco **Galeria Desabada**, N8 `(6,−8,−140)`, N9 `(12,−10,−150)`,
  N10 `(18,−13,−157)`. Ligações: N0–N1, N1–N2, N2–N3, N1–N4, N4–N5, N2–N6, N6–N7, N6–N8, N4–N8, N8–N9, N9–N10.
- **Esconderijo do Chefe** (secreto) atrás de pedras soltas no Lixão: o resto dos tesouros do templo + 120 po.
  Investigação CD 14.

### 1.8 A Fronteira dos Ossos
- Túnel N10 `(18,−13,−157)` → `(22,−16,−162)` → borda do poço `(27,−19,−167)`.
- **Totens de ossos**, fetiches pendurados no teto, arranhões de proteção nas paredes, uma **linha de cinzas e sal** no chão
  em `(21, chão, −160)`. É o último vestígio goblin.
- Nota: a partir daqui (os outros 98%), não há lixo nem fogueiras: só escuridão, frio e um **zumbido psíquico** baixo.
  Os goblins têm pavor do que vive lá embaixo. Temperatura −20 °C.

---

## CAMADA 2 — O Poço do Eco (Sessão 2, Nível 7)

Abismo cilíndrico colossal, escuro como breu. Centro **(40, ·, −180)**, **raio 22 m**, do fundo (y −178) até uma cúpula
(y −12). Calcário úmido, com pingentes de gelo no alto; nos últimos 25 m a rocha vira **obsidiana negra**.

### 2.1 A Borda
- O túnel da Fronteira chega à parede em θ ≈ 133°, y −19. Patamar mais largo (5 m) nos primeiros ~10 m.
- Restos de uma escada de corda partida pendendo no vazio (alguém tentou descer há muito tempo).

### 2.2 As Cornijas em Espiral
- Rampa helicoidal natural colada à parede: começa em θ 133°, y −19, **desce 32 m por volta** (≈ 15°), largura 3 m
  (de r 19 a r 22), até o fundo (≈ 4,9 voltas, terminando em θ ≈ 110°). Altura na volta t: y = −19 − 32·t;
  ângulo θ = 133° + 360°·t.
- **Passo do Desespero**: entre t 0,90 e t 1,05, a cornija estreita para 1,0 m.
- **Degraus Esfarelentos** (armadilha): três trechos de ~6 m onde a cornija é de calcário podre — em t 0,55–0,60,
  t 1,70–1,75 e t 3,30–3,35. Desabam sob peso. Percepção CD 14 (rachaduras), Destreza CD 14 para pular para trás;
  falha = queda até a volta de baixo (32 m: 10d6, ou agarrar a corda de segurança).
- **Vão**: em t 2,50–2,52, um vão natural de 1,3 m (pular).
- **Correntes de Vento** (armadilha ambiental): entre y −80 e −140, rajadas ascendentes a cada ~20 s. Cada rajada apaga
  tochas desprotegidas (Shadowdark). Escudo/capa: Destreza CD 12.

### 2.3 A Ponte da Estalactite Caída
- Uma estalactite de 40 m caiu da cúpula e ficou atravessada no poço, passando pelo centro: liga a cornija em t 1,0
  (θ 133°, y −51) à cornija em t 1,5 (θ 313°, y −67). Raio ~1,6 m, inclinada ~21°. Atalho dramático sobre o vazio.
- Nota: Acrobacia CD 10 (CD 15 com vento). Bom lugar para o Roper atacar com tentáculos.

### 2.4 A Câmara do Roper Glacial
- Caverna lateral saindo da cornija em t 1,25 (θ 223°, y −59). Centro `(13.7, −59, −204.6)`, ~26 × 22 m, 22 m de altura
  (alta e úmida). Teto coberto de **pingentes de gelo**, chão de estalagmites.
- O **Roper** se camufla entre as estalagmites em `(12, −59, −207)`. Não tem ligação com a trama: é um predador de
  caverna que caça goblins e aventureiros. O combate é vertical: ele iça os heróis para a escuridão.
- Ossos de goblins espalhados; **restos de um batedor goblin** com uma bolsa: 2 Poções de Escalada e 45 po.
- Nota: Percepção CD 16 para distinguir o Roper de uma estalagmite (o olho só abre quando a luz chega perto).

### 2.5 O Nicho do Eco (descanso curto)
- Alcova de 6 × 4 m, 3 m de altura, saindo da cornija em t 2,75 (θ 43°, y −107). Centro `(58.3, −107, −162.9)`.
- Nota: único lugar razoável para descanso curto no poço. Dá para barricar com pedras soltas (10 min).
  Descanso longo nas cornijas = exposto a predadores verticais.

### 2.6 O Fundo do Poço
- Chão em y −178: entulho, ossos de tudo o que caiu. Transição de calcário para obsidiana.
- **O Esqueleto Esquecido** em `(48, −178, −186)`: um aventureiro antigo que caiu, com uma corda partida.
  Segura a **Lâmina da Brasa Adormecida** (arma +1): emite calor fraco e **brilha e esquenta mais à medida que o grupo
  desce em direção ao Coração** — uma bússola narrativa.

### 2.7 O Corredor do Contraste Térmico (armadilha)
- Sai do fundo do poço pelo oeste (θ 180°): de `(18, −178, −180)` a `(−24, −178, −182)`. 42 m de comprimento,
  4,5 m de largura, 4,5 m de altura.
- Inundado por **água vulcânica escaldante** (superfície y −178,6, fundo de rocha y −180,5), coberta por uma
  **camada fina de gelo mágico** (topo em y −178,0) criada pelo feitiço do Alhoon. Vapor saindo pelas bordas,
  brilho avermelhado vindo de baixo do gelo.
- 8 placas de gelo de ~5 m; as placas 3 e 6 são **finas** e racham sob peso.
- Nota: Acrobacia CD 13 por placa. Falha: o gelo quebra, 2d6 cortante + frio na queda e **4d10 de fogo** ao mergulhar.
  Correr = desvantagem.

---

## CAMADA 3 — As Veias Vítreas (Sessão 3, Nível 7→8)

Antigos tubos de magma de **obsidiana negra** polida (reflexos vítreos), seção circular de ~7 m de diâmetro com o chão
plano coberto de **cinzas petrificadas**. Quente e seco comparado ao poço (8 °C).

### 3.1 Tubo A (de chegada)
- `(−24,−178,−182)` → `(−48,−180,−196)` → `(−62,−182,−220)` → portão do entroncamento `(−70,−184,−236)`.
- Primeiros sinais de Kaelen: marcas de arrasto no chão de cinza, um sapato infantil, correntes.

### 3.2 O Entroncamento de Kaelen (fortaleza do mid-boss)
- Cúpula onde quatro tubos se encontram. Centro `(−75, −186, −255)`, raio horizontal 22 m, chão y −186,
  teto y −166 (20 m).
- **Paliçada** em anel (raio 15) de madeira podre, **costelas de feras antigas** (ossos gigantes) e **correntes**,
  com o portão voltado para o Tubo A (sudeste).
- **Jaulas penduradas**: 6 jaulas de ferro presas por correntes ao teto, entre raios 6 e 9. Quatro têm **aldeões
  sequestrados** (frio, fome, olhar vazio); duas vazias.
- **Posto de comando de Kaelen** `(−80, −186, −262)`: pódio de obsidiana, mesa com mapas e a **Caderneta de Kaelen**.
  A caderneta não tem orações: tem **anotações frias e matemáticas sobre a "durabilidade" de cada sequestrado** e
  referências à **"Máquina do Frio"**.
- **Kaelen**: humano alto, machado de lenhador, capa de pele; aura de frio necrótico. Stat block de Warlock/Blackguard
  (dano necrótico e de frio). Frases de combate: "O fogo exige combustível!", "Eu carrego o peso para que a vila não
  precise congelar!".
- **Canil**: 2 **Hell Hounds corrompidos** (lobos da montanha distorcidos pela magia do Alhoon: brasas azuis nos olhos)
  em `(−66, −186, −266)`.
- **Braseiro de chama fria** (fogo azul que não aquece) no centro.
- **Baú de Kaelen** `(−84, −186, −258)`: **Anel de Aquecimento (Ring of Warmth)** — o item mais cobiçado da campanha.

### 3.3 Os Dutos de Respiro (passagem secreta)
- **Tubo D** sai da cúpula para o leste `(−53,−186,−258)` → `(−38,−186,−262)` e termina num beco de magma colapsado.
- No beco, uma **chaminé vertical** (escalável, 16 m) de `(−36,−186,−263)` até `(−36,−170,−263)`.
- No alto, um **duto de respiro** (≈1,3 m de diâmetro, rastejar) corre para oeste **por cima da cúpula**:
  `(−36,−170,−263)` → `(−55,−164,−258)` → `(−75,−163,−255)` → `(−95,−165,−275)`, e desce por outra chaminé até o
  Tubo C `(−85,−190,−288)`.
- **Olhos do Respiro**: 3 buracos verticais do duto até o teto da cúpula em `(−62,·,−257)`, `(−75,·,−255)`,
  `(−86,·,−252)`. Dá para espionar Kaelen e ouvir seus delírios antes de atacar, ou cair sobre as jaulas.
- Nota: Percepção CD 15 para notar a corrente de ar no beco. Atletismo CD 12 para escalar a chaminé.
  É também um desvio que contorna a fortaleza.

### 3.4 A Passagem dos Fungos da Memória
- **Tubo B** para oeste: `(−97,−186,−255)` → `(−115,−188,−250)` → `(−132,−189,−258)` → **Bosque Micônida**
  (câmara de raio 10 em `(−145, −190, −265)`).
- Paredes e chão cobertos de **fungos bioluminescentes** (ciano-esverdeados), nuvens de esporos. 6–8 **Micônidas**
  isolados e pacíficos.
- **Alucinação**: respirar os esporos faz os jogadores enxergarem a caverna como era há milênios — **o interior
  vulcânico e pulsante de um dragão titânico** (paredes de carne, veias de fogo pulsando). Entrega a lore sem NPCs.
- Nota: Constituição CD 13 ou fica Envenenado 1 hora com visões. O Soberano Micônida pode fazer o "Rapport de Esporos"
  e mostrar a memória do Dragão. Tesouro: **Esporo do Sonho Antigo** (uma visão extra a critério do Mestre).

### 3.5 A Fenda da Guerra Elemental
- **Tubo C** para o norte: `(−78,−186,−277)` → `(−82,−188,−292)` → início da fenda `(−85,−189,−300)`.
- Fenda alta e estreita de `(−85,−189,−300)` a `(−90,−194,−335)`: 6 m de largura, 30 m de altura.
  A parede **leste é congelada** (gelo, geada), a **oeste é derretida** (rachaduras de magma).
- **8 Mephits de Gelo** (atraídos pela magia dissipadora do Alhoon) contra **8 Mephits de Magma** (nascidos do calor
  residual do Dragão), lutando no alto da fenda. Flocos de neve contra brasas.
- Nota: Furtividade CD 13 para passar pelo fogo cruzado, ou manipular uma facção contra a outra.

---

## CAMADA 3b — O Labirinto Alienígena (Sessão 4, Nível 8)

O grupo entra na área de influência psíquica do Alhoon. A geometria foi **alterada por intelecto alienígena**: corredores
lisos demais, curvas antinaturais, rocha com veios roxos levemente brilhantes. 3 m de largura, 3,5 m de altura.

- Entrada: descida de `(−90,−194,−335)` a L0 `(−92,−200,−345)`.
- Nós: L0 `(−92,−200,−345)`, L1 `(−92,−200,−355)`, L2 `(−108,−201,−355)`, L3 `(−120,−201,−362)` beco **Ninho do Grell**,
  L4 `(−108,−201,−370)`, L5 `(−92,−202,−370)`, L6 `(−78,−202,−362)`, L7 `(−92,−202,−384)`,
  L8 `(−108,−203,−386)` → **Câmara da Paranoia** `(−115,−203,−392)`, L9 `(−78,−203,−386)`, L10 `(−70,−205,−392)` saída.
  Ligações: L0–L1, L1–L2, L2–L3, L2–L4, L4–L5, L1–L6, L5–L6, L5–L7, L7–L8, L7–L9, L9–L10, L6–L9.
- **Paredes Mutantes**: duas paredes que mudam de lugar quando ninguém está olhando (quando o jogador vira as costas),
  alternando entre bloquear L5–L7 e L6–L9. Corredores que causam confusão mental.
- **Zumbido psíquico**: distorção visual leve e sussurros ("...a matemática não mente...").
  Nota: Sabedoria/Inteligência CD 14 ao entrar e a cada descanso; falha = 1 nível de **Exaustão Psíquica**.
- **Aberrações no teto**: 2 **Devoradores de Intelecto** rastejando no teto (entre L2–L4 e L7–L9) e 1 **Grell** no Ninho.
- **A Câmara da Paranoia** (descanso da Sessão 4): beco de 10 m com entrada estreita que pode ser bloqueada por uma pedra.
  Nota: durante o descanso o Alhoon invade a mente deles (sonhos com cápsulas, a voz do "Espírito da Montanha").
  **Diário de um Sequestrado** caído aqui (fugiu de Kaelen e morreu perdido).

---

## CAMADA 4 — O Geodo Primordial (Sessão 5, Nível 8)

Câmara horizontal do tamanho de uma pequena cidade, como o interior de um geodo: **elipsoide** com centro
**(20, −232, −370)**, raios **(80, 26, 55)**, chão achatado em **y −255**, teto em ~−206. Paredes de ametista roxa com
pontas de quartzo; **cristais do tamanho de torres** brotam do chão e do teto (alguns com brilho interno). As armadilhas
aqui são o **sistema imunológico do Coração do Dragão**: anomalias elementais moldadas pelo subconsciente da fera.

### 4.1 O Balcão do Geodo
- Saída do labirinto: L10 → `(−55,−209,−388)` → `(−40,−213,−384)` → **balcão** `(−36, −214, −383)` (plataforma
  saliente na parede oeste, 41 m acima do chão). A primeira vista do geodo deve ser de tirar o fôlego.

### 4.2 A Escadaria de Cristal
- Ziguezague de **cristais gigantes caídos** apoiados em topos de torres de cristal:
  balcão → P1 `(−20,−224,−400)` → P2 `(−5,−234,−385)` → P3 `(−15,−244,−368)` → chão `(0,−255,−355)`.
  Rampas de ~25°.

### 4.3 O Selo dos Prismas (quebra-cabeça)
- **Fonte**: a **Fenda da Chama** no chão em `(−20, −255, −340)` vaza luz do fogo do dragão; um cristal coletor de
  quartzo acima dela emite um **feixe horizontal para leste** na altura y −253,5.
- **4 prismas giratórios** em pedestais (feixe a y −253,5): P1 `(5,·,−340)`, P2 `(5,·,−365)`, P3 `(35,·,−365)`,
  P4 `(35,·,−405)`. **Iscas**: D1 `(5,·,−320)` e D2 `(60,·,−365)`. Cada prisma gira em passos de 45° e reflete o feixe.
- **Alvo**: a **Muralha de Gelo Místico** que sela a passagem leste para a Porta de Adamante, em `(80, −255, −405)`.
- Solução: P1 desvia para o norte → P2 para o leste → P3 para o norte → P4 para o leste → derrete o gelo.
- Nota: cada tentativa errada consome tempo de tocha (10 min de jogo). Versão "Shadowdark": usar a luz das próprias tochas
  que estão acabando. Variante: a mesma mecânica derrete **rios de magma solidificado**.

### 4.4 A Zona de Gravidade Distorcida
- Região leste: caixa x 45..80, y −244..−212, z −360..−320, onde **a gravidade aponta para o teto**.
- Um **planalto de cristal** em `(60, −247, −340)` (8 m acima do chão). Pulando dele para dentro da zona, o personagem
  "cai para cima" e pousa em **estalagmites invertidas** (mesas de cristal penduradas no teto, base em ~−238).
  Caminhando "de cabeça para baixo" chega-se a uma cavidade no teto: **O Ninho de Cristal** `(72, −220, −335)`.
- Tesouro: **Escama do Dragão Ancestral** (amuleto: resistência a fogo; está morna ao toque).
- Nota: Sabedoria CD 12 ao entrar ou fica atordoado 1 rodada (náusea). Saltar para fora da zona inverte de volta.

### 4.5 O Campo de Gêiseres
- 9 respiros de **vapor superaquecido** numa grade 3 × 3 (espaçamento 7 m) em x 15..45, z −322..−336.
- Disparam em **padrão** (cada um 2 s, em sequência, ciclo de 12 s). Quem observa 1 ciclo deduz a ordem.
- Nota: Inteligência (Investigação) CD 13 para ler o padrão; atingido: Constituição CD 14, 4d6 fogo.

### 4.6 A Maré de Magma
- **Rio de magma** atravessando o noroeste do geodo ao longo de z ≈ −405, de x −55 a x 0 (sumidouro).
  O nível **sobe e desce como maré** (entre −256 e −254,5, período ~40 s).
- **Pedras de travessia** em x −25, z −402..−408: expostas na maré baixa, submersas na alta.
- Nota: controlar o fluxo redirecionando os cristais é uma solução alternativa (a critério do Mestre).

### 4.7 A Câmara do Ar Rarefeito
- Além do rio de magma: túnel de `(−25,−255,−412)` para o norte até a câmara `(−30, −253, −435)` (raio 9).
- O ar é tão rarefeito que **chamas encolhem** (a tocha fica fraca) e **qualquer magia de fogo explode com o dobro
  da área**, causando desmoronamentos.
- Tesouro: **Pedra de Fogo Elemental** (granada: Bola de Fogo 3º nível; aqui, área dobrada).

### 4.8 A Porta de Adamante e os Selos de Exaustão
- Passagem de `(80, −255, −405)` (atrás da Muralha de Gelo) para o leste até a **Porta de Adamante** em
  `(98, −255, −408)`: 8 m de largura, 10 m de altura, metal verde-negro (construção do Alhoon).
- **Três engrenagens arcanas frias** (cobertas de geada) cravadas na porta. O mecanismo não exige força, mas
  **calor vital**: para girar cada uma, um personagem transfere voluntariamente seu calor — **gasta Dados de Vida ou
  aceita 1 nível de Exaustão**. Com as 3 giradas, a porta afunda no chão.
- Nota: introduz mecanicamente o tema utilitarista do sacrifício do Alhoon.

---

## CAMADA 5 — O Parasita de Ferro (Sessão 6, Nível 9)

O único lugar com verdadeira intervenção arquitetônica. A rocha **pulsa com o calor vermelho** do Coração do Dragão.
Arquitetura **brutalista e profana**: não há alvenaria, só **andaimes de ferro enferrujado cravados violentamente** na
rocha natural. 45 °C.

### 5.1 A Descida de Ferro
- Atrás da porta: túnel descendo com **escadas de grade de ferro**: `(104,−255,−410)` → `(115,−265,−430)` →
  `(118,−278,−455)` → `(110,−290,−470)` → entrada da câmara `(108,−296,−462)`.

### 5.2 A Câmara do Coração
- Cavidade principal: elipsoide com centro **(100, −275, −505)**, raios **(48, 28, 45)**, chão em **y −300**.
- **As Costelas Fósseis**: 8 pares de costelas gigantes do dragão morto arqueando do chão até a **espinha** que corre
  pelo teto (eixo norte–sul). O **crânio do dragão** emerge da parede norte `(100, −280, −548)`, boca aberta, órbitas
  com brilho fraco. A caverna inteira é o corpo do dragão.
- **O Coração do Dragão**: centro `(100, −287, −505)`, raio ~8 m, órgão incandescente **pulsando** (ciclo ~1,8 s),
  veias de fogo, sobre um pedestal de rocha.
- **Fosso de Magma**: anel entre raio 22 e 30 em torno do Coração (superfície y −301).
- **A Ponte de Ferro**: atravessa o fosso da entrada sul até o anel de andaimes.

### 5.3 Os Andaimes e as Cápsulas (A Engenharia do Sofrimento)
- **3 anéis de andaime** de grade de ferro (raio 16, largura 3 m) em y −300, −292 e −284, ligados por escadas de ferro,
  com vigas diagonais cravadas nas paredes.
- **36 cápsulas** (12 por anel): cilindros de vidro e ferro (2,4 m) com líquido necromântico esverdeado; dentro, um
  **humano com tubos cravados no peito**, pele queimando e cicatrizando sem parar. **Tubos de bronze** ligam cada cápsula
  ao Coração. Algumas cápsulas estão **esgotadas** (vidro rachado, escuro: as mentes viraram pó).
- **A Vaga**: uma cápsula **vazia e aberta** no anel do meio, voltada para o altar. É a vaga que o Alhoon pede que um
  aventureiro ocupe.

### 5.4 As Sanguessugas Mecânicas
- 10 **cilindros de vidro** com líquido necromântico e **tubos de bronze** pendendo do teto irregular como sanguessugas
  mecânicas, drenando o calor para as **baterias arcanas**.

### 5.5 O Dissipador do Vazio e as Baterias Arcanas
- Acima do Coração, um **disco negro giratório** (o Vazio) em `(100, −256, −505)`: um fluxo de partículas de calor sobe
  do Coração e some nele. É literalmente a energia do continente sendo jogada fora.
- **4 baterias arcanas** (bobinas de ferro com núcleos violeta) nas diagonais, a ~25 m do centro.

### 5.6 O Altar do Alhoon
- Plataforma elevada ao norte `(100, −296, −532)` (4 m acima do chão), com escadas, púlpito de controle com alavancas e runas.
- **O Alhoon** em `(100, −296, −534)`: manto púrpura, rosto de tentáculos, olhos brancos brilhantes. Exausto, focado em
  manter o maquinário. Fala: *"Se vocês me matarem e devolverem o fogo à montanha, a cidade viverá por mais um mês, e então
  o céu choverá cinzas e o continente queimará. Sentem-se. Aqueçam-se no fogo que resta, e assistam essa cidade morrer para
  que o mundo viva."*
- **Os Diários de Calibração** (mesas em `(94,−296,−530)` e `(106,−296,−530)`): planilhas que catalogam o ponto exato de
  ruptura psicológica e a **eficiência termodinâmica da dor de cada raça**. Ele anota os gritos como dados.
  *"A agonia de cinquenta é o custo de operação para a paz de cinquenta milhões."*

### 5.7 O Arquivo de Calibração (sala lateral)
- Alcova a oeste `(55, −298, −500)`, raio 7: estantes de diários, frascos de espécimes, a **filactéria** escondida
  (segredo do Mestre: Investigação CD 18).

### 5.8 O Colapso
- Se a máquina for destruída: o fogo volta à montanha (a vila e o menino doente se curam), mas um **tremor grave**
  abala tudo: rochas caindo, luz vermelha pulsando, poeira. Fuga alucinada para a superfície.
- No programa: evento de Mestre "Colapso" ([K] no modo Mestre).

---

## Tesouros (resumo)

| Tesouro | Onde | Coordenada |
|---|---|---|
| Cálice dourado do templo | Grukk (Toca) | (0, −5, −100) |
| Esconderijo do Chefe (120 po + relíquias) | Lixão (secreto) | (−26, −7, −132) |
| 2 Poções de Escalada + 45 po | Câmara do Roper | (15, −59, −200) |
| **Lâmina da Brasa Adormecida** (+1, bússola de calor) | Fundo do Poço | (48, −178, −186) |
| **Anel de Aquecimento** | Baú de Kaelen | (−84, −186, −258) |
| Caderneta de Kaelen (lore) | Posto de Kaelen | (−80, −186, −262) |
| Esporo do Sonho Antigo | Bosque Micônida | (−145, −190, −265) |
| Diário de um Sequestrado (lore) | Câmara da Paranoia | (−115, −203, −392) |
| Escama do Dragão Ancestral | Ninho de Cristal | (72, −220, −335) |
| Pedra de Fogo Elemental | Câmara do Ar Rarefeito | (−30, −253, −435) |
| Diários de Calibração (lore) | Altar do Alhoon | (100, −296, −530) |
| Filactéria do Alhoon (segredo) | Arquivo de Calibração | (55, −298, −500) |

## Mecânicas globais simuladas no programa

- **Tocha (Shadowdark)**: dura **1 hora real**. Luz forte até ~9 m (30 ft), fraca até ~15 m. 5 tochas no inventário.
  [R] acende uma nova. Vento, água e flechas de barro apagam.
- **Visão no escuro**: [V]. Só **visão penumbrosa** em tons de cinza até 18 m (60 ft); não revela armadilhas.
- **Exaustão**: contador no HUD (Selos de Exaustão, frio, labirinto).
- **Temperatura**: indicador por região (−15 °C na entrada, −20 °C no poço, 8 °C nas veias, 45 °C no Parasita).
- **Lâmina da Brasa**: depois de pega, um indicador de calor no HUD cresce com a proximidade do Coração.

## Ajustes feitos na implementação (valem sobre o texto acima)

- **Camada 1**: há uma **trilha de cabras** na parede norte do abismo (quem cai no rio consegue voltar). Os túneis de
  rastejo **não têm entrada pelo corredor**: o acesso é pela Linha Goblin ou pelo ninho de atirador; a alcova nordeste da
  Linha Goblin tem um segundo túnel até a Toca. A parede sul da Toca fica em z −86, com 3 aberturas (ponte, tronco, Linha Goblin).
- **Camada 3**: o duto de respiro tem ~2,1 m de largura (1,3 m não deixava passar). A paliçada tem dois portões (sul e norte)
  e um anel livre de 7 m em volta, para o desvio pelo Tubo D continuar acessível.
- **Camada 4**: o geodo foi aumentado para caber o que está nele: centro **(20, −240, −370)**, raios **(80, 34, 60)**, chão
  útil ~140 × 105 m. Torres da escadaria em P1 (−9, −407), P2 (8, −384), P3 (−15, −368). Solução dos prismas: P1 norte,
  P2 leste, P3 norte, P4 leste (estado inicial: sul, oeste, leste, sul). A volta do Ninho de Cristal é só de ida sem
  Queda Suave, Levitação ou corda (degraus de 2,4 m).
- **Camada 5**: câmara com centro **(100, −285, −505)**, raios **(48, 38, 45)**. A boca do túnel de ferro chega em
  (121, −284, −470) e desce por uma passarela em espiral na parede sudeste. A frente do altar fica em z −529,5; o
  Arquivo de Calibração fica em x 37..55, atrás de um corredor.
- **Teclas de evento**: G emboscada goblin · B cortar a Ponte Podre · U rajada de vento no poço · Y Kaelen percebe o
  grupo · N restaurar degraus e gelo · J resolver os prismas · X abrir a Porta de Adamante · K colapso.
