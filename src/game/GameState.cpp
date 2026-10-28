#include "GameState.h"
#include <algorithm>
#include <cmath>

bool GameState::lightTorch()
{
    if (torchLit && torchSeconds > 60.f) { toast(QStringLiteral("A tocha já está acesa."), QColor(200, 200, 200), 2.f); return false; }
    if (torchLit || torchSeconds <= 0.f || torchSeconds >= kTorchDuration - 1.f) {
        // tocha atual acabando (ou sem tocha): usa uma nova do inventario
    }
    if (!torchLit && torchSeconds > 1.f && torchSeconds < kTorchDuration - 1.f) {
        torchLit = true;                                  // reacende a mesma tocha (apagada por vento)
        toast(QStringLiteral("Você reacende a tocha."), QColor(255, 200, 120), 2.5f);
        return true;
    }
    if (torchesInInventory <= 0) { toast(QStringLiteral("Você não tem mais tochas!"), QColor(255, 120, 100), 3.f); return false; }
    --torchesInInventory;
    torchSeconds = kTorchDuration;
    torchLit = true;
    toast(QStringLiteral("Você acende uma nova tocha (%1 restantes).").arg(torchesInInventory), QColor(255, 200, 120), 3.f);
    return true;
}

void GameState::extinguishTorch(const QString& reason)
{
    if (!torchLit) return;
    torchLit = false;
    toast(reason, QColor(150, 190, 255), 4.f);
}

void GameState::addItem(const QString& name, bool announce)
{
    items.insert(name);
    if (announce) toast(QStringLiteral("Item obtido: %1").arg(name), QColor(255, 220, 120), 4.f);
}

void GameState::addExhaustion(int n)
{
    exhaustion = std::min(6, exhaustion + n);
    toast(QStringLiteral("Exaustão: nível %1").arg(exhaustion), QColor(230, 170, 90), 3.5f);
}

void GameState::toast(const QString& text, const QColor& color, float duration)
{
    Toast t; t.text = text; t.color = color; t.duration = duration;
    toasts.push_back(t);
    if (toasts.size() > 6) toasts.erase(toasts.begin());
}

void GameState::damageText(const QString& text)
{
    toast(text, QColor(255, 80, 70), 5.f);
    flash(QColor(200, 20, 20), 0.7f);
    screenShake(0.05f, 0.35f);
}

void GameState::screenShake(float amplitude, float seconds) { shakeAmp = std::max(shakeAmp, amplitude); shakeTime = std::max(shakeTime, seconds); }

void GameState::fade(float toAlpha, float seconds, const QColor& color)
{
    fadeTarget = toAlpha; fadeColor = color;
    fadeSpeed = seconds > 0.f ? std::fabs(toAlpha - fadeAlpha) / seconds : 1000.f;
}

void GameState::setMeter(const QString& name, float v, const QColor& color)
{
    for (auto& m : meters) if (m.name == name) { m.value = v; m.color = color; return; }
    meters.push_back({name, v, color});
}

void GameState::removeMeter(const QString& name)
{
    meters.erase(std::remove_if(meters.begin(), meters.end(), [&](const Meter& m) { return m.name == name; }), meters.end());
}

void GameState::showBanner(const QString& title, const QString& subtitle) { bannerTitle = title; bannerSub = subtitle; bannerAge = 0.f; }
void GameState::flash(const QColor& c, float strength) { flashColor = c; damageFlash = std::max(damageFlash, strength); }

void GameState::update(float dt)
{
    if (torchLit) {
        torchSeconds -= dt;
        if (torchSeconds <= 0.f) { torchSeconds = 0.f; torchLit = false; toast(QStringLiteral("Sua tocha se consumiu. Escuridão total."), QColor(255, 120, 100), 5.f); }
        else if (torchSeconds < 300.f && torchSeconds + dt >= 300.f) toast(QStringLiteral("A tocha está acabando (5 minutos)."), QColor(255, 190, 100), 4.f);
    }
    for (auto& t : toasts) t.age += dt;
    toasts.erase(std::remove_if(toasts.begin(), toasts.end(), [](const Toast& t) { return t.age > t.duration; }), toasts.end());
    bannerAge += dt;
    hintAge += dt;
    damageFlash = std::max(0.f, damageFlash - dt * 1.6f);
    if (shakeTime > 0.f) { shakeTime -= dt; if (shakeTime <= 0.f) shakeAmp = 0.f; }
    if (fadeAlpha < fadeTarget) fadeAlpha = std::min(fadeTarget, fadeAlpha + fadeSpeed * dt);
    else if (fadeAlpha > fadeTarget) fadeAlpha = std::max(fadeTarget, fadeAlpha - fadeSpeed * dt);
}
