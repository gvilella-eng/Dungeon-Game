import QtQuick 2.15 as QQ2
import Qt3D.Core 2.15
import Qt3D.Render 2.15
import Qt3D.Input 2.15
import Qt3D.Extras 2.15

Entity {
    id: sceneRoot

    // =========================================================================
    // ESTADOS INTERATIVOS DO MESTRE (Controlados pelo Teclado)
    // =========================================================================
    property bool dmFullbrightMode: true       // [T] Alterna Visão do Mestre vs Escuridão Shadowdark
    property bool goblinBraziersLit: true      // [G] Goblins apagam as luzes com flechas de barro/água
    property bool geodePuzzleSolved: true      // [P] Ativa/Desativa os feixes de refração nos cristais
    property real dragonPulse: 1.0             // Animação contínua do Coração do Dragão

    QQ2.NumberAnimation on dragonPulse {
        from: 0.85
        to: 1.18
        duration: 1800
        loops: QQ2.Animation.Infinite
        easing.type: QQ2.Easing.InOutSine
    }

    // =========================================================================
    // CÂMERA, RENDERIZADOR E CONTROLES DE NAVEGAÇÃO
    // =========================================================================
    Camera {
        id: mainCamera
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 50
        nearPlane: 0.5
        farPlane: 1200.0
        position: Qt.vector3d(0.0, 20.0, 290.0)
        viewCenter: Qt.vector3d(0.0, -5.0, 0.0)
        upVector: Qt.vector3d(0.0, 1.0, 0.0)
    }

    OrbitCameraController {
        camera: mainCamera
        linearSpeed: 85.0
        lookSpeed: 240.0
    }

    components: [
        RenderSettings {
            activeFrameGraph: ForwardRenderer {
                camera: mainCamera
                clearColor: sceneRoot.dmFullbrightMode ? "#0d111a" : "#010103"
            }
        },
        InputSettings {}
    ]

    // Atalhos de Teclado para saltar entre as Camadas e testar Mecânicas
    KeyboardDevice { id: kbDevice }

    KeyboardHandler {
        sourceDevice: kbDevice
        focus: true
        onPressed: function(event) {
            if (event.key === Qt.Key_1) {
                // [1] Visão Geral (Corte Transversal de todas as camadas)
                mainCamera.position = Qt.vector3d(0, 15, 290)
                mainCamera.viewCenter = Qt.vector3d(0, -10, 0)
            } else if (event.key === Qt.Key_2) {
                // [2] Entrada e Garganta Goblin (Ponte Podre, Fosso, Fendas Sniper, Cascata Secreta)
                mainCamera.position = Qt.vector3d(-35, 110, 58)
                mainCamera.viewCenter = Qt.vector3d(-20, 96, 0)
            } else if (event.key === Qt.Key_3) {
                // [3] O Poço do Eco (Abismo Vertical, Cornijas em Espiral, Roper, Gelo Fino)
                mainCamera.position = Qt.vector3d(0, 52, 65)
                mainCamera.viewCenter = Qt.vector3d(0, 35, 0)
            } else if (event.key === Qt.Key_4) {
                // [4] As Veias Vítreas (Fortaleza de Kaelen, Jaulas, Duto Secreto no Teto, Fungos)
                mainCamera.position = Qt.vector3d(-15, -18, 52)
                mainCamera.viewCenter = Qt.vector3d(-15, -28, 0)
            } else if (event.key === Qt.Key_5) {
                // [5] O Geodo Primordial (Cristais Refratores, Gravidade Invertida, Porta de Exaustão)
                mainCamera.position = Qt.vector3d(25, -52, 70)
                mainCamera.viewCenter = Qt.vector3d(22, -62, 0)
            } else if (event.key === Qt.Key_6) {
                // [6] O Parasita de Ferro (Alhoon, Coração do Dragão, 8 Cápsulas de Carne Humana)
                mainCamera.position = Qt.vector3d(0, -92, 62)
                mainCamera.viewCenter = Qt.vector3d(0, -112, 0)
            } else if (event.key === Qt.Key_T) {
                // [T] Alterna Modo Mestre vs Modo Tocha Shadowdark
                sceneRoot.dmFullbrightMode = !sceneRoot.dmFullbrightMode
            } else if (event.key === Qt.Key_G) {
                // [G] Emboscada Goblin: Flechas de barro apagam os braseiros
                sceneRoot.goblinBraziersLit = !sceneRoot.goblinBraziersLit
            } else if (event.key === Qt.Key_P) {
                // [P] Liga/Desliga o feixe de luz do Puzzle no Geodo
                sceneRoot.geodePuzzleSolved = !sceneRoot.geodePuzzleSolved
            }
        }
    }

    // =========================================================================
    // ILUMINAÇÃO GLOBAL DO MESTRE & TOCHA DO GRUPO (SHADOWDARK)
    // =========================================================================
    Entity {
        id: globalDmLight
        components: [
            DirectionalLight {
                color: "#8aa4c8"
                intensity: sceneRoot.dmFullbrightMode ? 0.55 : 0.02
                worldDirection: Qt.vector3d(-0.3, -0.6, -0.7)
            }
        ]
    }

    // Tocha do Grupo (Acompanha o viewCenter da câmera para simular o raio de visão curto)
    Entity {
        id: shadowdarkTorch
        components: [
            PointLight {
                color: "#ff9933"
                intensity: sceneRoot.dmFullbrightMode ? 0.4 : 2.2
                constantAttenuation: 1.0
                linearAttenuation: 0.045
                quadraticAttenuation: 0.007
            },
            Transform {
                translation: Qt.vector3d(
                    mainCamera.viewCenter.x,
                    mainCamera.viewCenter.y + 4.0,
                    mainCamera.viewCenter.z + 6.0
                )
            }
        ]
    }

    // =========================================================================
    // CAMADA 0 & 1: ENTRADA CONGELADA + A GARGANTA ERODIDA (OCUPAÇÃO GOBLIN)
    // Altitude: Y = +88 até +115 (Os primeiros 2% da Caverna)
    // =========================================================================
    Entity {
        id: goblinLayer

        // Rótulo 3D da Camada
        Entity {
            components: [
                ExtrudedTextMesh {
                    text: "1. GARGANTA GOBLIN (2% INICIAIS)"
                    depth: 0.6
                    font.family: "Arial"
                    font.pointSize: 3
                },
                PhongMaterial { ambient: "#ffcc66"; diffuse: "#ffdd88" },
                Transform { translation: Qt.vector3d(-68, 112, 12); scale: 1.2 }
            ]
        }

        // Boca da Caverna na Montanha Congelada (Luz Fria Exterior)
        Entity {
            components: [
                TorusMesh { radius: 9; minorRadius: 2.5; rings: 24; slices: 16 },
                PhongMaterial {
                    ambient: sceneRoot.dmFullbrightMode ? "#6c8ba6" : "#1a2634"
                    diffuse: "#a8c8e8"
                },
                Transform {
                    translation: Qt.vector3d(-62, 100, 0)
                    rotationY: 90
                }
            ]
        }
        Entity {
            components: [
                PointLight {
                    color: "#b0d8ff"
                    intensity: 1.4
                    linearAttenuation: 0.04
                },
                Transform { translation: Qt.vector3d(-65, 103, 0) }
            ]
        }

        // Piso Esquerdo (Entrada onde os Goblins acumulam carvão e braseiros roubados)
        Entity {
            components: [
                CuboidMesh { xExtent: 26; yExtent: 3; zExtent: 22 },
                PhongMaterial {
                    ambient: sceneRoot.dmFullbrightMode ? "#2b2623" : "#080706"
                    diffuse: "#4e463f"
                },
                Transform { translation: Qt.vector3d(-48, 92, 0) }
            ]
        }

        // Piso Direito (Lado oposto do Fosso, levando ao Poço do Eco)
        Entity {
            components: [
                CuboidMesh { xExtent: 22; yExtent: 3; zExtent: 22 },
                PhongMaterial {
                    ambient: sceneRoot.dmFullbrightMode ? "#2b2623" : "#080706"
                    diffuse: "#4e463f"
                },
                Transform { translation: Qt.vector3d(-2, 92, 0) }
            ]
        }

        // Braseiros Sagrados de Bronze Roubados da Vila (A falsa pista do Padre)
        NodeInstantiator {
            model: 3
            delegate: Entity {
                readonly property real bx: -54 + index * 7
                readonly property real bz: (index % 2 === 0) ? -5 : 5

                // Pedestal de Bronze
                Entity {
                    components: [
                        CylinderMesh { radius: 1.1; length: 2.2 },
                        PhongMaterial { ambient: "#5a3d1c"; diffuse: "#cd7f32"; specular: "#ffd700"; shininess: 60 },
                        Transform { translation: Qt.vector3d(bx, 94.6, bz) }
                    ]
                }
                // Chama do Braseiro (Apaga quando G é pressionado - Flechas de Barro com Água)
                Entity {
                    enabled: sceneRoot.goblinBraziersLit
                    components: [
                        SphereMesh { radius: 0.9 },
                        PhongMaterial { ambient: "#ff6600"; diffuse: "#ffaa00" },
                        PointLight {
                            color: "#ff7711"
                            intensity: sceneRoot.goblinBraziersLit ? 1.8 : 0.0
                            linearAttenuation: 0.06
                            quadraticAttenuation: 0.01
                        },
                        Transform { translation: Qt.vector3d(bx, 96.2, bz) }
                    ]
                }
            }
        }

        // O Fosso com Estacas e Rio Subterrâneo Violento (Abaixo da Ponte)
        Entity {
            components: [
                CuboidMesh { xExtent: 22; yExtent: 1.5; zExtent: 22 },
                PhongMaterial { ambient: "#0a223a"; diffuse: "#164c78"; specular: "#88ccff"; shininess: 90 },
                Transform { translation: Qt.vector3d(-24, 81, 0) }
            ]
        }

        // Estacas Afiadas no Fundo do Fosso Goblin
        NodeInstantiator {
            model: 14
            delegate: Entity {
                components: [
                    ConeMesh { bottomRadius: 0.55; length: 4.5 },
                    PhongMaterial { ambient: "#2d1c0f"; diffuse: "#5c3a21" },
                    Transform {
                        translation: Qt.vector3d(
                            -32 + (index % 7) * 2.7,
                            83.5,
                            (index < 7) ? -3.0 : 3.0
                        )
                    }
                ]
            }
        }

        // A Ponte de Madeira Podre (Tábuas espaçadas sobre o abismo)
        NodeInstantiator {
            model: 10
            delegate: Entity {
                components: [
                    CuboidMesh { xExtent: 1.4; yExtent: 0.35; zExtent: 4.5 },
                    PhongMaterial { ambient: "#332211"; diffuse: "#6b4423" },
                    Transform {
                        translation: Qt.vector3d(-33.5 + index * 2.1, 93.4, 0)
                        // Algumas tábuas tortas para evidenciar a fragilidade
                        rotationZ: (index % 3 === 0) ? 4 : -3
                    }
                ]
            }
        }

        // Paredão de Rocha Traseiro com Fendas Táticas ("Murder Holes" / Nimble Escape)
        Entity {
            components: [
                CuboidMesh { xExtent: 72; yExtent: 18; zExtent: 2.5 },
                PhongMaterial {
                    ambient: sceneRoot.dmFullbrightMode ? "#1f1d24" : "#050507"
                    diffuse: "#3a3642"
                },
                Transform { translation: Qt.vector3d(-26, 100, -12) }
            ]
        }

        // Galeria/Túnel Estreito dos Goblins (Por trás dos buracos na parede + Pontos de Sniper)
        NodeInstantiator {
            model: 5
            delegate: Entity {
                readonly property real sx: -46 + index * 10
                // Nicho/Buraco de Tiro na Parede Alta
                Entity {
                    components: [
                        CuboidMesh { xExtent: 3.2; yExtent: 2.2; zExtent: 4.0 },
                        PhongMaterial { ambient: "#050505"; diffuse: "#111111" },
                        Transform { translation: Qt.vector3d(sx, 101.5, -10.5) }
                    ]
                }
                // Marcador Verde indicando Posição de Arqueiro Goblin (Com Flechas de Água)
                Entity {
                    components: [
                        SphereMesh { radius: 0.75 },
                        PhongMaterial { ambient: "#226622"; diffuse: "#44cc44" },
                        Transform { translation: Qt.vector3d(sx, 101.5, -9.2) }
                    ]
                }
            }
        }

        // Passagem Secreta atrás da Cascata Congelada (Flanco Norte)
        Entity {
            components: [
                CuboidMesh { xExtent: 6; yExtent: 14; zExtent: 0.8 },
                PhongAlphaMaterial {
                    ambient: "#66ccff"
                    diffuse: "#99e6ff"
                    alpha: 0.55
                },
                Transform { translation: Qt.vector3d(-38, 99, -9.8) }
            ]
        }
        // Túnel de contorno oculto atrás da cascata e dos buracos
        Entity {
            components: [
                CuboidMesh { xExtent: 48; yExtent: 2.5; zExtent: 3.5 },
                PhongMaterial { ambient: "#18242c"; diffuse: "#2c4251" },
                Transform { translation: Qt.vector3d(-26, 100.2, -15.5) }
            ]
        }
    }

    // =========================================================================
    // CAMADA 2: O POÇO DO ECO (TRANSIÇÃO VERTICAL EXTREMA + ROPER + GELO FINO)
    // Altitude: Y = +88 descendo até Y = -18 (106 metros de queda vertical)
    // =========================================================================
    Entity {
        id: echoPitLayer

        Entity {
            components: [
                ExtrudedTextMesh {
                    text: "2. O POCO DO ECO (DESCIDA VERTICAL & ROPER)"
                    depth: 0.6
                    font.family: "Arial"
                    font.pointSize: 3
                },
                PhongMaterial { ambient: "#66ccff"; diffuse: "#99ddff" },
                Transform { translation: Qt.vector3d(18, 75, 12); scale: 1.2 }
            ]
        }

        // Anéis de Rocha da Chaminé Vulcânica (Semi-abertos na frente para corte transversal)
        NodeInstantiator {
            model: 7
            delegate: Entity {
                components: [
                    TorusMesh { radius: 16; minorRadius: 2.2; rings: 28; slices: 16 },
                    PhongMaterial {
                        ambient: sceneRoot.dmFullbrightMode ? "#1c2029" : "#040507"
                        diffuse: "#333947"
                    },
                    Transform {
                        translation: Qt.vector3d(0, 82 - index * 16, 0)
                        rotationX: 90
                    }
                ]
            }
        }

        // Espiral de Cornijas Naturais de Calcário (28 degraus descendo pela parede do abismo)
        NodeInstantiator {
            model: 28
            delegate: Entity {
                readonly property real angle: index * 0.42
                readonly property real radius: 13.0
                readonly property real lx: Math.cos(angle) * radius
                readonly property real lz: Math.sin(angle) * radius
                readonly property real ly: 88.0 - index * 3.7

                Entity {
                    components: [
                        CuboidMesh { xExtent: 4.8; yExtent: 1.1; zExtent: 3.4 },
                        PhongMaterial {
                            ambient: sceneRoot.dmFullbrightMode ? "#2e3440" : "#090b0e"
                            diffuse: "#586375"
                        },
                        Transform {
                            translation: Qt.vector3d(lx, ly, lz)
                            rotationY: -angle * (180.0 / Math.PI)
                        }
                    ]
                }
            }
        }

        // Ponte de Estalactite Caída (Cruzando o meio do Abismo em Y = +46)
        Entity {
            components: [
                CylinderMesh { radius: 1.6; length: 28 },
                PhongMaterial { ambient: "#2a2e38"; diffuse: "#525866" },
                Transform {
                    translation: Qt.vector3d(0, 46, 0)
                    rotationZ: 82
                }
            ]
        }

        // O Covil do Açoitador / Roper Glacial (Camuflado entre Estalactites de Gelo em Y = +56)
        NodeInstantiator {
            model: 9
            delegate: Entity {
                readonly property bool isRoper: (index === 4)
                components: [
                    ConeMesh {
                        bottomRadius: isRoper ? 2.2 : 1.1
                        length: isRoper ? 9.5 : 6.0
                    },
                    PhongMaterial {
                        ambient: isRoper ? "#4a154b" : "#3a6b88"
                        diffuse: isRoper ? "#8a2be2" : "#77c4ef"
                        specular: "#ffffff"
                        shininess: 80
                    },
                    Transform {
                        translation: Qt.vector3d(-8 + (index % 3) * 4, 58, -6 + Math.floor(index / 3) * 4)
                        rotationX: 180 // Penduradas do teto da cornija
                    }
                ]
            }
        }

        // Corredor da Armadilha de Contraste Térmico (Y = +18: Água Fervente sob Camada Fina de Gelo)
        Entity {
            // Água Vulcânica Escaldante (Fundo Vermelho/Laranja brilhante)
            Entity {
                components: [
                    CuboidMesh { xExtent: 26; yExtent: 2.0; zExtent: 10 },
                    PhongMaterial { ambient: "#cc3300"; diffuse: "#ff5500" },
                    PointLight {
                        color: "#ff4400"
                        intensity: 1.1
                        linearAttenuation: 0.07
                    },
                    Transform { translation: Qt.vector3d(26, 15.5, 0) }
                ]
            }
            // Camada Fina de Gelo Mágico Translúcido na Superfície
            Entity {
                components: [
                    CuboidMesh { xExtent: 26; yExtent: 0.35; zExtent: 10 },
                    PhongAlphaMaterial {
                        ambient: "#88ddff"
                        diffuse: "#ccf2ff"
                        alpha: 0.6
                    },
                    Transform { translation: Qt.vector3d(26, 17.2, 0) }
                ]
            }
        }
    }

    // =========================================================================
    // CAMADA 3: AS VEIAS VÍTREAS - O DOMÍNIO DE KAELEN (O LENHADOR / MID-BOSS)
    // Altitude: Y = -18 até Y = -42 (Tubos de Magma de Obsidiana Negra)
    // =========================================================================
    Entity {
        id: kaelenLayer

        Entity {
            components: [
                ExtrudedTextMesh {
                    text: "3. VEIAS VITREAS (FORTALEZA DE KAELEN & FUNGOS)"
                    depth: 0.6
                    font.family: "Arial"
                    font.pointSize: 3
                },
                PhongMaterial { ambient: "#ff7744"; diffuse: "#ff9966" },
                Transform { translation: Qt.vector3d(-75, -14, 12); scale: 1.2 }
            ]
        }

        // Piso de Obsidiana Negra Polida
        Entity {
            components: [
                CuboidMesh { xExtent: 64; yExtent: 2.5; zExtent: 28 },
                PhongMaterial {
                    ambient: "#08060c"
                    diffuse: "#161224"
                    specular: "#7755aa"
                    shininess: 120
                },
                Transform { translation: Qt.vector3d(-18, -32, 0) }
            ]
        }

        // Passagem Secreta Superior: Duto de Respiro Vulcânico no Teto (Para espionar Kaelen)
        Entity {
            components: [
                CuboidMesh { xExtent: 42; yExtent: 1.2; zExtent: 5.0 },
                PhongMaterial { ambient: "#221a2e"; diffuse: "#3d2f52" },
                Transform { translation: Qt.vector3d(-20, -17.5, 0) }
            ]
        }

        // Jaulas de Ferro Penduradas (Aldeões Sequestrados aguardando envio ao Lich)
        NodeInstantiator {
            model: 4
            delegate: Entity {
                readonly property real cx: -38 + index * 9
                // Corrente
                Entity {
                    components: [
                        CylinderMesh { radius: 0.2; length: 6.0 },
                        PhongMaterial { ambient: "#444444"; diffuse: "#888888" },
                        Transform { translation: Qt.vector3d(cx, -21, -5) }
                    ]
                }
                // Gaiola
                Entity {
                    components: [
                        CylinderMesh { radius: 2.0; length: 3.8 },
                        PhongAlphaMaterial { ambient: "#553322"; diffuse: "#996644"; alpha: 0.7 },
                        Transform { translation: Qt.vector3d(cx, -25.5, -5) }
                    ]
                }
            }
        }

        // Posto de Comando de Kaelen (Mesa com o Diário Matemático + Kaelen + 2 Hell Hounds)
        Entity {
            // Pódio de Obsidiana
            Entity {
                components: [
                    CylinderMesh { radius: 4.5; length: 1.5 },
                    PhongMaterial { ambient: "#220a18"; diffuse: "#441530" },
                    Transform { translation: Qt.vector3d(-26, -30, 4) }
                ]
            }
            // Kaelen (Token Vermelho/Necrótico)
            Entity {
                components: [
                    CylinderMesh { radius: 1.0; length: 3.8 },
                    PhongMaterial { ambient: "#990022"; diffuse: "#ff1144" },
                    PointLight { color: "#ff2244"; intensity: 1.5; linearAttenuation: 0.08 },
                    Transform { translation: Qt.vector3d(-26, -27.5, 4) }
                ]
            }
            // 2 Hell Hounds Corrompidos
            NodeInstantiator {
                model: 2
                delegate: Entity {
                    components: [
                        SphereMesh { radius: 1.1 },
                        PhongMaterial { ambient: "#cc3300"; diffuse: "#ff6600" },
                        Transform { translation: Qt.vector3d(-22 + index * -8, -28.5, 7) }
                    ]
                }
            }
        }

        // Túnel Lateral dos Fungos da Memória (Myconids Bioluminescentes - Lore do Dragão)
        NodeInstantiator {
            model: 6
            delegate: Entity {
                readonly property real fx: -46 + index * 3.5
                readonly property real fz: 9 + (index % 2) * 3
                // Tronco do Cogumelo Gigante
                Entity {
                    components: [
                        CylinderMesh { radius: 0.4; length: 3.5 },
                        PhongMaterial { ambient: "#1b3a32"; diffuse: "#346e5f" },
                        Transform { translation: Qt.vector3d(fx, -29, fz) }
                    ]
                }
                // Chapéu Bioluminescente
                Entity {
                    components: [
                        ConeMesh { bottomRadius: 2.1; length: 1.6 },
                        PhongMaterial { ambient: "#00cc88"; diffuse: "#33ffbb" },
                        PointLight {
                            color: "#00ffaa"
                            intensity: 0.65
                            linearAttenuation: 0.12
                        },
                        Transform { translation: Qt.vector3d(fx, -27, fz) }
                    ]
                }
            }
        }
    }

    // =========================================================================
    // CAMADA 4: O GEODO PRIMORDIAL (PUZZLE DE CRISTAIS, GRAVIDADE E EXAUSTÃO)
    // Altitude: Y = -45 até Y = -78 (Câmara Colossal de Cristais)
    // =========================================================================
    Entity {
        id: geodeLayer

        Entity {
            components: [
                ExtrudedTextMesh {
                    text: "4. O GEODO PRIMORDIAL (CRISTAIS & PORTA DE EXAUSTAO)"
                    depth: 0.6
                    font.family: "Arial"
                    font.pointSize: 3
                },
                PhongMaterial { ambient: "#ee66ff"; diffuse: "#ff99ff" },
                Transform { translation: Qt.vector3d(16, -44, 12); scale: 1.2 }
            ]
        }

        // Piso da Grande Câmara do Geodo
        Entity {
            components: [
                CylinderMesh { radius: 36; length: 2.5 },
                PhongMaterial {
                    ambient: sceneRoot.dmFullbrightMode ? "#1f142e" : "#07040a"
                    diffuse: "#372352"
                },
                Transform { translation: Qt.vector3d(20, -68, 0) }
            ]
        }

        // Torres de Cristais Prismáticos Gigantes (Círculo de Refração de Luz)
        NodeInstantiator {
            model: 6
            delegate: Entity {
                readonly property real cAngle: index * (Math.PI / 3.0)
                readonly property real cx: 20 + Math.cos(cAngle) * 20
                readonly property real cz: Math.sin(cAngle) * 20

                // Cristal Prismático
                Entity {
                    components: [
                        CylinderMesh { radius: 2.2; length: 14; slices: 6 },
                        PhongAlphaMaterial {
                            ambient: (index % 2 === 0) ? "#9900cc" : "#0099cc"
                            diffuse: (index % 2 === 0) ? "#e64dff" : "#33ddff"
                            alpha: 0.8
                        },
                        PointLight {
                            color: (index % 2 === 0) ? "#d933ff" : "#33ddff"
                            intensity: 0.9
                            linearAttenuation: 0.08
                        },
                        Transform {
                            translation: Qt.vector3d(cx, -60, cz)
                            rotationZ: (index % 2 === 0) ? 12 : -12
                        }
                    ]
                }

                // Feixes de Luz Refletida entre os Cristais (Ativados por [P])
                Entity {
                    enabled: sceneRoot.geodePuzzleSolved
                    components: [
                        CylinderMesh { radius: 0.35; length: 20 },
                        PhongMaterial { ambient: "#00ffff"; diffuse: "#ffffff" },
                        Transform {
                            translation: Qt.vector3d(20 + Math.cos(cAngle) * 10, -58, Math.sin(cAngle) * 10)
                            rotationY: -cAngle * (180.0 / Math.PI)
                            rotationZ: 90
                        }
                    ]
                }
            }
        }

        // Zona de Gravidade Distorcida (Plataformas e Estalagmites Invertidas no Teto do Geodo)
        NodeInstantiator {
            model: 7
            delegate: Entity {
                components: [
                    CuboidMesh { xExtent: 4.2; yExtent: 1.5; zExtent: 4.2 },
                    PhongMaterial { ambient: "#442266"; diffuse: "#8844cc" },
                    Transform {
                        translation: Qt.vector3d(2 + index * 6, -47 - (index % 2) * 2.5, -10)
                        rotationZ: 180
                    }
                ]
            }
        }

        // A Porta de Adamante & Os 3 Selos de Exaustão (Engrenagens que exigem Calor Vital / Hit Dice)
        Entity {
            id: adamantineGate
            // Marco da Porta de Adamante
            Entity {
                components: [
                    CuboidMesh { xExtent: 14; yExtent: 14; zExtent: 2.2 },
                    PhongMaterial {
                        ambient: "#101820"
                        diffuse: "#203040"
                        specular: "#88ffcc"
                        shininess: 150
                    },
                    Transform { translation: Qt.vector3d(20, -60, -24) }
                ]
            }

            // As 3 Engrenagens Arcanas de Exaustão
            NodeInstantiator {
                model: 3
                delegate: Entity {
                    components: [
                        TorusMesh { radius: 2.2; minorRadius: 0.5; rings: 12; slices: 8 },
                        PhongMaterial {
                            ambient: "#ff3300"
                            diffuse: "#ff8800"
                        },
                        Transform {
                            translation: Qt.vector3d(15 + index * 5, -60, -22.5)
                            rotationZ: sceneRoot.dragonPulse * 90 * (index + 1)
                        }
                    ]
                }
            }
        }
    }

    // =========================================================================
    // CAMADA 5: O PARASITA DE FERRO & O CORAÇÃO DO DRAGÃO (COVIL DO ALHOON / LICH)
    // Altitude: Y = -85 até Y = -125 (O Fundo da Fenda Tectônica)
    // =========================================================================
    Entity {
        id: lichCoreLayer

        Entity {
            components: [
                ExtrudedTextMesh {
                    text: "5. O PARASITA DE FERRO (CORACAO DO DRAGAO & ALHOON)"
                    depth: 0.6
                    font.family: "Arial"
                    font.pointSize: 3
                },
                PhongMaterial { ambient: "#ff3300"; diffuse: "#ff5522" },
                Transform { translation: Qt.vector3d(-72, -88, 12); scale: 1.2 }
            ]
        }

        // Cratera Basal de Magma e Rocha Vulcânica
        Entity {
            components: [
                CylinderMesh { radius: 42; length: 3.0 },
                PhongMaterial { ambient: "#1a0505"; diffuse: "#330a0a" },
                Transform { translation: Qt.vector3d(0, -122, 0) }
            ]
        }

        // Anel Brutalista de Andaimes de Ferro Enferrujado do Lich
        Entity {
            components: [
                TorusMesh { radius: 24; minorRadius: 1.4; rings: 32; slices: 12 },
                PhongMaterial { ambient: "#2b1d16"; diffuse: "#593a2b"; specular: "#aa7755"; shininess: 40 },
                Transform {
                    translation: Qt.vector3d(0, -108, 0)
                    rotationX: 90
                }
            ]
        }

        // O CORAÇÃO DO DRAGÃO ANCESTRAL (Núcleo Pulsante de Fogo Elemental Primordial)
        Entity {
            id: dragonHeartCore
            components: [
                SphereMesh { radius: 7.5; rings: 32; slices: 32 },
                PhongMaterial {
                    ambient: "#ff3300"
                    diffuse: "#ffaa00"
                    specular: "#ffffff"
                    shininess: 100
                },
                PointLight {
                    color: "#ff4500"
                    intensity: 3.8 * sceneRoot.dragonPulse
                    constantAttenuation: 1.0
                    linearAttenuation: 0.02
                    quadraticAttenuation: 0.002
                },
                Transform {
                    translation: Qt.vector3d(0, -110, 0)
                    scale: sceneRoot.dragonPulse
                }
            ]
        }

        // A ENGENHARIA DO SOFRIMENTO: 8 Cápsulas de Carne Humana + Tubos de Sifão de Bronze
        // (A cápsula de índice 0 pisca em vermelho indicando falha: é a vaga que o Lich pede para um jogador!)
        NodeInstantiator {
            model: 8
            delegate: Entity {
                readonly property real pAngle: index * (Math.PI / 4.0)
                readonly property real px: Math.cos(pAngle) * 22
                readonly property real pz: Math.sin(pAngle) * 22
                readonly property bool isFailingSlot: (index === 0)

                // Tubo de Sifão de Bronze drenando o calor do Coração para a Cápsula
                Entity {
                    components: [
                        CylinderMesh { radius: 0.55; length: 16 },
                        PhongMaterial { ambient: "#4d3319"; diffuse: "#996633"; specular: "#ffcc66"; shininess: 50 },
                        Transform {
                            translation: Qt.vector3d(Math.cos(pAngle) * 14, -110, Math.sin(pAngle) * 14)
                            rotationY: -pAngle * (180.0 / Math.PI)
                            rotationZ: 90
                        }
                    ]
                }

                // Cilindro de Vidro Necromântico (Cápsula Humana)
                Entity {
                    components: [
                        CylinderMesh { radius: 2.3; length: 7.5 },
                        PhongAlphaMaterial {
                            ambient: isFailingSlot ? "#990000" : "#008855"
                            diffuse: isFailingSlot ? "#ff2222" : "#00ffaa"
                            alpha: 0.65
                        },
                        PointLight {
                            color: isFailingSlot ? "#ff0000" : "#00ff88"
                            intensity: 1.1
                            linearAttenuation: 0.1
                        },
                        Transform { translation: Qt.vector3d(px, -110, pz) }
                    ]
                }

                // Silhueta do Conduíte Humano dentro da Cápsula (Exceto na cápsula vazia/esgotada)
                Entity {
                    enabled: !isFailingSlot
                    components: [
                        CylinderMesh { radius: 0.7; length: 4.5 },
                        PhongMaterial { ambient: "#111111"; diffuse: "#222222" },
                        Transform { translation: Qt.vector3d(px, -110, pz) }
                    ]
                }
            }
        }

        // O Altar de Controle do Alhoon (Lich Devorador de Mentes)
        Entity {
            // Plataforma Elevada Necromântica
            Entity {
                components: [
                    CylinderMesh { radius: 5.5; length: 2.0 },
                    PhongMaterial { ambient: "#140a24"; diffuse: "#2b164d" },
                    Transform { translation: Qt.vector3d(0, -102, -26) }
                ]
            }
            // Representação do Alhoon (Manto Púrpura + Aura Psíquica)
            Entity {
                components: [
                    ConeMesh { bottomRadius: 1.6; length: 5.2 },
                    PhongMaterial { ambient: "#5500aa"; diffuse: "#aa00ff" },
                    PointLight { color: "#bf00ff"; intensity: 2.2; linearAttenuation: 0.07 },
                    Transform { translation: Qt.vector3d(0, -98.5, -26) }
                ]
            }
        }
    }
}