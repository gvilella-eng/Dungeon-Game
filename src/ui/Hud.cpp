#include "Hud.h"
#include <QKeySequence>
#include <QPainterPath>
#include <QLinearGradient>

QFont Hud::font(double px, bool bold) const
{
    QFont f(QStringLiteral("Segoe UI"));
    f.setPixelSize(std::max(8, (int)std::lround(px * m_s)));
    f.setBold(bold);
    return f;
}

void Hud::text(QPainter& p, const QRectF& r, const QString& s, double px, const QColor& c, int flags, bool bold, bool shadow)
{
    p.setFont(font(px, bold));
    if (shadow) { p.setPen(QColor(0, 0, 0, std::min(200, c.alpha()))); p.drawText(r.translated(1.5 * m_s, 1.5 * m_s), flags, s); }
    p.setPen(c);
    p.drawText(r, flags, s);
}

bool Hud::click(const QPointF& pos)
{
    for (auto it = m_hits.rbegin(); it != m_hits.rend(); ++it)
        if (it->r.contains(pos)) { if (it->fn) it->fn(); return true; }
    return false;
}

bool Hud::wheel(const QPointF&, int delta, UiState& ui)
{
    if (ui.notesOpen) { ui.noteScroll -= delta * 0.5f; if (ui.noteScroll < 0) ui.noteScroll = 0; return true; }
    if (ui.tabMenu) { ui.scroll[ui.tab] -= delta * 0.5f; if (ui.scroll[ui.tab] < 0) ui.scroll[ui.tab] = 0; return true; }
    return false;
}

void Hud::paintLoading(QPainter& p, int w, int h, float progress, const QString& status, const QString& detail)
{
    m_s = std::max(0.7, h / 1080.0);
    QLinearGradient g(0, 0, 0, h); g.setColorAt(0, QColor(10, 8, 12)); g.setColorAt(1, QColor(26, 14, 10));
    p.fillRect(0, 0, w, h, g);
    text(p, QRectF(0, h * 0.30, w, 80 * m_s), QStringLiteral("A Chama Primordial"), 54, QColor(255, 200, 130), Qt::AlignCenter, true);
    text(p, QRectF(0, h * 0.30 + 70 * m_s, w, 40 * m_s), QStringLiteral("Masmorra de D&D · Explorador 3D"), 20, QColor(180, 160, 140), Qt::AlignCenter);
    double bw = std::min(w * 0.5, 700.0 * m_s), bh = 14 * m_s, bx = (w - bw) / 2, by = h * 0.58;
    p.setPen(Qt::NoPen); p.setBrush(QColor(255, 255, 255, 30)); p.drawRoundedRect(QRectF(bx, by, bw, bh), 7, 7);
    QLinearGradient bg(bx, 0, bx + bw, 0); bg.setColorAt(0, QColor(255, 120, 40)); bg.setColorAt(1, QColor(255, 210, 100));
    p.setBrush(bg); p.drawRoundedRect(QRectF(bx, by, bw * std::max(0.02f, std::min(1.f, progress)), bh), 7, 7);
    text(p, QRectF(0, by + 24 * m_s, w, 30 * m_s), status, 18, QColor(220, 210, 200), Qt::AlignCenter);
    if (!detail.isEmpty()) text(p, QRectF(0, by + 54 * m_s, w, 26 * m_s), detail, 14, QColor(150, 140, 130), Qt::AlignCenter);
}

void Hud::drawCompass(QPainter& p, int w, int, Player& pl)
{
    double hdg = std::fmod(-pl.yawDeg() + 720.0, 360.0);
    double cw = 440 * m_s, cx = w / 2.0, y = 16 * m_s, pxd = cw / 120.0;
    p.save();
    p.setClipRect(QRectF(cx - cw / 2, y - 4, cw, 46 * m_s));
    QLinearGradient fade(cx - cw / 2, 0, cx + cw / 2, 0);
    fade.setColorAt(0, QColor(255, 255, 255, 0)); fade.setColorAt(0.3, QColor(255, 255, 255, 200)); fade.setColorAt(0.7, QColor(255, 255, 255, 200)); fade.setColorAt(1, QColor(255, 255, 255, 0));
    p.setPen(QPen(QBrush(fade), 1.5));
    static const char* names[8] = {"N", "NE", "L", "SE", "S", "SO", "O", "NO"};
    for (int a = (int)std::floor(hdg - 62); a <= (int)std::ceil(hdg + 62); ++a) {
        if (a % 5) continue;
        double x = cx + (a - hdg) * pxd;
        int an = ((a % 360) + 360) % 360;
        bool big = an % 45 == 0;
        p.setPen(QPen(QColor(255, 255, 255, big ? 220 : 110), 1.5));
        p.drawLine(QPointF(x, y + 20 * m_s), QPointF(x, y + (big ? 12 : 16) * m_s));
        if (big) text(p, QRectF(x - 20 * m_s, y - 4 * m_s, 40 * m_s, 18 * m_s), QString::fromLatin1(names[an / 45]), an % 90 == 0 ? 15 : 12, an == 0 ? QColor(255, 120, 100) : QColor(235, 235, 235), Qt::AlignCenter, an % 90 == 0);
    }
    p.restore();
    p.setPen(Qt::NoPen); p.setBrush(QColor(255, 200, 100));
    QPolygonF tri; tri << QPointF(cx - 5 * m_s, y + 34 * m_s) << QPointF(cx + 5 * m_s, y + 34 * m_s) << QPointF(cx, y + 26 * m_s);
    p.drawPolygon(tri);
    text(p, QRectF(cx - 40 * m_s, y + 34 * m_s, 80 * m_s, 18 * m_s), QStringLiteral("%1°").arg((int)hdg), 12, QColor(210, 210, 210), Qt::AlignCenter);
}

void Hud::drawBars(QPainter& p, int w, int h, World& world, Player& pl, UiState& ui)
{
    const GameState& st = world.state;
    const ZoneState& zs = world.zoneState();
    double x0 = 28 * m_s, y0 = h - 28 * m_s;
    // tocha
    {
        QRectF r(x0, y0 - 44 * m_s, 300 * m_s, 40 * m_s);
        p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, 110)); p.drawRoundedRect(r, 8, 8);
        // chama
        QPainterPath f; double fx = r.left() + 22 * m_s, fy = r.center().y() + 11 * m_s;
        f.moveTo(fx, fy - 24 * m_s); f.cubicTo(fx + 11 * m_s, fy - 10 * m_s, fx + 9 * m_s, fy, fx, fy); f.cubicTo(fx - 9 * m_s, fy, fx - 11 * m_s, fy - 10 * m_s, fx, fy - 24 * m_s);
        p.setBrush(st.torchLit ? QColor(255, 170, 60) : QColor(90, 90, 90)); p.drawPath(f);
        p.setBrush(QColor(110, 70, 40)); p.drawRect(QRectF(fx - 2.5 * m_s, fy, 5 * m_s, 8 * m_s));
        int secs = (int)st.torchSeconds;
        QString t = st.torchLit ? QStringLiteral("%1:%2").arg(secs / 60, 2, 10, QChar('0')).arg(secs % 60, 2, 10, QChar('0')) : (st.torchSeconds < GameState::kTorchDuration - 1 && st.torchSeconds > 0 ? QStringLiteral("apagada") : QStringLiteral("sem tocha"));
        QColor tc = !st.torchLit ? QColor(160, 160, 170) : (st.torchSeconds < 300 ? QColor(255, 120, 90) : QColor(255, 225, 170));
        text(p, QRectF(r.left() + 48 * m_s, r.top(), 170 * m_s, r.height()), t, 20, tc, Qt::AlignLeft | Qt::AlignVCenter, true);
        text(p, QRectF(r.left() + 215 * m_s, r.top(), 80 * m_s, r.height()), QStringLiteral("× %1").arg(st.torchesInInventory), 18, QColor(220, 210, 190), Qt::AlignLeft | Qt::AlignVCenter);
    }
    // exaustao
    if (true) {
        QRectF r(x0, y0 - 84 * m_s, 300 * m_s, 32 * m_s);
        p.setBrush(QColor(0, 0, 0, 90)); p.setPen(Qt::NoPen); p.drawRoundedRect(r, 8, 8);
        text(p, r.adjusted(14 * m_s, 0, 0, 0), QStringLiteral("Exaustão: %1").arg(st.exhaustion), 17, st.exhaustion ? QColor(255, 170, 90) : QColor(200, 200, 200));
    }
    // canto superior direito: temperatura / profundidade
    {
        double depth = -pl.pos.y();
        if (std::fabs(depth) < 0.5) depth = 0;
        QString d = depth >= 0 ? QStringLiteral("Profundidade: %1 m (%2 ft)").arg(depth, 0, 'f', 0).arg(depth * 3.281, 0, 'f', 0)
                               : QStringLiteral("Altitude: +%1 m (%2 ft)").arg(-depth, 0, 'f', 0).arg(-depth * 3.281, 0, 'f', 0);
        QRectF r(w - 340 * m_s, 18 * m_s, 320 * m_s, 30 * m_s);
        text(p, r, d, 17, QColor(225, 225, 225), Qt::AlignRight | Qt::AlignVCenter);
        QColor tc = zs.temperature < 0 ? QColor(150, 200, 255) : (zs.temperature > 30 ? QColor(255, 150, 90) : QColor(230, 230, 230));
        text(p, r.translated(0, 28 * m_s), QStringLiteral("%1 °C").arg(zs.temperature, 0, 'f', 0), 22, tc, Qt::AlignRight | Qt::AlignVCenter, true);
    }
    // medidores customizados + folego (centro inferior)
    {
        double bw = 320 * m_s, bh = 14 * m_s, cx = w / 2.0, y = h - 46 * m_s;
        auto bar = [&](const QString& name, float v, const QColor& c) {
            p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, 130)); p.drawRoundedRect(QRectF(cx - bw / 2 - 2, y - 2, bw + 4, bh + 4), 5, 5);
            p.setBrush(c); p.drawRoundedRect(QRectF(cx - bw / 2, y, bw * clampf(v, 0.f, 1.f), bh), 4, 4);
            text(p, QRectF(cx - bw / 2, y - 20 * m_s, bw, 18 * m_s), name, 13, QColor(225, 225, 225), Qt::AlignCenter);
            y -= 40 * m_s;
        };
        if (st.breathVisible) bar(QStringLiteral("Fôlego"), st.breath / st.maxBreath, QColor(90, 170, 255));
        for (const auto& m : st.meters) bar(m.name, m.value, m.color);
    }
    // indicadores de modo
    {
        QStringList fl = ui.modeFlags;
        if (pl.mode == Player::Mode::Fly) fl.prepend(QStringLiteral("MODO MESTRE · voo livre (%1 m/s)").arg(pl.flyBase * pl.flySpeedScale, 0, 'f', 0));
        if (ui.overview) fl.prepend(QStringLiteral("VISTA GERAL · arraste: girar · roda: zoom · clique no rótulo: teleportar"));
        double y = 18 * m_s;
        for (const QString& s : fl) { text(p, QRectF(18 * m_s, y, 700 * m_s, 22 * m_s), s, 14, QColor(255, 210, 120)); y += 20 * m_s; }
        if (ui.showStats)
            text(p, QRectF(18 * m_s, y + 4 * m_s, 700 * m_s, 22 * m_s),
                 QStringLiteral("%1 fps · %2 ms · %3 draws · %4k tris · %5 chunks").arg(ui.fps, 0, 'f', 0).arg(ui.frameMs, 0, 'f', 2).arg(ui.drawCalls).arg((double)ui.tris / 1000.0, 0, 'f', 0).arg((qulonglong)ui.chunks), 13, QColor(160, 255, 160));
    }
}

void Hud::drawToasts(QPainter& p, int w, int h, World& world)
{
    double y = h * 0.74;
    for (auto it = world.state.toasts.rbegin(); it != world.state.toasts.rend(); ++it) {
        float a = 1.f;
        if (it->age < 0.15f) a = it->age / 0.15f;
        if (it->age > it->duration - 0.7f) a = std::max(0.f, (it->duration - it->age) / 0.7f);
        QColor c = it->color; c.setAlphaF(a);
        p.setFont(font(21, true));
        QRectF br = p.boundingRect(QRectF(0, 0, w * 0.8, 100), Qt::AlignCenter, it->text);
        QRectF r(w / 2.0 - br.width() / 2 - 16 * m_s, y - br.height() / 2 - 6 * m_s, br.width() + 32 * m_s, br.height() + 12 * m_s);
        p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, (int)(120 * a))); p.drawRoundedRect(r, 8, 8);
        text(p, r, it->text, 21, c, Qt::AlignCenter, true);
        y += 44 * m_s;
    }
}

void Hud::drawBanner(QPainter& p, int w, int h, World& world)
{
    const GameState& st = world.state;
    if (st.bannerAge > 6.f || st.bannerTitle.isEmpty()) return;
    float a = st.bannerAge < 0.8f ? st.bannerAge / 0.8f : (st.bannerAge > 4.6f ? std::max(0.f, (6.f - st.bannerAge) / 1.4f) : 1.f);
    text(p, QRectF(0, h * 0.14, w, 70 * m_s), st.bannerTitle, 46, QColor(255, 228, 180, (int)(255 * a)), Qt::AlignCenter, true);
    if (!st.bannerSub.isEmpty()) text(p, QRectF(0, h * 0.14 + 62 * m_s, w, 30 * m_s), st.bannerSub, 20, QColor(220, 200, 170, (int)(230 * a)), Qt::AlignCenter);
}

void Hud::drawLabels(QPainter& p, int w, int h, World& world, UiState& ui, const QMatrix4x4& vp, Player& pl)
{
    for (const auto& poi : world.pois()) {
        QVector4D c = vp * QVector4D(poi->pos, 1.f);
        if (c.w() <= 0.1f) continue;
        double sx = (c.x() / c.w() * 0.5 + 0.5) * w, sy = (1.0 - (c.y() / c.w() * 0.5 + 0.5)) * h;
        if (sx < -50 || sx > w + 50 || sy < -20 || sy > h + 20) continue;
        float dist = (poi->pos - pl.eyePos()).length();
        if (!ui.overview && dist > 220.f) continue;
        QColor col = poiCategoryColor(poi->category);
        p.setFont(font(13, true));
        QRectF tr = p.boundingRect(QRectF(0, 0, 400, 30), Qt::AlignLeft, poi->title);
        QRectF r(sx - tr.width() / 2 - 8 * m_s, sy - 12 * m_s, tr.width() + 16 * m_s, 22 * m_s);
        p.setPen(QPen(col, 1.5)); QColor bg = col.darker(400); bg.setAlpha(190); p.setBrush(bg); p.drawRoundedRect(r, 6, 6);
        p.setPen(Qt::NoPen); p.setBrush(col); p.drawEllipse(QPointF(sx, sy + 17 * m_s), 3.5 * m_s, 3.5 * m_s);
        text(p, r, poi->title, 13, QColor(255, 255, 255), Qt::AlignCenter, true, false);
        const Poi* pp = poi.get();
        if (ui.overview) m_hits.push_back({r, [&ui, pp]() { if (ui.onPoiGo) ui.onPoiGo(pp); }});
    }
}

void Hud::drawTabMenu(QPainter& p, int w, int h, World& world, UiState& ui)
{
    p.fillRect(0, 0, w, h, QColor(0, 0, 0, 150));
    double pw = std::min(w - 60.0, 1100.0 * m_s), ph = std::min(h - 60.0, 760.0 * m_s);
    QRectF panel((w - pw) / 2, (h - ph) / 2, pw, ph);
    p.setPen(QPen(QColor(255, 190, 110, 120), 1.5)); p.setBrush(QColor(18, 14, 14, 240)); p.drawRoundedRect(panel, 12, 12);
    m_hits.push_back({panel, nullptr});
    static const QStringList tabs = {QStringLiteral("Áreas"), QStringLiteral("Notas do Mestre"), QStringLiteral("Eventos")};
    double tx = panel.left() + 20 * m_s;
    for (int i = 0; i < 3; ++i) {
        QRectF r(tx, panel.top() + 14 * m_s, 190 * m_s, 38 * m_s);
        p.setPen(Qt::NoPen); p.setBrush(i == ui.tab ? QColor(255, 160, 70, 220) : QColor(255, 255, 255, 25)); p.drawRoundedRect(r, 8, 8);
        text(p, r, tabs[i], 18, i == ui.tab ? QColor(20, 12, 8) : QColor(230, 220, 210), Qt::AlignCenter, true, false);
        m_hits.push_back({r, [&ui, i]() { ui.tab = i; }});
        tx += 200 * m_s;
    }
    text(p, QRectF(panel.right() - 330 * m_s, panel.top() + 14 * m_s, 310 * m_s, 38 * m_s), QStringLiteral("Tab / Esc: fechar"), 14, QColor(170, 160, 150), Qt::AlignRight | Qt::AlignVCenter);

    QRectF area(panel.left() + 20 * m_s, panel.top() + 66 * m_s, panel.width() - 40 * m_s, panel.height() - 86 * m_s);
    p.save(); p.setClipRect(area);
    double y = area.top() - ui.scroll[ui.tab];
    double rowH = 36 * m_s;
    auto heading = [&](const QString& s) {
        text(p, QRectF(area.left(), y, area.width(), 30 * m_s), s, 16, QColor(255, 190, 110), Qt::AlignLeft | Qt::AlignVCenter, true);
        p.setPen(QPen(QColor(255, 190, 110, 60), 1)); p.drawLine(QPointF(area.left(), y + 30 * m_s), QPointF(area.right(), y + 30 * m_s));
        y += 36 * m_s;
    };
    if (ui.tab == 0) {
        QStringList order; QMap<QString, std::vector<Viewpoint>> groups;
        for (const Viewpoint& v : world.viewpoints()) { QString g = v.group.isEmpty() ? QStringLiteral("Pontos de vista") : v.group; if (!groups.contains(g)) order << g; groups[g].push_back(v); }
        for (const QString& g : order) {
            heading(g);
            double colW = area.width() / 2.0 - 6 * m_s;
            int i = 0;
            for (const Viewpoint& v : groups[g]) {
                QRectF r(area.left() + (i % 2) * (colW + 12 * m_s), y + (i / 2) * (rowH + 4 * m_s), colW, rowH);
                bool hov = false; (void)hov;
                p.setPen(Qt::NoPen); p.setBrush(QColor(255, 255, 255, 22)); p.drawRoundedRect(r, 6, 6);
                text(p, r.adjusted(12 * m_s, 0, -8 * m_s, 0), v.name + (v.fly ? QStringLiteral("  ✈") : QString()), 16, QColor(235, 230, 225), Qt::AlignLeft | Qt::AlignVCenter, false, false);
                Viewpoint vc = v;
                if (r.intersects(area)) m_hits.push_back({r, [&ui, vc]() { if (ui.onViewpoint) ui.onViewpoint(vc); }});
                ++i;
            }
            y += ((i + 1) / 2) * (rowH + 4 * m_s) + 10 * m_s;
        }
        if (order.isEmpty()) text(p, area, QStringLiteral("Nenhum ponto de vista cadastrado."), 16, QColor(180, 180, 180), Qt::AlignCenter);
    } else if (ui.tab == 1) {
        QStringList order; QMap<QString, std::vector<const Poi*>> groups;
        for (const auto& poi : world.pois()) {
            QString g = !poi->area.isEmpty() ? poi->area : world.areaAt(poi->pos);
            if (g.isEmpty()) g = QStringLiteral("Sem área");
            if (!groups.contains(g)) order << g;
            groups[g].push_back(poi.get());
        }
        for (const QString& g : order) {
            heading(g);
            for (const Poi* poi : groups[g]) {
                QRectF r(area.left(), y, area.width(), rowH);
                p.setPen(Qt::NoPen); p.setBrush(QColor(255, 255, 255, 18)); p.drawRoundedRect(r, 6, 6);
                QColor c = poiCategoryColor(poi->category);
                p.setBrush(c); p.drawEllipse(QPointF(r.left() + 16 * m_s, r.center().y()), 6 * m_s, 6 * m_s);
                text(p, QRectF(r.left() + 32 * m_s, r.top(), r.width() * 0.55, rowH), poi->title, 16, QColor(240, 235, 230), Qt::AlignLeft | Qt::AlignVCenter, false, false);
                text(p, QRectF(r.left() + r.width() * 0.55, r.top(), r.width() * 0.3, rowH), poiCategoryName(poi->category) + (poi->session.isEmpty() ? QString() : " · " + poi->session), 13, c, Qt::AlignLeft | Qt::AlignVCenter, false, false);
                QRectF go(r.right() - 60 * m_s, r.top() + 4 * m_s, 52 * m_s, rowH - 8 * m_s);
                p.setPen(Qt::NoPen); p.setBrush(QColor(255, 160, 70, 200)); p.drawRoundedRect(go, 5, 5);
                text(p, go, QStringLiteral("Ir"), 14, QColor(20, 12, 8), Qt::AlignCenter, true, false);
                if (r.intersects(area)) {
                    m_hits.push_back({r, [&ui, poi]() { if (ui.onPoiOpen) ui.onPoiOpen(poi); }});
                    m_hits.push_back({go, [&ui, poi]() { if (ui.onPoiGo) ui.onPoiGo(poi); }});
                }
                y += rowH + 4 * m_s;
            }
            y += 8 * m_s;
        }
        if (order.isEmpty()) text(p, area, QStringLiteral("Nenhuma nota do Mestre cadastrada."), 16, QColor(180, 180, 180), Qt::AlignCenter);
    } else {
        for (const GameEvent& e : world.events()) {
            QRectF r(area.left(), y, area.width(), rowH + 4 * m_s);
            p.setPen(Qt::NoPen); p.setBrush(QColor(255, 255, 255, 22)); p.drawRoundedRect(r, 6, 6);
            text(p, r.adjusted(14 * m_s, 0, 0, 0), e.label, 17, QColor(240, 235, 230), Qt::AlignLeft | Qt::AlignVCenter, false, false);
            if (e.hotkey) text(p, r.adjusted(0, 0, -16 * m_s, 0), QStringLiteral("[%1]").arg(QKeySequence(e.hotkey).toString()), 15, QColor(255, 190, 110), Qt::AlignRight | Qt::AlignVCenter, true, false);
            QString id = e.id;
            if (r.intersects(area)) m_hits.push_back({r, [&ui, id]() { if (ui.onEvent) ui.onEvent(id); }});
            y += rowH + 10 * m_s;
        }
        if (world.events().empty()) text(p, area, QStringLiteral("Nenhum evento registrado."), 16, QColor(180, 180, 180), Qt::AlignCenter);
    }
    double contentH = y + ui.scroll[ui.tab] - area.top();
    float maxScroll = (float)std::max(0.0, contentH - area.height());
    if (ui.scroll[ui.tab] > maxScroll) ui.scroll[ui.tab] = maxScroll;
    p.restore();
}

void Hud::drawNotes(QPainter& p, int w, int h, World& world, UiState& ui)
{
    const Poi* poi = ui.notePoi;
    if (!poi) return;
    p.fillRect(0, 0, w, h, QColor(0, 0, 0, 140));
    m_hits.push_back({QRectF(0, 0, w, h), [&ui]() { ui.notesOpen = false; }});
    double pw = std::min(w - 60.0, 800.0 * m_s), ph = std::min(h - 80.0, 640.0 * m_s);
    QRectF panel((w - pw) / 2, (h - ph) / 2, pw, ph);
    QColor c = poiCategoryColor(poi->category);
    p.setPen(QPen(c, 2)); p.setBrush(QColor(20, 16, 14, 245)); p.drawRoundedRect(panel, 12, 12);
    m_hits.push_back({panel, nullptr});
    text(p, QRectF(panel.left() + 28 * m_s, panel.top() + 16 * m_s, panel.width() - 56 * m_s, 20 * m_s), QStringLiteral("NOTAS DO MESTRE · ") + poiCategoryName(poi->category).toUpper(), 13, c, Qt::AlignLeft | Qt::AlignVCenter, true);
    text(p, QRectF(panel.left() + 28 * m_s, panel.top() + 38 * m_s, panel.width() - 56 * m_s, 48 * m_s), poi->title, 30, QColor(255, 236, 205), Qt::AlignLeft | Qt::AlignVCenter, true);
    QString sub = poi->session;
    QString ar = !poi->area.isEmpty() ? poi->area : world.areaAt(poi->pos);
    if (!ar.isEmpty()) sub = sub.isEmpty() ? ar : ar + " · " + sub;
    if (!sub.isEmpty()) text(p, QRectF(panel.left() + 28 * m_s, panel.top() + 86 * m_s, panel.width() - 56 * m_s, 24 * m_s), sub, 15, QColor(190, 180, 170), Qt::AlignLeft | Qt::AlignVCenter);
    QRectF body(panel.left() + 28 * m_s, panel.top() + 122 * m_s, panel.width() - 56 * m_s, panel.height() - 170 * m_s);
    p.setFont(font(18));
    QRectF need = p.boundingRect(QRectF(0, 0, body.width(), 100000), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, poi->text);
    float maxS = (float)std::max(0.0, need.height() - body.height());
    if (ui.noteScroll > maxS) ui.noteScroll = maxS;
    p.save(); p.setClipRect(body);
    text(p, QRectF(body.left(), body.top() - ui.noteScroll, body.width(), need.height() + 10), poi->text, 18, QColor(235, 228, 220), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, false, false);
    p.restore();
    text(p, QRectF(panel.left(), panel.bottom() - 36 * m_s, panel.width(), 28 * m_s), maxS > 0 ? QStringLiteral("Roda do mouse: rolar · E / Esc / clique fora: fechar") : QStringLiteral("E / Esc / clique fora: fechar"), 13, QColor(160, 150, 140), Qt::AlignCenter);
}

void Hud::drawHelp(QPainter& p, int w, int h)
{
    p.fillRect(0, 0, w, h, QColor(0, 0, 0, 170));
    double pw = std::min(w - 60.0, 980.0 * m_s), ph = std::min(h - 60.0, 760.0 * m_s);
    QRectF panel((w - pw) / 2, (h - ph) / 2, pw, ph);
    p.setPen(QPen(QColor(255, 190, 110, 140), 1.5)); p.setBrush(QColor(18, 14, 14, 245)); p.drawRoundedRect(panel, 12, 12);
    text(p, QRectF(panel.left(), panel.top() + 14 * m_s, panel.width(), 44 * m_s), QStringLiteral("Ajuda · Teclas"), 28, QColor(255, 210, 150), Qt::AlignCenter, true);
    struct K { const char* k; const char* d; };
    static const K keys[] = {
        {"W A S D", "mover"}, {"Mouse", "olhar"}, {"Espaço", "pular (na água: subir)"}, {"Ctrl", "agachar (na água: descer)"},
        {"Shift", "correr"}, {"E", "interagir / Notas do Mestre"}, {"R", "acender uma nova tocha"}, {"V", "visão no escuro"},
        {"Tab", "menu: Áreas · Notas do Mestre · Eventos"}, {"F1", "esta ajuda"}, {"F2", "Modo Mestre (voo livre / noclip)"},
        {"F3", "Luz do Mestre (tudo iluminado)"}, {"F4", "rótulos dos POIs"}, {"F5", "mostrar zonas / gatilhos / colisores"},
        {"F6", "wireframe"}, {"F7", "plano de corte  ( [ ]  move · F8 troca o eixo )"}, {"F9", "esconder terreno / raio-x"},
        {"O", "vista geral (arraste: girar · roda: zoom · clique no rótulo)"}, {"F10", "estatísticas de desempenho"},
        {"F12", "captura de tela"}, {"Esc", "soltar o mouse / fechar painéis"}, {"Roda do mouse", "velocidade do voo (Modo Mestre)"},
        {"G K J U Y N B X", "reservadas para eventos do Mestre (veja a aba Eventos)"},
    };
    double y = panel.top() + 74 * m_s, rh = 28.5 * m_s;
    for (const K& k : keys) {
        text(p, QRectF(panel.left() + 36 * m_s, y, 240 * m_s, rh), QString::fromUtf8(k.k), 17, QColor(255, 190, 110), Qt::AlignLeft | Qt::AlignVCenter, true, false);
        text(p, QRectF(panel.left() + 290 * m_s, y, panel.width() - 320 * m_s, rh), QString::fromUtf8(k.d), 17, QColor(235, 230, 225), Qt::AlignLeft | Qt::AlignVCenter, false, false);
        y += rh;
    }
    text(p, QRectF(panel.left(), panel.bottom() - 36 * m_s, panel.width(), 28 * m_s), QStringLiteral("F1 ou Esc: fechar"), 14, QColor(160, 150, 140), Qt::AlignCenter);
}

void Hud::paint(QPainter& p, int w, int h, World& world, Player& pl, UiState& ui, const QMatrix4x4& vp)
{
    m_s = std::max(0.7, h / 1080.0);
    m_hits.clear();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    if (!ui.overview) {
        // mira
        p.setPen(Qt::NoPen); p.setBrush(QColor(255, 255, 255, 190)); p.drawEllipse(QPointF(w / 2.0, h / 2.0), 2.6 * m_s, 2.6 * m_s);
        p.setBrush(QColor(0, 0, 0, 90)); p.drawEllipse(QPointF(w / 2.0, h / 2.0), 4.0 * m_s, 4.0 * m_s);
        p.setBrush(QColor(255, 255, 255, 190)); p.drawEllipse(QPointF(w / 2.0, h / 2.0), 2.4 * m_s, 2.4 * m_s);
    }
    if (!ui.overview) drawCompass(p, w, h, pl);
    drawBars(p, w, h, world, pl, ui);
    if (ui.showLabels || ui.overview || ui.dmMode) { if (ui.showLabels || ui.overview) drawLabels(p, w, h, world, ui, vp, pl); }

    // prompt de interacao
    if (!ui.tabMenu && !ui.notesOpen && !ui.overview) {
        QString prompt;
        if (Interactable* in = world.focusedInteractable()) prompt = QStringLiteral("[E] ") + (in->promptFn ? in->promptFn() : in->prompt);
        else if (Poi* po = world.focusedPoi()) prompt = QStringLiteral("[E] Notas do Mestre: ") + po->title;
        if (!prompt.isEmpty()) {
            p.setFont(font(20, true));
            QRectF br = p.boundingRect(QRectF(0, 0, w * 0.7, 80), Qt::AlignCenter, prompt);
            QRectF r(w / 2.0 - br.width() / 2 - 18 * m_s, h / 2.0 + 44 * m_s, br.width() + 36 * m_s, br.height() + 14 * m_s);
            p.setPen(Qt::NoPen); p.setBrush(QColor(0, 0, 0, 140)); p.drawRoundedRect(r, 8, 8);
            text(p, r, prompt, 20, QColor(255, 235, 190), Qt::AlignCenter, true);
        }
    }
    drawToasts(p, w, h, world);
    drawBanner(p, w, h, world);
    const GameState& st = world.state;
    if (st.hintAge < st.hintDur && !st.hint.isEmpty()) {
        float a = st.hintAge < 0.8f ? st.hintAge / 0.8f : (st.hintAge > st.hintDur - 1.2f ? std::max(0.f, (st.hintDur - st.hintAge) / 1.2f) : 1.f);
        text(p, QRectF(0, h * 0.86, w, 30 * m_s), st.hint, 19, QColor(255, 235, 200, (int)(230 * a)), Qt::AlignCenter, false);
    }
    if (ui.notesOpen) drawNotes(p, w, h, world, ui);
    else if (ui.tabMenu) drawTabMenu(p, w, h, world, ui);
    if (ui.help) drawHelp(p, w, h);
}
