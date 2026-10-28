// Janela principal: laco de atualizacao, entrada, carregamento, capturas (--shots) e HUD.
#pragma once
#include "CommandLine.h"
#include "world/World.h"
#include "world/Mesher.h"
#include "game/Player.h"
#include "render/Renderer.h"
#include "ui/Hud.h"
#include <QElapsedTimer>
#include <QOpenGLWindow>
#include <QSet>
#include <memory>

class GameWindow : public QOpenGLWindow {
public:
    GameWindow(World& world, const Options& opt);
    ~GameWindow() override;

protected:
    void initializeGL() override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;

private:
    enum class Phase { Loading, Playing, Shots };
    struct ShotJob { Viewpoint vp; QString file; bool dm; bool ov = false; float yaw = 35.f, pitch = 40.f; int ui = 0; };

    void loadStep(float dt);
    void beginPlay();
    void frame(float dt);
    void stepSim(float dt, const Player::Input& in);
    void updateCamera(float aspect);
    FrameParams buildFrame(int pw, int ph, unsigned fbo, bool overviewAllowed);
    void drawHud(QPainter& p, int w, int h);
    void goViewpoint(const Viewpoint& vp);
    void teleportNear(const Vec3& target);
    void startAt(const Viewpoint& vp);
    void openNote(const Poi* p);
    bool menuOpen() const { return m_ui.tabMenu || m_ui.notesOpen || m_ui.help; }
    void setCaptured(bool c);
    Player::Input gatherInput();
    void toggleOverview();
    void saveScreenshot();
    void shotStep();
    void prepareShots();
    void setModeFlags();

    World& m_world;
    Options m_opt;
    Player m_player;
    Renderer m_renderer;
    Hud m_hud;
    UiState m_ui;
    std::unique_ptr<Mesher> m_mesher;
    Phase m_phase = Phase::Loading;
    QElapsedTimer m_timer;
    qint64 m_lastNs = 0;
    float m_avgDt = 0.016f;
    float m_fpsAcc = 0.f; int m_fpsFrames = 0;
    QSet<int> m_keys;
    bool m_jumpPressed = false;
    bool m_captured = false;
    QPoint m_centerGlobal;
    bool m_wantShot = false;
    float m_under = 0.f, m_dark = 0.f;
    Light m_torchLight;
    // vista geral
    float m_ovYaw = 35.f, m_ovPitch = 40.f, m_ovDist = 500.f;
    bool m_ovDrag = false; QPoint m_ovLast; QPoint m_ovPress;
    // visual de desenvolvimento
    bool m_debugDraw = false, m_wire = false, m_clip = false;
    int m_clipAxis = 1; float m_clipPos = 0.f;
    int m_terrainMode = 0;      // 0 normal, 1 sem terreno, 2 raio-x
    QMatrix4x4 m_view, m_proj;
    Vec3 m_camPos;
    // capturas
    std::vector<ShotJob> m_jobs;
    size_t m_jobIndex = 0;
    QStringList m_shotFiles;
    double m_shotRenderMs = 0.0; int m_shotCount = 0;
    int m_warmFrames = 0;
    QElapsedTimer m_loadTimer;
    bool m_loadDone = false;
    double m_benchTime = 0.0; int m_benchFrames = 0;
};
