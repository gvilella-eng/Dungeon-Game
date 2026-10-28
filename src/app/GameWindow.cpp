#include "GameWindow.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLContext>
#include <QWheelEvent>
#include <QPainter>
#include <QImage>

static const float kFovY = 68.f;

static QString sanitize(const QString& s)
{
    QString r;
    for (QChar c : s.normalized(QString::NormalizationForm_D)) {
        if (c.category() == QChar::Mark_NonSpacing) continue;
        if (c.isLetterOrNumber()) r += c.toLower(); else if (!r.endsWith('_')) r += '_';
    }
    while (r.endsWith('_')) r.chop(1);
    return r;
}

GameWindow::GameWindow(World& world, const Options& opt) : m_world(world), m_opt(opt), m_player(world)
{
    m_world.attachPlayer(&m_player);
    m_torchLight.anim = LightAnim::TorchFlicker; m_torchLight.animDepth = 0.28f;
    setTitle(QStringLiteral("A Chama Primordial — Masmorra"));
    m_timer.start();
}

GameWindow::~GameWindow()
{
    makeCurrent();
    m_mesher.reset();
    m_renderer.clearChunks();
    doneCurrent();
}

void GameWindow::initializeGL()
{
    auto* f = context()->versionFunctions<QOpenGLFunctions_3_3_Core>();
    f->initializeOpenGLFunctions();
    GL = f;
    outLine(QStringLiteral("OpenGL: %1 | %2").arg(QString::fromLatin1((const char*)GL->glGetString(GL_RENDERER))).arg(QString::fromLatin1((const char*)GL->glGetString(GL_VERSION))));
    if (!m_renderer.init()) outLine(QStringLiteral("ERRO: falha ao inicializar o renderer (veja os avisos de shader acima)."));
    Mesher::Config cfg;
    cfg.useCache = !m_opt.noCache;
    cfg.cacheDir = QCoreApplication::applicationDirPath() + "/cache";
    m_mesher = std::make_unique<Mesher>(m_world.sdf, cfg);
    m_mesher->start();
    m_loadTimer.start();
    m_ovDist = m_world.overviewRadius() * 1.9f;
    m_ui.onViewpoint = [this](const Viewpoint& v) { goViewpoint(v); };
    m_ui.onPoiGo = [this](const Poi* p) { teleportNear(p->pos); };
    m_ui.onPoiOpen = [this](const Poi* p) { openNote(p); };
    m_ui.onEvent = [this](const QString& id) { m_world.fireEvent(id); m_ui.tabMenu = false; setCaptured(true); };
}

void GameWindow::setCaptured(bool c)
{
    if (m_captured == c) return;
    m_captured = c;
    if (c) {
        setCursor(Qt::BlankCursor);
        m_centerGlobal = mapToGlobal(QPoint(width() / 2, height() / 2));
        QCursor::setPos(m_centerGlobal);
    } else unsetCursor();
}

void GameWindow::paintGL()
{
    qint64 now = m_timer.nsecsElapsed();
    float dt = (float)((now - m_lastNs) * 1e-9);
    m_lastNs = now;
    dt = std::min(dt, 0.1f);
    switch (m_phase) {
    case Phase::Loading: loadStep(dt); break;
    case Phase::Playing: frame(dt); break;
    case Phase::Shots: shotStep(); break;
    }
    update();
}

void GameWindow::loadStep(float)
{
    std::vector<ChunkMesh> v;
    if (m_mesher->poll(v) > 0) m_renderer.addChunks(v);
    GL->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    GL->glViewport(0, 0, (int)(width() * devicePixelRatio()), (int)(height() * devicePixelRatio()));
    GL->glClearColor(0.04f, 0.03f, 0.04f, 1.f);
    GL->glClear(GL_COLOR_BUFFER_BIT);
    {
        QPainter p(this);
        QString st = m_mesher->fromCache() ? QStringLiteral("Carregando a masmorra do cache…") : QStringLiteral("Escavando a montanha…");
        QString det = QStringLiteral("%1 / %2 blocos · %3 mil vértices").arg((int)(m_mesher->progress() * m_mesher->totalChunks())).arg(m_mesher->totalChunks()).arg((qulonglong)(m_mesher->vertexCount() / 1000));
        m_hud.paintLoading(p, width(), height(), m_mesher->progress(), st, det);
    }
    if (m_mesher->finished()) {
        m_mesher->waitAll();
        double secs = m_mesher->elapsedSeconds();
        outLine(QStringLiteral("Malha: %1 chunks, %2 vértices em %3 s (%4)").arg(m_renderer.chunkCount()).arg((qulonglong)m_mesher->vertexCount()).arg(secs, 0, 'f', 2).arg(m_mesher->fromCache() ? "cache" : "calculado"));
        outLine(QStringLiteral("Carregamento total até o jogo: %1 s").arg(m_loadTimer.elapsed() / 1000.0, 0, 'f', 2));
        beginPlay();
    }
}

void GameWindow::startAt(const Viewpoint& vp)
{
    m_player.mode = vp.fly ? Player::Mode::Fly : Player::Mode::Walk;
    Vec3 feet = vp.pos - Vec3(0, 1.65f, 0);
    m_player.up = Vec3(0, 1, 0);
    m_player.teleport(feet, vp.yawDeg, vp.pitchDeg);
    if (!vp.fly) m_player.snapToGround();
    updateCamera((float)width() / std::max(1, height()));
    for (int i = 0; i < 3; ++i) m_world.update(0.016f);
}

void GameWindow::beginPlay()
{
    Viewpoint start;
    bool have = false;
    if (!m_opt.start.isEmpty())
        for (const Viewpoint& v : m_world.viewpoints()) if (v.name.contains(m_opt.start, Qt::CaseInsensitive)) { start = v; have = true; break; }
    if (!have && m_world.hasSpawn()) { start = m_world.spawn(); have = true; }
    if (!have && !m_world.viewpoints().empty()) { start = m_world.viewpoints().front(); have = true; }
    if (!have) { start.pos = Vec3(0, 1.65f, 0); }
    m_player.up = Vec3(0, 1, 0);
    startAt(start);
    m_world.state.setHint(QStringLiteral("[R] acender tocha · [F1] ajuda"), 10.f);
    m_world.state.fade(0.f, 0.01f);
    if (m_opt.shots) { prepareShots(); m_phase = Phase::Shots; return; }
    m_phase = Phase::Playing;
    setCaptured(true);
}

void GameWindow::updateCamera(float aspect)
{
    CameraInfo& c = m_world.cam;
    c.pos = m_player.eyePos(); c.fwd = m_player.fwd; c.up = m_player.up;
    c.view = m_player.viewMatrix();
    c.proj.setToIdentity(); c.proj.perspective(kFovY, aspect, 0.05f, 1500.f);
    c.frustum.set(c.proj * c.view);
    c.fovY = kFovY; c.aspect = aspect; c.valid = true;
}

Player::Input GameWindow::gatherInput()
{
    Player::Input in;
    if (menuOpen()) return in;
    in.forward = (m_keys.contains(Qt::Key_W) || m_keys.contains(Qt::Key_Up) ? 1.f : 0.f) - (m_keys.contains(Qt::Key_S) || m_keys.contains(Qt::Key_Down) ? 1.f : 0.f);
    in.strafe = (m_keys.contains(Qt::Key_D) || m_keys.contains(Qt::Key_Right) ? 1.f : 0.f) - (m_keys.contains(Qt::Key_A) || m_keys.contains(Qt::Key_Left) ? 1.f : 0.f);
    in.jump = m_jumpPressed; m_jumpPressed = false;
    in.jumpHeld = m_keys.contains(Qt::Key_Space);
    in.crouch = m_keys.contains(Qt::Key_Control);
    in.sprint = m_keys.contains(Qt::Key_Shift);
    return in;
}

void GameWindow::stepSim(float dt, const Player::Input& in)
{
    m_player.update(dt, in);
    updateCamera(m_world.cam.valid ? m_world.cam.aspect : 16.f / 9.f);
    m_world.update(dt);
    GameState& st = m_world.state;
    st.update(dt);
    if (st.torchLit && m_player.eyeUnderwater) st.extinguishTorch(QStringLiteral("A água apagou sua tocha!"));
}

void GameWindow::setModeFlags()
{
    QStringList f;
    if (m_ui.fullbright) f << QStringLiteral("Luz do Mestre (F3)");
    if (m_wire) f << QStringLiteral("Wireframe (F6)");
    if (m_debugDraw) f << QStringLiteral("Zonas / gatilhos / colisores (F5)");
    if (m_clip) f << QStringLiteral("Plano de corte %1 = %2  ( [ ] mover · F8 eixo )").arg(QChar("XYZ"[m_clipAxis])).arg(m_clipPos, 0, 'f', 1);
    if (m_terrainMode == 1) f << QStringLiteral("Terreno escondido (F9)");
    if (m_terrainMode == 2) f << QStringLiteral("Raio-X (F9)");
    if (m_world.state.darkvision) f << QStringLiteral("Visão no escuro");
    m_ui.modeFlags = f;
}

FrameParams GameWindow::buildFrame(int pw, int ph, unsigned fbo, bool)
{
    FrameParams fp;
    fp.w = pw; fp.h = ph; fp.targetFbo = fbo;
    float aspect = (float)pw / std::max(1, ph);
    const float t = m_world.time();
    fp.time = t;
    fp.zone = m_world.zoneState();
    fp.ambUp = m_player.up;
    GameState& st = m_world.state;
    if (m_ui.overview) {
        Vec3 tgt = m_world.overviewCenter();
        float cy = std::cos(m_ovPitch * 0.0174533f), sy = std::sin(m_ovPitch * 0.0174533f);
        Vec3 dir(std::sin(m_ovYaw * 0.0174533f) * cy, sy, std::cos(m_ovYaw * 0.0174533f) * cy);
        Vec3 eye = tgt + dir * m_ovDist;
        fp.view.setToIdentity(); fp.view.lookAt(eye, tgt, Vec3(0, 1, 0));
        fp.proj.setToIdentity(); fp.proj.perspective(45.f, aspect, std::max(1.f, m_ovDist * 0.01f), m_ovDist * 6.f + 4000.f);
        fp.camPos = eye;
        fp.fullbright = true; fp.noFog = true; fp.backTint = true;
    } else {
        fp.view = m_world.cam.view; fp.proj = m_world.cam.proj; fp.camPos = m_world.cam.pos;
        fp.fullbright = m_ui.fullbright; fp.backTint = m_ui.fullbright; fp.noFog = false;
    }
    fp.xray = m_terrainMode == 2; fp.hideTerrain = m_terrainMode >= 1;
    fp.wireframe = m_wire; fp.debugDraw = m_debugDraw;
    if (m_clip) {
        Vec3 n = m_clipAxis == 0 ? Vec3(1, 0, 0) : (m_clipAxis == 1 ? Vec3(0, 1, 0) : Vec3(0, 0, 1));
        fp.clipOn = true; fp.clip = QVector4D(n, m_clipPos);
    }
    // tocha na mao direita, abaixo do olho
    if (st.torchLit && !m_ui.overview) {
        float scale = fp.zone.torchScale;
        m_torchLight.intensity = 1.f;
        float fl = m_torchLight.intensityAt(t);
        if (fp.zone.windGust) fl *= 0.6f;
        fp.torchOn = true;
        fp.torchPos = m_world.cam.pos + m_player.right() * 0.28f - m_player.up * 0.22f + m_player.fwd * 0.30f;
        fp.torchIntensity = 4.5f * fl * scale;
        fp.torchRange = 18.f * (0.55f + 0.45f * scale);
        float low = st.torchSeconds < 120.f ? std::max(0.35f, st.torchSeconds / 120.f) : 1.f;
        fp.torchIntensity *= low;
    }
    fp.darkvision = st.darkvision ? 1.f : 0.f;
    fp.post.time = t;
    fp.post.darkvision = m_dark;
    fp.post.hallucination = fp.zone.hallucination;
    fp.post.psychic = fp.zone.psychic;
    fp.post.damageFlash = st.damageFlash; fp.post.flashColor = st.flashColor;
    fp.post.fadeAlpha = st.fadeAlpha; fp.post.fadeColor = st.fadeColor;
    fp.post.underwater = m_under;
    fp.post.exposure = fp.fullbright ? 1.0f : 1.1f;
    return fp;
}

void GameWindow::drawHud(QPainter& p, int w, int h)
{
    setModeFlags();
    m_ui.fps = m_avgDt > 0 ? 1.f / m_avgDt : 0.f; m_ui.frameMs = m_avgDt * 1000.f;
    m_ui.drawCalls = m_renderer.drawCalls; m_ui.tris = m_renderer.triangles; m_ui.chunks = m_renderer.chunkCount();
    QMatrix4x4 vp = m_ui.overview ? QMatrix4x4() : m_world.cam.proj * m_world.cam.view;
    if (m_ui.overview) {
        FrameParams fp = buildFrame(w, h, 0, true);
        vp = fp.proj * fp.view;
    }
    m_hud.paint(p, w, h, m_world, m_player, m_ui, vp);
}

void GameWindow::frame(float dt)
{
    m_avgDt += (dt - m_avgDt) * 0.05f;
    if (m_opt.bench > 0.f) {
        // varredura automatica da camera durante a medicao
        m_player.look(dt * 260.f, 0.f);
        m_benchTime += dt; ++m_benchFrames;
        if (m_benchTime >= m_opt.bench) {
            outLine(QStringLiteral("Bench: %1 quadros em %2 s = %3 fps (%4 ms/quadro), draws %5, tris %6k")
                        .arg(m_benchFrames).arg(m_benchTime, 0, 'f', 1).arg(m_benchFrames / m_benchTime, 0, 'f', 0)
                        .arg(1000.0 * m_benchTime / m_benchFrames, 0, 'f', 2).arg(m_renderer.drawCalls).arg(m_renderer.triangles / 1000));
            QCoreApplication::quit();
        }
    }
    // olhar
    if (m_captured && !menuOpen() && !m_ui.overview && m_opt.bench <= 0.f) {
        m_centerGlobal = mapToGlobal(QPoint(width() / 2, height() / 2));
        QPoint cur = QCursor::pos();
        QPoint d = cur - m_centerGlobal;
        if (d.x() || d.y()) { m_player.look((float)d.x(), (float)d.y()); QCursor::setPos(m_centerGlobal); }
    }
    Player::Input in = gatherInput();
    if (m_ui.overview) in = Player::Input();
    stepSim(dt, in);

    GameState& st = m_world.state;
    m_dark += ((st.darkvision ? 1.f : 0.f) - m_dark) * std::min(1.f, dt * 6.f);
    m_under += ((m_player.eyeUnderwater ? 1.f : 0.f) - m_under) * std::min(1.f, dt * 6.f);

    int pw = (int)std::lround(width() * devicePixelRatio()), ph = (int)std::lround(height() * devicePixelRatio());
    m_world.cam.aspect = (float)pw / std::max(1, ph);
    FrameParams fp = buildFrame(pw, ph, defaultFramebufferObject(), true);
    m_renderer.render(m_world, fp);
    {
        QPainter p(this);
        drawHud(p, width(), height());
    }
    if (m_wantShot) { m_wantShot = false; saveScreenshot(); }
}

void GameWindow::saveScreenshot()
{
    int pw = (int)std::lround(width() * devicePixelRatio()), ph = (int)std::lround(height() * devicePixelRatio());
    QImage img(pw, ph, QImage::Format_RGBA8888);
    GL->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    GL->glReadBuffer(GL_BACK);
    GL->glPixelStorei(GL_PACK_ALIGNMENT, 1);
    GL->glReadPixels(0, 0, pw, ph, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    img = img.mirrored(false, true);
    QString path = QCoreApplication::applicationDirPath() + "/screenshot_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png";
    img.save(path);
    m_world.state.toast(QStringLiteral("Captura salva: ") + QFileInfo(path).fileName(), QColor(200, 255, 200), 3.f);
    outLine("Captura: " + path);
}

// ------------------------------------------------------------ teleportes / notas
void GameWindow::goViewpoint(const Viewpoint& vp)
{
    m_ui.tabMenu = false; m_ui.notesOpen = false; m_ui.help = false;
    if (m_ui.overview) toggleOverview();
    startAt(vp);
    setCaptured(true);
}

void GameWindow::teleportNear(const Vec3& t)
{
    m_ui.tabMenu = false; m_ui.notesOpen = false;
    if (m_ui.overview) toggleOverview();
    const float radii[4] = {2.2f, 3.2f, 1.4f, 4.5f};
    for (float r : radii)
        for (int a = 0; a < 12; ++a) {
            float ang = a * 0.5236f;
            Vec3 c = t + Vec3(std::cos(ang) * r, 0.f, std::sin(ang) * r);
            RayHit h = m_world.raycast(c + Vec3(0, 2.0f, 0), Vec3(0, -1, 0), 8.f);
            if (!h.hit || h.t < 0.5f) continue;
            Vec3 feet = h.pos + Vec3(0, 0.08f, 0);
            if (m_world.solidDistance(feet + Vec3(0, 0.9f, 0)) < 0.5f || m_world.solidDistance(feet + Vec3(0, 1.6f, 0)) < 0.3f) continue;
            if (!m_world.lineOfSight(feet + Vec3(0, 1.6f, 0), t)) continue;
            Vec3 d = t - (feet + Vec3(0, 1.6f, 0));
            float yaw = std::atan2(-d.x(), -d.z()) * 57.2957795f;
            float pitch = std::atan2(d.y(), std::sqrt(d.x() * d.x() + d.z() * d.z())) * 57.2957795f;
            m_player.mode = Player::Mode::Walk;
            m_player.up = Vec3(0, 1, 0);
            m_player.teleport(feet, yaw, pitch);
            setCaptured(true);
            return;
        }
    Viewpoint v; v.pos = t + Vec3(0, 1.65f, 0); v.fly = true;
    startAt(v);
    setCaptured(true);
}

void GameWindow::openNote(const Poi* p)
{
    m_ui.notePoi = p; m_ui.notesOpen = true; m_ui.noteScroll = 0.f; m_ui.tabMenu = false;
    setCaptured(false);
}

void GameWindow::toggleOverview()
{
    m_ui.overview = !m_ui.overview;
    if (m_ui.overview) { setCaptured(false); m_ui.showLabels = true; }
    else { m_ui.showLabels = m_player.mode == Player::Mode::Fly; if (!menuOpen()) setCaptured(true); }
}

// ------------------------------------------------------------ entrada
void GameWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->isAutoRepeat()) return;
    int k = e->key();
    m_keys.insert(k);
    if (m_phase != Phase::Playing) return;
    GameState& st = m_world.state;
    if (k == Qt::Key_Space) m_jumpPressed = true;
    switch (k) {
    case Qt::Key_Escape:
        if (m_ui.help) { m_ui.help = false; setCaptured(true); }
        else if (m_ui.notesOpen) { m_ui.notesOpen = false; setCaptured(true); }
        else if (m_ui.tabMenu) { m_ui.tabMenu = false; setCaptured(true); }
        else setCaptured(false);
        return;
    case Qt::Key_Tab:
        m_ui.tabMenu = !m_ui.tabMenu; m_ui.notesOpen = false; m_ui.help = false;
        setCaptured(!m_ui.tabMenu && !m_ui.overview);
        return;
    case Qt::Key_F1:
        m_ui.help = !m_ui.help; if (m_ui.help) setCaptured(false); else if (!m_ui.overview) setCaptured(true);
        return;
    case Qt::Key_F2:
        m_player.toggleFly(); m_ui.dmMode = m_player.mode == Player::Mode::Fly; m_ui.showLabels = m_ui.dmMode;
        st.toast(m_ui.dmMode ? QStringLiteral("Modo Mestre: voo livre ligado") : QStringLiteral("Modo Mestre desligado"), QColor(255, 210, 120), 2.f);
        return;
    case Qt::Key_F3: m_ui.fullbright = !m_ui.fullbright; return;
    case Qt::Key_F4: m_ui.showLabels = !m_ui.showLabels; return;
    case Qt::Key_F5: m_debugDraw = !m_debugDraw; return;
    case Qt::Key_F6: m_wire = !m_wire; return;
    case Qt::Key_F7:
        m_clip = !m_clip;
        if (m_clip) { Vec3 p = m_player.eyePos(); m_clipPos = m_clipAxis == 0 ? p.x() : (m_clipAxis == 1 ? p.y() : p.z()); }
        return;
    case Qt::Key_F8: m_clipAxis = (m_clipAxis + 1) % 3; { Vec3 p = m_player.eyePos(); m_clipPos = m_clipAxis == 0 ? p.x() : (m_clipAxis == 1 ? p.y() : p.z()); } return;
    case Qt::Key_BracketLeft: m_clipPos -= (e->modifiers() & Qt::ShiftModifier) ? 10.f : 1.f; return;
    case Qt::Key_BracketRight: m_clipPos += (e->modifiers() & Qt::ShiftModifier) ? 10.f : 1.f; return;
    case Qt::Key_F9: m_terrainMode = (m_terrainMode + 1) % 3; return;
    case Qt::Key_F10: m_ui.showStats = !m_ui.showStats; return;
    case Qt::Key_F12: m_wantShot = true; return;
    case Qt::Key_O: toggleOverview(); return;
    case Qt::Key_R: if (!menuOpen()) st.lightTorch(); return;
    case Qt::Key_V:
        if (!menuOpen()) { st.darkvision = !st.darkvision; st.toast(st.darkvision ? QStringLiteral("Visão no escuro: ligada (18 m, tons de cinza)") : QStringLiteral("Visão no escuro: desligada"), QColor(200, 220, 200), 2.f); }
        return;
    case Qt::Key_E:
        if (m_ui.notesOpen) { m_ui.notesOpen = false; setCaptured(true); }
        else if (!menuOpen() && !m_ui.overview) {
            if (Interactable* in = m_world.focusedInteractable()) { if (in->callback) in->callback(); }
            else if (Poi* p = m_world.focusedPoi()) openNote(p);
        }
        return;
    default: break;
    }
    if (!menuOpen() && !(e->modifiers() & (Qt::ControlModifier | Qt::AltModifier)))
        for (const GameEvent& ev : m_world.events())
            if (ev.hotkey && ev.hotkey == k && ev.fn) { ev.fn(); return; }
}

void GameWindow::keyReleaseEvent(QKeyEvent* e) { if (!e->isAutoRepeat()) m_keys.remove(e->key()); }

void GameWindow::mousePressEvent(QMouseEvent* e)
{
    if (m_phase != Phase::Playing) return;
    if (e->button() == Qt::LeftButton) {
        if (m_ui.overview) { m_ovDrag = true; m_ovLast = e->pos(); m_ovPress = e->pos(); }
        else if (!m_captured && !menuOpen()) setCaptured(true);
    }
}

void GameWindow::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_phase != Phase::Playing || e->button() != Qt::LeftButton) return;
    bool wasDrag = m_ovDrag && (e->pos() - m_ovPress).manhattanLength() > 5;
    m_ovDrag = false;
    if (wasDrag) return;
    if (menuOpen() || m_ui.overview) m_hud.click(e->localPos());
}

void GameWindow::mouseMoveEvent(QMouseEvent* e)
{
    if (m_ovDrag && m_ui.overview) {
        QPoint d = e->pos() - m_ovLast; m_ovLast = e->pos();
        m_ovYaw -= d.x() * 0.4f;
        m_ovPitch = clampf(m_ovPitch + d.y() * 0.4f, -89.f, 89.f);
    }
}

void GameWindow::wheelEvent(QWheelEvent* e)
{
    int d = e->angleDelta().y();
    if (menuOpen()) { m_hud.wheel(e->position(), d, m_ui); return; }
    if (m_ui.overview) { m_ovDist = clampf(m_ovDist * std::exp(-d * 0.0012f), 20.f, 6000.f); return; }
    if (m_player.mode == Player::Mode::Fly) m_player.flySpeedScale = clampf(m_player.flySpeedScale * std::pow(1.15f, d / 120.f), 0.05f, 40.f);
}

void GameWindow::focusOutEvent(QFocusEvent*) { m_keys.clear(); if (m_phase == Phase::Playing) setCaptured(false); }

// ------------------------------------------------------------ --shots
void GameWindow::prepareShots()
{
    QDir().mkpath(m_opt.shotsDir);
    if (m_opt.shotsMode == "ui") {   // menus e paineis (teste interno)
        Viewpoint v = m_world.viewpoints().size() > 2 ? m_world.viewpoints()[2] : Viewpoint();
        const char* names[5] = {"tab_areas", "tab_notas", "tab_eventos", "notas", "ajuda"};
        for (int i = 0; i < 5; ++i) { ShotJob j{v, QStringLiteral("ui_%1.png").arg(QLatin1String(names[i])), false}; j.ui = i + 1; m_jobs.push_back(j); }
        return;
    }
    if (m_opt.shotsMode == "overview") {   // vista geral (teste interno)
        Viewpoint v = m_world.viewpoints().empty() ? Viewpoint() : m_world.viewpoints().front();
        const float ys[3] = {35.f, 130.f, 250.f};
        for (int i = 0; i < 3; ++i) { ShotJob j{v, QStringLiteral("overview_%1.png").arg(i), true}; j.ov = true; j.yaw = ys[i]; j.pitch = i == 1 ? 60.f : 30.f; m_jobs.push_back(j); }
        return;
    }
    int n = 0;
    for (const Viewpoint& v : m_world.viewpoints()) {
        ++n;   // numeracao estavel (indice do ponto de vista), mesmo com --filter
        if (!m_opt.filter.isEmpty() && !v.name.contains(m_opt.filter, Qt::CaseInsensitive)) continue;
        QString base = QStringLiteral("%1_%2").arg(n, 2, 10, QChar('0')).arg(sanitize(v.name));
        if (m_opt.shotsMode != "dm") m_jobs.push_back({v, base + ".png", false});
        if (m_opt.shotsMode != "explore") m_jobs.push_back({v, base + "_mestre.png", true});
    }
    if (m_jobs.empty()) outLine(QStringLiteral("Nenhum ponto de vista corresponde ao filtro."));
}

void GameWindow::shotStep()
{
    if (m_jobIndex >= m_jobs.size()) {
        outLine(QStringLiteral("Capturas (%1):").arg(m_shotFiles.size()));
        for (const QString& f : m_shotFiles) outLine("  " + f);
        if (m_shotCount) outLine(QStringLiteral("Tempo médio de render por captura: %1 ms (GPU+CPU, 1 quadro)").arg(m_shotRenderMs / m_shotCount, 0, 'f', 1));
        QCoreApplication::quit();
        return;
    }
    const ShotJob job = m_jobs[m_jobIndex++];
    int W = m_opt.width > 0 ? m_opt.width : 1600, H = m_opt.height > 0 ? m_opt.height : 900;
    GameState& st = m_world.state;
    m_ui = UiState(); m_ui.onViewpoint = nullptr;
    m_ui.fullbright = job.dm; m_ui.showLabels = job.dm; m_ui.dmMode = job.dm; m_ui.overview = job.ov; m_ovYaw = job.yaw; m_ovPitch = job.pitch;
    st.torchLit = !job.dm; st.torchSeconds = GameState::kTorchDuration; st.darkvision = false;
    st.toasts.clear(); st.hintAge = 99.f; st.bannerAge = 99.f; st.fadeAlpha = 0.f; st.fadeTarget = 0.f; st.damageFlash = 0.f;
    m_world.cam.aspect = (float)W / H;
    startAt(job.vp);
    m_world.cam.aspect = (float)W / H;
    // ~1,5 s de simulacao
    for (int i = 0; i < 90; ++i) stepSim(1.f / 60.f, Player::Input());
    m_dark = 0.f; m_under = m_player.eyeUnderwater ? 1.f : 0.f;
    if (job.ui >= 1 && job.ui <= 3) { m_ui.tabMenu = true; m_ui.tab = job.ui - 1; }
    if (job.ui == 4 && !m_world.pois().empty()) { m_ui.notesOpen = true; m_ui.notePoi = m_world.pois()[6].get(); }
    if (job.ui == 5) m_ui.help = true;
    st.bannerAge = 0.9f;
    QOpenGLFramebufferObject fbo(W, H);
    QElapsedTimer tm; tm.start();
    FrameParams fp = buildFrame(W, H, fbo.handle(), false);
    m_renderer.render(m_world, fp);
    GL->glFinish();
    m_shotRenderMs += tm.nsecsElapsed() * 1e-6; ++m_shotCount;
    QImage img = fbo.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    {
        QPainter p(&img);
        QMatrix4x4 vp = fp.proj * fp.view;
        m_ui.showStats = false;
        m_hud.paint(p, W, H, m_world, m_player, m_ui, vp);
    }
    QString path = QDir(m_opt.shotsDir).absoluteFilePath(job.file);
    img.save(path);
    m_shotFiles << path;
    // restaura a tela padrao (janela ainda e apresentada)
    GL->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    GL->glClearColor(0, 0, 0, 1); GL->glClear(GL_COLOR_BUFFER_BIT);
}
