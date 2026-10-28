// Estado de jogo: tocha, exaustao, itens, flags, e auxiliares de HUD (toasts, tremor, fade, medidores).
#pragma once
#include <QColor>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <vector>

class GameState {
public:
    static constexpr float kTorchDuration = 3600.f;   // 1 hora real (Shadowdark)

    // ---- tocha ----
    bool torchLit = false;
    float torchSeconds = kTorchDuration;              // restante na tocha atual
    int torchesInInventory = 5;                       // tochas ainda nao acesas
    bool lightTorch();                                // [R]: acende uma nova (ou reacende a atual se ainda tem tempo)
    void extinguishTorch(const QString& reason);      // vento, agua, flecha de barro...

    // ---- personagem ----
    int exhaustion = 0;                               // niveis de exaustao
    int hitDiceSpent = 0;
    bool darkvision = false;                          // [V]
    QSet<QString> items;
    void addItem(const QString& name, bool announce = true);
    bool hasItem(const QString& name) const { return items.contains(name); }
    void addExhaustion(int n = 1);
    // flags genericas de quebra-cabecas/eventos
    void setFlag(const QString& k, double v = 1.0) { m_flags[k] = v; }
    double flag(const QString& k, double def = 0.0) const { return m_flags.value(k, def); }
    bool hasFlag(const QString& k) const { return m_flags.value(k, 0.0) != 0.0; }

    // ---- folego ----
    float breath = 90.f, maxBreath = 90.f;            // segundos
    bool breathVisible = false;

    // ---- helpers de HUD ----
    struct Toast { QString text; QColor color; float age = 0.f, duration = 4.f; };
    void toast(const QString& text, const QColor& color = QColor(235, 235, 235), float duration = 4.f);
    void damageText(const QString& text);             // toast vermelho + flash
    void screenShake(float amplitude, float seconds); // amplitude em metros (~0.05 = leve)
    void fade(float toAlpha, float seconds, const QColor& color = Qt::black);   // fade da tela
    void setMeter(const QString& name, float value01, const QColor& color = QColor(255, 160, 60));
    void removeMeter(const QString& name);
    void showBanner(const QString& title, const QString& subtitle);
    void flash(const QColor& c, float strength = 0.6f);

    struct Meter { QString name; float value; QColor color; };
    std::vector<Toast> toasts;
    std::vector<Meter> meters;
    QString bannerTitle, bannerSub; float bannerAge = 99.f;
    float damageFlash = 0.f;                          // 0..1
    QColor flashColor{200, 20, 20};
    float shakeAmp = 0.f, shakeTime = 0.f;
    float fadeAlpha = 0.f, fadeTarget = 0.f, fadeSpeed = 1.f; QColor fadeColor{0, 0, 0};
    QString hint; float hintAge = 99.f;               // dica inicial
    void setHint(const QString& s, float seconds = 8.f) { hint = s; hintDur = seconds; hintAge = 0.f; }
    float hintDur = 8.f;

    void update(float dt);

private:
    QHash<QString, double> m_flags;
};
