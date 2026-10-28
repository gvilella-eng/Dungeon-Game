// HUD e menus desenhados com QPainter sobre o quadro GL (Segoe UI, textos em pt-BR).
#pragma once
#include "world/World.h"
#include "game/Player.h"
#include <functional>
#include <vector>
#include <QPainter>

struct UiState {
    bool help = false;
    bool tabMenu = false;
    int tab = 0;                       // 0 Areas, 1 Notas do Mestre, 2 Eventos
    bool notesOpen = false;
    const Poi* notePoi = nullptr;
    float scroll[3] = {0, 0, 0};
    float noteScroll = 0.f;
    bool showLabels = false;           // F4
    bool dmMode = false;               // F2
    bool overview = false;             // O
    bool fullbright = false;           // F3
    bool showStats = false;
    bool loading = false;
    QStringList modeFlags;             // ex.: "Plano de corte X", "Wireframe"
    float fps = 0.f, frameMs = 0.f;
    int drawCalls = 0; size_t tris = 0; size_t chunks = 0;
    std::function<void(const Viewpoint&)> onViewpoint;
    std::function<void(const Poi*)> onPoiGo;
    std::function<void(const Poi*)> onPoiOpen;
    std::function<void(const QString&)> onEvent;
};

class Hud {
public:
    void paint(QPainter& p, int w, int h, World& world, Player& player, UiState& ui, const QMatrix4x4& viewProj);
    bool click(const QPointF& pos);                       // retorna true se consumiu
    bool wheel(const QPointF& pos, int delta, UiState& ui);
    void paintLoading(QPainter& p, int w, int h, float progress, const QString& status, const QString& detail = QString());
private:
    struct Hit { QRectF r; std::function<void()> fn; };
    std::vector<Hit> m_hits;
    double m_s = 1.0;
    QFont font(double px, bool bold = false) const;
    void text(QPainter& p, const QRectF& r, const QString& s, double px, const QColor& c, int flags = Qt::AlignLeft | Qt::AlignVCenter, bool bold = false, bool shadow = true);
    void drawCompass(QPainter& p, int w, int h, Player& pl);
    void drawBars(QPainter& p, int w, int h, World& world, Player& pl, UiState& ui);
    void drawToasts(QPainter& p, int w, int h, World& world);
    void drawBanner(QPainter& p, int w, int h, World& world);
    void drawLabels(QPainter& p, int w, int h, World& world, UiState& ui, const QMatrix4x4& vp, Player& pl);
    void drawTabMenu(QPainter& p, int w, int h, World& world, UiState& ui);
    void drawNotes(QPainter& p, int w, int h, World& world, UiState& ui);
    void drawHelp(QPainter& p, int w, int h);
};
