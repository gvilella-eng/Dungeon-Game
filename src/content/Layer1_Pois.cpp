// Camada 1 — notas do Mestre (POIs) e pontos de vista.
#include "Layer1_Common.h"

namespace L1 {

void buildPois(World& w)
{
    const QString S = Q("Sessão 1 · Nível 6");
    const QString A11 = Q("1.1 Antecâmara do Falso Ritual"), A12 = Q("1.2 Corredor das Fendas"), A13 = Q("1.3 O Abismo do Rio Violento"),
                  A14 = Q("1.4 A Toca"), A15 = Q("1.5 A Cascata Congelada"), A16 = Q("1.6 O Sifão Turvo"),
                  A17 = Q("1.7 O Labirinto dos Leitos Secos"), A18 = Q("1.8 A Fronteira dos Ossos");

    // =============================================================== 1.1 ANTECAMARA
    w.addPoi(V(0, 1.2f, -7.f), PoiCategory::Area, Q("1.1 Antecâmara do Falso Ritual"),
        Q("O QUE É: câmara de x −11..11, z 0..−28, teto a 9 m, chão que desce de 0 a −2. Nos primeiros 10 m há neve acumulada que o vento empurrou para dentro; depois, rocha úmida e o cheiro de fumaça velha. Temperatura ≈ −10 °C junto aos braseiros.\n\n"
          "O QUE OS JOGADORES PERCEBEM: um brilho laranja fraco no centro da câmara; cinco braseiros de bronze em círculo, um disco dourado sobre uma pedra, espirais vermelhas nas paredes. À primeira vista é um ALTAR DE RITUAL.\n\n"
          "SEGURANÇA: não há armadilha mecânica aqui, mas há DUAS SETEIRAS no alto da parede leste (11, +4, −14) e (11, +4, −22) por onde goblins vigiam o grupo. Percepção passiva 15 nota um brilho verde de olhos num breu de 0,6 × 0,5 m. Se o grupo gastar muito tempo na câmara, use a Emboscada [G] aqui: os braseiros se apagam, os olhos acendem e uma flecha de barro apaga a tocha.\n\n"
          "COMBATE (se houver): os goblins NÃO descem; atiram das seteiras com cobertura 3/4 e usam Nimble Escape para se esconder. Eles não querem lutar aqui; querem que o grupo SAIA ou entre no corredor onde o terreno é deles.\n\n"
          "SAÍDA: norte, pela passagem de 5 m que leva ao Corredor das Fendas (1.2)."), S, A11);
    w.addPoi(V(0, 0.2f, -15.f), PoiCategory::Lore, Q("O Falso Ritual (a 'prova' do padre)"),
        Q("O QUE É: cinco braseiros sagrados de bronze (roubados do templo da vila) num círculo de 3,5 m de raio em torno de uma pedra com o Relicário do Sol (o disco dourado do templo). As brasas estão fracas: ninguém ofereceu nada, ninguém cantou.\n\n"
          "COMO O PADRE LÊ ISTO: braseiros + relicário + espirais vermelhas + carvão empilhado = 'sacrifício ao calor da montanha'. Ele jura que os goblins estão SUGANDO o calor da vila. Tudo isto é a 'prova' dele.\n\n"
          "A VERDADE (Investigação CD 13): os braseiros são usados só para AQUECIMENTO. Em volta há peles de dormir (marcas de corpos pequenos enrolados), ossos de rato e restos de comida, tigelas de barro, pelos de rato esfolado perto das brasas. Não há símbolos arcanos, nem sacrifício, nem sangue em altar. Arcana CD 10: não há magia alguma, só calor comum. Religião CD 12: as espirais de ocre são pedidos de calor ao estilo dos povos da montanha, gestos de desespero, não um rito de drenagem.\n\n"
          "COMO USAR: deixe a mesa chegar à conclusão certa SOZINHA. O grupo que entende isto aqui chega à Toca já desconfiado da versão do padre. Quem aceita a versão do padre ataca de primeira e descobre depois (a anciã, os filhotes) o preço."), S, A11);
    w.addPoi(V(0, 0.6f, -17.f), PoiCategory::Treasure, Q("Relicário do Sol (disco dourado do templo)"),
        Q("Disco de ouro com 16 raios, ~60 cm de diâmetro, sobre uma haste de bronze apoiada numa pedra no centro do círculo de braseiros. É a peça mais sagrada do templo da vila (valor ≈ 250 po; vale muito mais para o padre). Foi levado junto com os braseiros.\n\n"
          "Não é mágico (Detectar Magia nada mostra). Pegar o disco é fácil e não dispara nada; o problema é ser visto: os goblins das seteiras vão achar que o grupo é ladrão e atirar.\n\n"
          "GANCHO: devolver o relicário ao padre sem dizer onde estava rende gratidão; devolvê-lo contando a verdade rende desconfiança. Reaparece na vila."), S, A11);
    w.addPoi(V(-6.f, -0.4f, -21.5f), PoiCategory::Lore, Q("Carvão, lenha e cobertores roubados"),
        Q("PILHAS DE ROUBO: carvão em (−6, chão, −22), lenha em (7, chão, −24) e cobertores em (−7, chão, −10): tudo da vila. Os cobertores têm o xadrez vermelho/verde/azul do mercado de lã, alguns com selo de tecelagem; dois estão estendidos como camas.\n\n"
          "LEITURA PARA OS JOGADORES: o carvão vem de casas de família (misturado a brasa de lareira), a lenha é cortada em toras de 70 cm (tamanho de braseiro de casa) e os cobertores são de gente comum. Quem roubou ia levar tudo para queimar e vestir, não para vender.\n\n"
          "MECÂNICA: carvão e lenha dão combustível para 8 horas de braseiro; um jogador que pegar um cobertor ganha vantagem em salvaguardas de Constituição contra o frio por 1 hora (a critério do Mestre)."), S, A11);
    w.addPoi(V(-9.6f, 1.2f, -13.5f), PoiCategory::Lore, Q("Espirais de ocre vermelho"),
        Q("Espirais pintadas com ocre e gordura nas paredes, em alturas diferentes: umas a 1,5 m (altura de goblin), outras mais altas (pintadas em pé sobre ombros). Alguns pares de mãos em ocre.\n\n"
          "SIGNIFICADO: é a tentativa goblin de 'chamar o calor', um costume trazido do inverno do norte. Religião/Sobrevivência CD 12 reconhece o sinal. As espirais estão repetidas nos cruzamentos do Labirinto (1.7) como marcas de guia, e na fronteira (1.8) viram arranhões de proteção.\n\n"
          "Não são magia. A tinta ainda está úmida nas mais baixas: pintadas esta semana, provavelmente por crianças."), S, A11);
    w.addPoi(V(10.3f, 3.8f, -18.f), PoiCategory::Trap, Q("Seteiras da antecâmara (parede leste)"),
        Q("DUAS FENDAS de 0,6 × 0,5 m a ~5 m do chão, em (11, +4, −14) e (11, +4, −22), ligadas ao túnel de rastejo leste (que corre atrás da parede, 1,0 m de altura, 0,9 m de largura). Não dá para um humano passar por elas.\n\n"
          "NOTAS: Percepção passiva 15 vê olhos verdes na fenda quando o Mestre aciona a Emboscada [G]. Atirador na fenda tem cobertura de 3/4 (+5 de CA) e usa Nimble Escape para se esconder como ação bônus. Arma de longo alcance ou magia de projétil acertam a fenda a 2 m de largura. Ataques corpo a corpo não alcançam.\n\n"
          "DICA TÁTICA: o túnel leste conecta as duas seteiras, o corredor e a Linha Goblin sobre o abismo; os atiradores se deslocam rastejando, rápidos, e reaparecem em qualquer seteira. Para eliminá-los, é preciso entrar no túnel pelo lado do abismo (Linha Goblin) ou fazê-los recuar."), S, A11);

    // =============================================================== 1.2 CORREDOR
    w.addPoi(V(0, -1.2f, -33.f), PoiCategory::Area, Q("1.2 Corredor das Fendas (Murder Holes)"),
        Q("O QUE É: corredor de 42 m de (0, −2, −28) a (0, −4, −70), 5 m de largura, 8 m de altura, chão quase plano descendo 2 m. As paredes têm bolsões e uma passagem lateral para a Cascata (oeste, z −40) e outra para o Sifão (leste, z −62).\n\n"
          "SETEIRAS: seis fendas (0,6 × 0,5 m) a 5 m do chão: oeste (x −2,5) em z −34, −46, −58; leste (x +2,5) em z −40, −52, −64. Cobertura 3/4 para atiradores.\n\n"
          "TÚNEIS DE RASTEJO: paralelos atrás das paredes (x ±4,2), piso na altura das seteiras, 1,0 m de altura × 0,9 m de largura. O leste liga a antecâmara à Linha Goblin; o oeste termina no Ninho de Atirador (−8, 0, −70).\n\n"
          "COMBATE TÍPICO: 6 a 8 goblins (Goblin: CA 15, PV 7, Nimble Escape) em rodízio nas fendas, flechas de barro (1d6 perfurante + apagam a tocha: CD 12 de Destreza para proteger a tocha). Quem corre pelo corredor cai nas armadilhas. Os personagens Médios só entram nos túneis rastejando (metade do deslocamento).\n\n"
          "ARMADILHAS: Fosso de Estacas (z −46..−49) e Fio de Tropeço (z −59). EVENTO [G]: emboscada."), S, A12);
    w.addPoi(V(-2.0f, 2.2f, -46.f), PoiCategory::Trap, Q("Seteiras (murder holes) do corredor"),
        Q("Fendas de 0,6 × 0,5 m, a 5 m do chão, dos dois lados do corredor, escalonadas (oeste z −34/−46/−58, leste z −40/−52/−64) para que ninguém tenha ponto cego. Por trás de cada uma há um goblin prostrado sobre o piso do túnel de rastejo.\n\n"
          "PERCEPÇÃO: CD 12 vê a fenda; CD 15 vê o brilho dos olhos verdes quando eles estão atentos. Quem tem Visão no Escuro [V] só nota a sombra da fenda.\n\n"
          "REGRAS: cobertura 3/4; Nimble Escape (ação bônus: Desengajar/Esconder); a flecha de barro cheia de água apaga a tocha desprotegida (Destreza CD 12 para proteger; escudo ou capuz dá vantagem). Um personagem de pequeno porte (Halfling/Gnomo) consegue se espremer por uma fenda se um goblin sair antes.\n\n"
          "COMO VENCER: contornar (túnel de rastejo, Cascata Congelada) ou apagar a visão do inimigo (fumaça, Escuridão, magia de luz cegando os olhos acostumados ao breu). Ataques sobre a fenda a 3/4 recebem desvantagem com Visão no Escuro sem luz."), S, A12);
    w.addPoi(V(4.2f, 2.4f, -46.f), PoiCategory::Area, Q("Túneis de rastejo goblin"),
        Q("Dois túneis paralelos ao corredor, atrás das paredes: oeste em x −4,2 e leste em x +4,2, de z −30 a −68, 1,0 m de altura × 0,9 m de largura; o piso fica à altura das seteiras (~4,8 m acima do chão do corredor).\n\n"
          "LIGAÇÕES: o leste vem das seteiras da antecâmara (x +13), corre ao longo do corredor e vira para o leste atrás da parede sul do abismo até a Linha Goblin em (24, −2, −70). O oeste começa numa alcova de dormir em (−4,6, 3,3, −29) e acaba no Ninho de Atirador (−8, 0, −70), voltado para o abismo e a ponte.\n\n"
          "ACESSO DOS JOGADORES: os túneis não têm entrada pelo corredor. O grupo entra por: (a) a Linha Goblin (Atletismo CD 15), (b) o Ninho (Escalada) ou (c) por uma abertura que o Mestre queira dar. Personagens Médios rastejam (metade do deslocamento, Desvantagem em ataques e CA contra ataques à distância). Visibilidade total: o túnel é um breu.\n\n"
          "Os goblins se movem pelos túneis ágeis e silenciosos (Furtividade +6). Use F9/raio-x para mostrar o esquema à mesa."), S, A12);
    w.addPoi(V(0, -2.0f, -47.5f), PoiCategory::Trap, Q("Fosso de Estacas Coberto"),
        Q("TRAP (goblin): z −46..−49, x −1,5..1,5, 3 m de profundidade, coberto por lona suja, terra e folhas secas. Deixa 1 m de borda livre de cada lado do corredor.\n\n"
          "DETECÇÃO: Percepção PASSIVA 14 nota a costura da lona e a terra solta; Investigação CD 12 (ou cutucar com uma vara/lança) revela o vazio. Um personagem que sonda o chão com um bastão de 3 m não cai.\n\n"
          "EFEITO: quem pisa na lona cai 3 m (1d6 contundente) e sobre estacas (2d6 perfurante). Destreza CD 12 para cair de lado e reduzir o dano pela metade. Escalar para sair: Atletismo CD 10 (as paredes ásperas aceitam [Espaço] com escalada). Os goblins só descem para saquear depois. Quem cai fica visível das seteiras: um alvo preso.\n\n"
          "NO PROGRAMA: o jogador que pisa na lona a faz cair (toast + tremor); uma zona de escalada permite sair do fosso. Use 'Rearmar Camada 1' para reconstruir a lona."), S, A12);
    w.addPoi(V(0, -3.0f, -59.f), PoiCategory::Trap, Q("Fio de Tropeço (potes de óleo e pedras)"),
        Q("TRAP (goblin): fio de tendão de rato na altura do tornozelo em z −59, preso a estacas de madeira nos dois lados. Ele puxa duas cordas ligadas a uma PRATELEIRA no teto (y +4) com potes de barro cheios de óleo de lamparina e pedras.\n\n"
          "DETECÇÃO: Percepção CD 13 (passiva 13 ainda nota o brilho do fio quando a tocha o toca de lado). Atenção: o fio é difícil de ver em escuridão sem luz direta.\n\n"
          "EFEITO: Destreza CD 13: falha = 2d10 de dano contundente (pedras + potes) e fica COBERTO DE ÓLEO (vulnerável a dano de fogo até se limpar, ação inteira com pano); sucesso = metade do dano, sem óleo nas roupas. O barulho alerta os goblins: eles atiram flechas (a emboscada fica mais fácil de acionar com [G]). Nada mais é soltável.\n\n"
          "CONTRAMEDIDA: cortar o fio com uma lâmina, pular por cima (Acrobacia CD 10), ou desarmar subindo pelo túnel de rastejo. Oleado + tocha = Mestre decide se o grupo pega fogo (1d6 por rodada, extingue com ação)."), S, A12);
    w.addPoi(V(-7.8f, 0.9f, -69.2f), PoiCategory::Creature, Q("Ninho de atirador (−8, 0, −70)"),
        Q("Bolsão onde termina o túnel de rastejo oeste, 4 m acima do chão da cabeceira da ponte, com uma janela de 1,2 × 1,0 m voltada para o abismo e para a ponte. Aqui um goblin com arco curto espera.\n\n"
          "USO TÁTICO: o atirador acerta qualquer pessoa na ponte (alcance 15–25 m, cobertura 3/4 pela borda). É o gatilho humano da ruptura: quando alguém pesado chega ao meio, o atirador grita e o Mestre corta as cordas [B]. Acessível pela Linha Goblin, pelo tronco-estalactite (a partir da fenda) ou por escalada (Atletismo CD 15, parede úmida).\n\n"
          "Se for atacado, o goblin foge rastejando para o leste; Nimble Escape."), S, A12);

    // =============================================================== 1.3 ABISMO
    w.addPoi(V(0, -2.8f, -72.f), PoiCategory::Area, Q("1.3 O Abismo do Rio Violento"),
        Q("O QUE É: abismo leste–oeste de x −32 a 28, z −70..−84, do chão (y −4) até o leito (y −21). Um rio subterrâneo violento corre a y −18,5 de oeste para leste e some num sumidouro em x 28. Tudo cheira a água fria; o rugido sobe das paredes.\n\n"
          "PASSAGEM: a Ponte Podre no meio (x 0). Alternativas: o tronco-estalactite (x −20, rota secreta da Cascata) e a Linha Goblin (x +24, só goblins e acrobatas). Uma trilha de cabras (rampa estreita, junto à parede norte) permite voltar do leito para a cabeceira norte.\n\n"
          "PERIGOS: queda de 16 m (5d6 de dano contundente) na água gelada de 2 m de profundidade, com ESTACAS no leito raso sob a ponte (x −4..4, z −73..−81). Corrente: Atletismo CD 14 por rodada ou é arrastado ao sumidouro em x 28 (3 rodadas para chegar; zona de perigo: respawn).\n\n"
          "Temperatura −6 °C; Constituição CD 12 por minuto na água ou 1 nível de exaustão."), S, A13);
    w.addPoi(V(0, -3.0f, -77.f), PoiCategory::Trap, Q("A Ponte Podre"),
        Q("TRAP + EVENTO: ponte de corda de (0, −4, −70) a (0, −4, −84), 1,5 m de largura, 20 tábuas (algumas tortas, outras faltando), 2 postes de cada lado e corrimões de corda. Cede 0,6 m no meio. Cada tábua aguenta 1 criatura Média por vez.\n\n"
          "GATILHO: quando alguém de ARMADURA PESADA chega ao meio da ponte, os goblins cortam as cordas (o atirador do Ninho e o da alcova leste). No programa: o corte é AUTOMÁTICO quando o jogador chega ao meio (uma vez) ou pelo evento [B] 'Cortar as cordas da Ponte Podre'.\n\n"
          "EFEITO: Destreza CD 15 (reação) para agarrar a corda ou uma tábua; sucesso = pendurado, escalada de 1 rodada até a cabeceira mais próxima; falha = queda de 16 m (5d6) na água gelada (estacas sob a ponte: +2d6 perfurante se cair em x −4..4) e Atletismo CD 14 para não ser arrastado ao sumidouro.\n\n"
          "SEM ARMADURA PESADA: a ponte aguenta. Se o grupo atravessa um de cada vez e devagar, o Mestre pode avisar a Atletismo/Acrobacia (cada tábua faltando é CD 10).\n\n"
          "DEPOIS: o grupo precisa da Linha Goblin, do tronco-estalactite (Cascata) ou da trilha de cabras no leito para chegar à Toca."), S, A13);
    w.addPoi(V(0, -18.0f, -77.f), PoiCategory::Trap, Q("Estacas do leito raso"),
        Q("Sob a ponte (x −4..4, z −73..−81) o leito sobe a um banco raso (cerca de 0,7 m de água) cheio de ESTACAS de madeira fincadas, de 2 a 3 m de altura, algumas inclinadas. Ossos e um crânio revelam quem já caiu aqui.\n\n"
          "REGRA: quem cai sobre as estacas (x −4..4) sofre 2d6 de dano perfurante adicional (Destreza CD 13: metade) além dos 5d6 da queda; o banco raso impede nadar (profundidade < 1,35 m), então a pessoa ainda precisa ir para a água funda para a correnteza ou escalar pelas estacas.\n\n"
          "Os goblins colocaram as estacas depois da primeira vez que um viajante pesado atravessou a ponte e sobreviveu."), S, A13);
    w.addPoi(V(22.f, -17.9f, -77.f), PoiCategory::Trap, Q("Correnteza e sumidouro"),
        Q("O rio corre para o leste e desaparece num buraco de 4,5 m na base da parede leste (x 28). Quem entra na água é empurrado ~1,7 m/s; o rugido encobre outros sons.\n\n"
          "REGRAS: Atletismo CD 14 a cada rodada para nadar contra/oblíquo e agarrar uma pedra do leito; falhar 3 rodadas seguidas = arrastado para o sumidouro (5d6 + preso numa galeria inundada; no programa, o jogador volta ao último chão seguro). Armadura pesada: desvantagem. Tocha apaga.\n\n"
          "Quem cai da ponte tem ~12 m de água para alcançar a trilha de cabras (extremo oeste do leito) ou as estacas (banco raso) antes do sumidouro. Boa cena de tensão."), S, A13);
    w.addPoi(V(24.f, -1.8f, -78.f), PoiCategory::Area, Q("Linha Goblin (corda sobre o abismo)"),
        Q("Uma corda única, esticada em x +24 entre a saída do túnel de rastejo leste (24, −2, −70) e uma alcova (24, −3, −86) ligada à Toca. Goblins (Pequenos) atravessam pendurados em segundos.\n\n"
          "REGRAS para Médios: Atletismo CD 15 (cada metade do caminho exige um teste; falha = queda de 16 m com correnteza); com um goblin disparando contra você, desvantagem. Quem passa pela corda chega ao túnel leste: rota de flanco para os atiradores.\n\n"
          "DICA: cortar a corda (Atacar CA 12, PV 3) fecha o acesso dos goblins entre os dois lados: eles ficam isolados do túnel leste."), S, A13);
    w.addPoi(V(-18.f, -10.8f, -82.4f), PoiCategory::Area, Q("Trilha de cabras (rampa do leito)"),
        Q("Rampa estreita (2,7 m) colada à parede norte, de 27° a 30°, do extremo oeste do leito até a cabeceira norte da ponte. Os goblins a usam para descer ao rio e recuperar o que cai. Um grupo que cai da ponte tem um caminho de volta aqui (nade para oeste, ~25 m, depois suba).\n\n"
          "Pedras soltas: Acrobacia CD 10 para correr; sem tocha, é perigosa (CD 12). Os arqueiros da galeria alta da Toca veem esta rampa."), S, A13);

    // =============================================================== 1.5 CASCATA
    w.addPoi(V(-4.5f, -1.2f, -40.f), PoiCategory::Secret, Q("Abertura lateral (passagem secreta)"),
        Q("Na parede oeste do corredor em z −40 há uma abertura de 3 m que parece só uma reentrância irregular na rocha, mas leva à Câmara da Cascata (−12, −3, −40). Percepção CD 13 nota corrente de ar frio e sinais de passagem (a poeira está limpa).\n\n"
          "É uma rota secreta que os goblins não usam (eles acham que é maldita): é a passagem 'esquecida' pela qual o grupo flanqueia as seteiras. Mas quem vai e vem ainda passa na frente de uma seteira (z −40 leste, −34 oeste)."), S, A15);
    w.addPoi(V(-12.f, -1.5f, -39.f), PoiCategory::Rest, Q("Câmara da Cascata (descanso curto possível)"),
        Q("Câmara de 10 × 8 m, 6 m de altura, calcária, úmida, fria (−14 °C). Sem tráfego goblin. Estalactites no teto, estalagmites no chão. O gelo da cascata no fundo cobre a única saída além da abertura.\n\n"
          "DESCANSO: bom lugar para descanso curto (1 hora): defensável (uma entrada de 3 m), sem corrente de ar, sem barulho. Descanso longo: expõe o grupo a uma patrulha goblin ocasional pela abertura (a critério do Mestre). Não acenda fogo: a fumaça atrai atenção pelo corredor."), S, A15);
    w.addPoi(V(-12.f, -1.5f, -43.4f), PoiCategory::Secret, Q("A Cascata Congelada"),
        Q("PASSAGEM SECRETA: lâmina de gelo translúcido de 4 m × 6,6 m na parede norte da câmara, em (−12, −3, −44,5), que preenche um nicho. Atrás dela corre uma FENDA natural.\n\n"
          "DETECÇÃO: Percepção CD 15; com a luz de uma tocha ENCOSTADA no gelo vê-se a sombra da fenda (vantagem). Arcana CD 12: o gelo não é mágico, é de cascata perene congelada.\n\n"
          "COMO PASSAR: (1) QUEBRAR o gelo [E] (barulho: os goblins são alertados; ataque CA 11, PV 25, vulnerável a fogo e contundente); (2) com fogo, derreter devagar sem ruído (≈ 10 min com uma tocha); (3) o Mestre pode permitir passar pelo gelo se ele for fino em algum ponto. No programa, [E] quebra o gelo e soa o alarme.\n\n"
          "ROTA DE FLANCO: a fenda leva à borda sul do abismo (−20, −4, −70) e, de lá, ao tronco-estalactite até a Toca, evitando a ponte e as seteiras."), S, A15);
    w.addPoi(V(-13.9f, -2.1f, -51.2f), PoiCategory::Secret, Q("A fenda natural"),
        Q("Fenda de 1,0 m de largura × 2,2 m de altura que serpenteia ~26 m de (−12, −3, −46) até (−20, −4, −70) por trás do gelo. Fria, úmida, com dobras de pedra polida pela água. Pessoas Médias passam de lado com mochila cheia (Acrobacia CD 10 para não prender o equipamento).\n\n"
          "Sem iluminação: breu absoluto. A fenda passa ao lado do corredor, a 8–15 m das seteiras, sem ser vista. Eco: sons chegam ao corredor, então o grupo precisa de cuidado (Furtividade CD 12)."), S, A15);
    w.addPoi(V(-20.f, -3.4f, -77.f), PoiCategory::Secret, Q("A Estalactite-Tronco"),
        Q("Estalactite caída que atravessa o abismo como um tronco (raio ≈ 0,6 m) de (−20, −4, −70) a (−20, −5, −84), ligando a fenda à Toca (lobo oeste, por baixo da galeria dos arqueiros).\n\n"
          "REGRAS: Acrobacia CD 12 para atravessar (14 m); falha = queda de 16 m (5d6) na água. Quem atravessa se arrastando (metade do deslocamento) tem +3 no teste. A estalactite é lisa e úmida: com tocha na mão, desvantagem. Dá cobertura contra o ninho de atirador.\n\n"
          "Do lado norte, o lobo oeste da Toca fica DIRETAMENTE abaixo da galeria dos arqueiros: dá para surpreender os goblins por baixo."), S, A15);

    // =============================================================== 1.6 SIFAO
    w.addPoi(V(25.f, -3.8f, -62.f), PoiCategory::Secret, Q("Abertura leste do corredor (rumo ao Sifão)"),
        Q("Na parede leste em z −62 há uma abertura de 3 m que leva a um túnel úmido de 25 m até um poço de água turva em (30, −5, −62), de 6 × 6 m, com a superfície em y −5,5. O ar cheira a lama e minerais.\n\n"
          "O poço é a entrada do SIFÃO TURVO: o único jeito de usar o túnel é mergulhar. Percepção CD 14 nota que a água não está parada (tem corrente suave) e que sai uma bolha de ar contínua do fundo."), S, A16);
    w.addPoi(V(36.f, -9.f, -78.f), PoiCategory::Trap, Q("O Sifão Turvo (túnel submerso, ~28 m)"),
        Q("TÚNEL TOTALMENTE SUBMERSO: de (30, −5, −62) desce a (36, −9, −66), segue ao norte por ~24 m a y −9 e sobe a (30, −5,5, −94), emergindo num poço na alcova leste da Toca. Largura ~3 m, sem bolsões de ar.\n\n"
          "REGRAS: prender a respiração por (1 + mod. CON) minutos (mínimo 30 s); a nado, deslocamento de 6 m/rodada (~28 m = ~5 rodadas com sobra de 1 min; 50% a mais se carregar equipamento pesado). Água a 2 °C: Constituição CD 12 ou 1 nível de exaustão. Armadura média/pesada: nada de nado (Atletismo CD 14 por 10 m ou afunda). Tocha apaga na entrada.\n\n"
          "NO PROGRAMA: zona de água com barra de fôlego; [Espaço] sobe, [Ctrl] desce. A tinta submersa e a visibilidade ruim tornam fácil se perder: o tubo é contínuo, basta seguir a parede da direita.\n\n"
          "TÁTICO: sai pelas costas do acampamento; o grupo emerge a ~12 m da galeria dos arqueiros e da tenda de Grukk: emboscada pelo flanco. Os goblins não usam o sifão: acham que é a boca do que vive embaixo."), S, A16);
    w.addPoi(V(30.f, -4.5f, -94.f), PoiCategory::Area, Q("Poço de saída do Sifão (Toca)"),
        Q("Poço de 6 × 6 m na alcova leste da Toca, superfície em y −5,5, ligado à Toca por um túnel curto (de (18, −5, −100) a (26, −5,3, −96)). Quem emerge aqui está a ~8 m do braseiro central, atrás das tendas leste.\n\n"
          "Sair da água fria: Constituição CD 12 se não houve descanso depois do mergulho. Roupas ensopadas a −9 °C: 1 nível de exaustão em 10 min sem se secar."), S, A16);

    // =============================================================== 1.4 A TOCA
    w.addPoi(V(0, -3.4f, -90.f), PoiCategory::Area, Q("1.4 A Toca (acampamento goblin)"),
        Q("O QUE É: câmara de x −18..18 (lobo oeste até −24), z −86..−112, chão a y −5, teto a 7 m, quente (−9 °C) por causa do braseiro central roubado. Seis tendas feitas de COBERTORES ROUBADOS da vila, peles de dormir, ossos de rato, restos de comida.\n\n"
          "ENTRADAS: cabeceira norte da ponte (0, −4, −84), tronco-estalactite (lobo oeste), Linha Goblin (alcova NE), túnel leste (para o sifão) e a saída norte (0, −5, −112) para o Labirinto.\n\n"
          "OCUPANTES: ~10 goblins magros (CR 1/4: CA 15, PV 7, Nimble Escape, arco curto), 3 arqueiros na galeria alta oeste, 3 filhotes, uma anciã e GRUKK Dente-Quebrado (goblin boss, CA 17, PV 21, Reação: redireciona ataques para um goblin adjacente). Mesa de ~4 PCs de nível 6: encontro difícil, mas a Toca é mais cena do que combate.\n\n"
          "TOM: nota de horror. Depois da luta, o silêncio. Os filhotes e a anciã não lutam: só tremem. É o momento de a mesa perceber que os goblins não são os vilões. Uma alternativa a combate: negociar com Grukk (Persuasão CD 14 se o grupo trouxer o cobertor/carvão, ou se entregar comida)."), S, A14);
    w.addPoi(V(-3.f, -4.2f, -98.f), PoiCategory::Creature, Q("Goblins magros (o bando de Grukk)"),
        Q("~10 adultos encolhidos junto ao braseiro e nas tendas, olhos amarelo-esverdeados, costelas aparentes, trapos de lã roubados. Magros demais: PV 5 em vez de 7 (desnutrição), Nimble Escape, Furtividade +6. Arcos curtos com flechas de barro cheias d'água (apagam tochas) e algumas flechas comuns.\n\n"
          "COMO LUTAM: como ratos encurralados. Atiram do alto, recuam, tentam apagar as luzes antes de atacar (Emboscada [G]: os braseiros se apagam, os olhos verdes acendem). Em corpo a corpo, adagas e facas de osso.\n\n"
          "MORAL: se Grukk cair, a maioria se rende (CD 10 de Persuasão/Intimidação para manter a rendição). Eles falam um dialeto goblin da montanha, com algumas palavras comuns. Perguntas úteis: 'Quem está embaixo?' (eles tremem), 'Por que o carvão?' ('frio, frio, os pequenos')."), S, A14);
    w.addPoi(V(0, -4.2f, -97.4f), PoiCategory::Creature, Q("Filhotes e a anciã (não lutam)"),
        Q("Três filhotes goblin (altura 60 cm) e uma ANCIÃ de xale cinza junto ao braseiro central, tentando se aquecer entre dois cobertores. NÃO atacam. Os filhotes se encolhem; a anciã segura a mão deles e murmura uma canção num dialeto antigo.\n\n"
          "CENA: matar um filhote é um ato que a mesa vai lembrar. Se o grupo ataca de primeira, o Mestre deve deixar a cena respirar depois: silêncio, o fogo estalando, os filhotes olhando. Um cobertor entregue com respeito muda o tom de toda a negociação.\n\n"
          "INFORMAÇÃO: a anciã (Persuasão CD 12, ou comida) conta, em goblin quebrado, que 'os grandes desceram há três luas para dentro do fundo; voltaram sem rosto' (Kaelen e os sequestros), que 'o frio vem de baixo' e que eles roubaram da vila só para sobreviver."), S, A14);
    w.addPoi(V(0, -3.2f, -102.8f), PoiCategory::Creature, Q("Grukk Dente-Quebrado (goblin boss)"),
        Q("GRUKK: goblin boss esquelético de 1,3 m, trono de caixotes empilhados, dentes quebrados num sorriso torto, cabelo de rato e colar de ossos. CA 17, PV 21 (desnutrido: 18), Nimble Escape, Redirect Attack (reação: troca de lugar com um goblin adjacente e redireciona o golpe). Fala comum de forma tosca.\n\n"
          "PERSONALIDADE: o último que acredita que a Toca ainda se salva. Protege o cálice dourado do templo como símbolo: 'quem tem o dourado tem o fogo'. Pode ser convencido a trocá-lo por comida/carvão se o grupo mostrar boa-fé. Se atacado: luta até cair, depois suplica pelos filhotes.\n\n"
          "TÁTICA: grita para a galeria atirar, joga um pote de óleo no braseiro (fogo inesperado, 2d6 em área de 1,5 m se alguém estiver adjacente), foge pelo túnel norte rumo ao Labirinto onde conhece cada curva. O Esconderijo do Chefe fica no Lixão (1.7)."), S, A14);
    w.addPoi(V(0, -4.4f, -100.f), PoiCategory::Treasure, Q("Cálice dourado do templo"),
        Q("Cálice de ouro (≈ 200 po) com uma gema vermelha incrustada, sobre uma pedra baixa diante do trono de Grukk em (0, −5, −100). É o cálice de comunhão do templo da vila, roubado junto com os braseiros.\n\n"
          "Não é mágico. O padre o reconhece de longe; devolvê-lo à vila é uma recompensa de história (Persuasão/Religião +2 com o padre e a vila). Para Grukk, entregá-lo significa renunciar ao seu último símbolo de poder: dê ao grupo a chance de pedir em vez de tomar."), S, A14);
    w.addPoi(V(-15.f, 0.5f, -98.f), PoiCategory::Area, Q("Galeria alta oeste (arqueiros)"),
        Q("Plataforma de rocha em x −18..−12, 4 m acima do chão (y −1), z −88..−108, com acesso por uma rampa de rochas caídas (x −5,5 → −12,5, z −96). Três goblins arqueiros com feixes de flechas de barro, potes de água e peles.\n\n"
          "TÁTICA: cobertura parcial (borda de 0,5 m), vantagem de altura (+2 no ataque sem pena para o alcance). Para eliminá-los: subir pela rampa (3 m de largura, terreno difícil), chegar por baixo (tronco-estalactite) ou usar escuridão/fumaça. Eles apagam as luzes antes de atacar (Emboscada [G]); olhos verdes na galeria.\n\n"
          "Se o grupo vence o chefe antes, os arqueiros se rendem."), S, A14);

    // =============================================================== 1.7 LABIRINTO
    w.addPoi(V(0, -4.4f, -118.f), PoiCategory::Area, Q("1.7 O Labirinto dos Leitos Secos"),
        Q("O QUE É: de z −112 a −160, corredores sinuosos de 3 a 4 m de largura e 3,5 m de altura, com o chão descendo de −6 a −13. Leitos de rios subterrâneos que secaram: seixos polidos, paredes lisas, curvas amplas. Espirais de ocre marcam os cruzamentos; arranhões em X marcam os becos sem saída.\n\n"
          "NÓS: N0 (0, −5, −112) · N1 (0, −6, −120) · N2 (−15, −7, −126) · N3 (−26, −7, −132) beco O Lixão · N4 (12, −7, −127) · N5 (22, −8, −122) beco A Despensa Congelada · N6 (−6, −8, −136) · N7 (−16, −9, −146) beco Galeria Desabada · N8 (6, −8, −140) · N9 (12, −10, −150) · N10 (18, −13, −157).\n"
          "LIGAÇÕES: N0–N1, N1–N2, N2–N3, N1–N4, N4–N5, N2–N6, N6–N7, N6–N8, N4–N8, N8–N9, N9–N10.\n\n"
          "CAMINHO CERTO: N0 → N1 → N4 → N8 → N9 → N10 (as espirais apontam a direção dos goblins; os becos têm X de aviso). Sem mapa, cada nó pede Sobrevivência CD 12 (ou os arranhões) para não se perder; 3 falhas = 1 hora perdida (a tocha continua queimando).\n\n"
          "AMBIENTE: sem goblins. Eles evitam o Labirinto: 'onde os grandes desceram'. Silêncio, eco e pingos. É a primeira cena sem inimigos humanoides."), S, A17);
    w.addPoi(V(-26.f, -5.8f, -132.f), PoiCategory::Area, Q("O Lixão (beco N3)"),
        Q("Beco de ~10 × 9 m onde os goblins despejam o lixo: ossos de rato e cabra, trapos, potes quebrados, restos de comida podre, barris rachados. Cheiro intenso (Constituição CD 10 ou náusea 1 min, enjoo = desvantagem).\n\n"
          "O Mestre pode fazer o grupo perder tempo aqui; o lixo parece nada, mas esconde um SEGREDO: atrás de um monte de pedras soltas, a oeste, fica o Esconderijo do Chefe (Investigação CD 14).\n\n"
          "Lixo útil: Medicina CD 12 identifica ossos de criaturas maiores (goblins mortos de frio e de doença); Investigação CD 10 acha 5 pc e uma faca de osso."), S, A17);
    w.addPoi(V(-34.f, -5.9f, -136.5f), PoiCategory::Treasure, Q("Esconderijo do Chefe (secreto)"),
        Q("SEGREDO: nicho de 6 × 5 m atrás de pedras soltas no Lixão, a oeste, em (−34, −7, −136). Descoberta: Investigação CD 14 (ou [E] diante das pedras). Dentro: um baú aberto com 120 po em moedas do templo, um candelabro de ouro, um incensário de bronze, um relicário menor e um cobertor dobrado.\n\n"
          "É o que sobrou dos tesouros do templo: Grukk guarda a parte mais valiosa para o caso de a Toca cair: 'se os grandes voltarem, trocamos pelo calor'. Valor total ≈ 120 po + peças (≈ 400 po de relíquias), que o padre reconhecerá.\n\n"
          "No programa: [E] sobre as pedras abre o esconderijo e dá o item ao inventário."), S, A17);
    w.addPoi(V(22.f, -6.4f, -122.f), PoiCategory::Lore, Q("A Despensa Congelada (beco N5)"),
        Q("Beco frio (−18 °C) onde os goblins guardam comida: carcaças congeladas de 3 cabras da vila (sequestradas) e um URSO, penduradas em ganchos e cobertas de gelo; pingentes no teto.\n\n"
          "LEITURA: o urso não foi caçado por goblins: está com marcas de garras GRANDES e de um golpe que partiu o osso. Medicina/Sobrevivência CD 13: algo mais forte que eles o matou e deixou a carcaça. Ninguém no Labirinto foi visto de perto, mas os goblins dizem 'os grandes' (Kaelen e seus lobos). É uma dica discreta do que vive embaixo.\n\n"
          "UTILIDADE: comida para o grupo (3 rações cada carcaça, a carne está dura mas comível). Se o grupo trouxer para a Toca, vira moeda de negociação."), S, A17);
    w.addPoi(V(-16.f, -7.5f, -146.f), PoiCategory::Area, Q("A Galeria Desabada (beco N7)"),
        Q("Antiga galeria que desabou: o teto caiu e soterrou a passagem ao norte. Poeira constante no ar, vigas quebradas, uma mão verde sob as pedras (um goblin que morreu aqui tentando abrir caminho).\n\n"
          "ARMADILHA NATURAL: cada ruído alto (barulho de combate, gritos) pode derrubar mais pedras: Destreza CD 12 ou 2d6 contundente numa área de 3 m. Cavar o desabamento leva horas e dá no mesmo ponto onde o rio secou. Beco sem saída: a rota de fuga é de volta por N6.\n\n"
          "LORE: os goblins tentaram escavar por aqui para escapar do frio de fora, sem sucesso."), S, A17);
    w.addPoi(V(0, -4.9f, -110.5f), PoiCategory::Area, Q("Saída norte da Toca (N0)"),
        Q("Túnel de 4 m que sai da parede norte da Toca em (0, −5, −112) e desce suavemente ao Labirinto. Entrada defensável: um goblin com arco a bloqueia por 10 minutos; os fugitivos usam esta saída quando a Toca cai."), S, A17);

    // =============================================================== 1.8 FRONTEIRA
    w.addPoi(V(20.5f, -13.5f, -158.5f), PoiCategory::Area, Q("1.8 A Fronteira dos Ossos"),
        Q("O QUE É: túnel de N10 (18, −13, −157) → (22, −16, −162) → borda do poço (27, −19, −167). Totens de ossos, fetiches pendurados no teto, arranhões de proteção nas paredes e uma linha de cinzas e sal no chão em (21, chão, −160). É o último vestígio goblin.\n\n"
          "ATMOSFERA: a partir daqui, os outros 98% da masmorra: não há lixo nem fogueiras, só escuridão, frio e um ZUMBIDO PSÍQUICO baixo. Temperatura −20 °C. A tocha brilha menos (Mestre: desvantagem em Furtividade, a mesa deve sentir a pressão).\n\n"
          "MENSAGEM AOS JOGADORES: os goblins têm pavor do que vive lá embaixo. Os totens são avisos e oferendas; os arranhões, proteção; a linha de sal e cinza, uma barreira simbólica. Aqui os goblins param: não passam desta linha."), S, A18);
    w.addPoi(V(21.f, -14.2f, -160.f), PoiCategory::Lore, Q("Linha de cinzas e sal"),
        Q("Linha de cinza branca e sal grosso, com cerca de 15 cm de largura, atravessando o túnel de lado a lado em (21, chão, −160). Uma faixa de cinza escura corre ao lado.\n\n"
          "SIGNIFICADO: 'daqui para baixo não somos mais'. Religião/Sobrevivência CD 12: costume goblin de marcar o limite do 'território dos grandes'. Arcana CD 14: é apenas cinza e sal, sem poder. Atravessá-la não faz nada mecânico; os goblins que seguem o grupo param exatamente aqui e não passam, mesmo que perdendo a presa.\n\n"
          "Ótima deixa para o Mestre dizer: 'os goblins te observam do outro lado da linha, em silêncio, com medo' antes de o grupo descer."), S, A18);
    w.addPoi(V(19.f, -12.5f, -158.5f), PoiCategory::Lore, Q("Totens de ossos e fetiches pendurados"),
        Q("Totens de ossos (2 m a 2,5 m, crânios de rato, cabra e goblin em varas) e fetiches pendurados no teto: crânios, ossos cruzados em anel, penas negras. Quanto mais fundo, mais fetiches; todos voltados para o poço.\n\n"
          "LORE: são proteção contra o que vive embaixo. Um dos crânios pequenos é de CRIANÇA goblin: um filhote cuja mãe ainda reza por ele. Religião CD 12 reconhece fórmulas de proteção e luto. Nada aqui é mágico, mas o conjunto tem um peso emocional enorme.\n\n"
          "Arranhões em grupos de três nas paredes: 'o que ouve não deve ouvir de volta' (um pedido de silêncio)."), S, A18);
    w.addPoi(V(27.f, -17.6f, -167.f), PoiCategory::Area, Q("Borda do Poço (fim da Camada 1)"),
        Q("O túnel chega ao rebordo do POÇO DO ECO em (27, −19, −167), a cerca de 3 m da parede do abismo cilíndrico (centro (40, ·, −180), raio 22). Daqui começa a Camada 2.\n\n"
          "SENSAÇÃO: uma corrente de ar frio sobe do vazio, o zumbido psíquico aumenta e o eco da respiração do grupo vem de longe. Sem luz, nada se vê além da borda. O Mestre deve fazer uma pausa antes de descer: descanso curto aqui é razoável (os goblins não vêm até aqui)."), S, A18);

    // =============================================================== PONTOS DE VISTA
    const QString G = Q("Camada 1 · A Garganta Erodida");
    auto vp = [&](const QString& n, Vec3 p, float yaw, float pitch, bool fly = false) { w.addViewpoint(n, p, yaw, pitch, fly, G); };
    // 1.1
    vp(Q("1.1 Antecâmara do Falso Ritual (entrada)"), V(0, 1.5f, -2.5f), 0.f, -4.f);
    vp(Q("1.1 O Falso Ritual (braseiros e relicário)"), V(0, 0.9f, -10.5f), 0.f, -4.f);
    vp(Q("1.1 Seteiras da parede leste"), V(-3.f, 0.3f, -18.f), -90.f, 22.f);
    vp(Q("1.1 Carvão, lenha e cobertores"), V(-3.f, 0.1f, -12.f), 55.f, -6.f);
    // 1.2
    vp(Q("1.2 Corredor das Fendas (seteiras)"), V(0, -0.4f, -29.f), 0.f, 8.f);
    vp(Q("1.2 Fosso de Estacas Coberto"), V(0, -1.1f, -43.f), 0.f, -16.f);
    vp(Q("1.2 Fio de Tropeço e prateleira"), V(0, -1.9f, -54.f), 0.f, 12.f);
    vp(Q("1.2 Túnel de rastejo leste (voo)"), V(4.2f, 3.0f, -33.f), 0.f, -3.f, true);
    vp(Q("1.2 Corredor visto do alto (voo)"), V(0, 5.5f, -31.f), 0.f, -22.f, true);
    // 1.3
    vp(Q("1.3 Ponte Podre (cabeceira sul)"), V(0, -2.35f, -69.2f), 0.f, -8.f);
    vp(Q("1.3 O abismo visto do oeste (voo)"), V(-29.f, -5.f, -77.f), -90.f, -4.f, true);
    vp(Q("1.3 Ponte, rio e estacas vistos de cima (voo)"), V(-8.f, 3.f, -71.f), -53.f, -35.f, true);
    vp(Q("1.3 Linha Goblin e sumidouro (voo)"), V(24.f, -1.f, -73.f), 0.f, -6.f, true);
    // 1.4
    vp(Q("1.4 A Toca (entrada pela ponte)"), V(0, -3.25f, -87.5f), 0.f, -3.f);
    vp(Q("1.4 O braseiro, os filhotes e Grukk"), V(0, -3.4f, -93.5f), 0.f, -6.f);
    vp(Q("1.4 Galeria dos arqueiros"), V(-15.f, 0.7f, -90.f), 0.f, -2.f);
    vp(Q("1.4 A Toca vista do alto (voo)"), V(10.f, 1.f, -109.f), 140.f, -14.f, true);
    // 1.5
    vp(Q("1.5 Câmara da Cascata Congelada"), V(-12.f, -1.35f, -38.5f), 0.f, 6.f);
    vp(Q("1.5 Fenda e tronco-estalactite (voo)"), V(-20.f, -2.f, -69.f), 0.f, -14.f, true);
    // 1.6
    vp(Q("1.6 Poço do Sifão (entrada)"), V(25.2f, -3.5f, -62.f), -90.f, -24.f);
    vp(Q("1.6 Dentro do Sifão (submerso, voo)"), V(36.f, -9.f, -72.f), 0.f, 0.f, true);
    // 1.7
    vp(Q("1.7 Labirinto: N1"), V(0, -4.35f, -118.5f), 0.f, -2.f);
    vp(Q("1.7 O Lixão e as pedras soltas"), V(-25.f, -5.3f, -130.5f), 62.f, -5.f);
    vp(Q("1.7 Despensa Congelada"), V(16.f, -6.3f, -123.f), -90.f, 0.f);
    vp(Q("1.7 Galeria Desabada"), V(-15.5f, -7.4f, -141.5f), 0.f, -2.f);
    vp(Q("1.7 Labirinto: nó N8 (voo)"), V(6.f, -5.8f, -139.f), 0.f, -20.f, true);
    // 1.8
    vp(Q("1.8 Fronteira dos Ossos (totens)"), V(18.2f, -11.5f, -157.2f), -38.f, -6.f);
    vp(Q("1.8 Linha de cinzas e sal"), V(19.2f, -12.4f, -157.4f), -38.f, -27.f);
    vp(Q("1.8 Borda do Poço"), V(25.6f, -16.9f, -165.6f), -45.f, -8.f);
}

} // namespace L1
