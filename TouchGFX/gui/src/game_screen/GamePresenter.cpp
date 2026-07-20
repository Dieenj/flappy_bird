#include <gui/game_screen/GameView.hpp>
#include <gui/game_screen/GamePresenter.hpp>
#ifndef SIMULATOR
#include "stm32f4xx_hal.h"

#define PA0_PORT   GPIOA
#define PA0_PIN    GPIO_PIN_0
#define LED_G_PORT GPIOG
#define LED_G_PIN  GPIO_PIN_13
#define LED_R_PORT GPIOG
#define LED_R_PIN  GPIO_PIN_14

bool GamePresenter::readPA0() {
    return HAL_GPIO_ReadPin(PA0_PORT, PA0_PIN) == GPIO_PIN_SET;
}

void GamePresenter::pulseGreen(uint8_t t) {
    HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN, GPIO_PIN_SET);
    _ledGTimer = t;
}
void GamePresenter::pulseRed(uint8_t t) {
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, GPIO_PIN_SET);
    _ledRTimer = t;
}
void GamePresenter::pulseBoth(uint8_t t) { pulseGreen(t); pulseRed(t); }

void GamePresenter::updateLEDs() {
    if (_ledGTimer > 0 && --_ledGTimer == 0)
        HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN, GPIO_PIN_RESET);
    if (_ledRTimer > 0 && --_ledRTimer == 0)
        HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, GPIO_PIN_RESET);
}

static void ledsOff() {
    HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, GPIO_PIN_RESET);
}
#else
// Simulator trên PC: không có GPIO/LED, vô hiệu hóa phần phần cứng
bool GamePresenter::readPA0() { return false; }
void GamePresenter::pulseGreen(uint8_t t) { _ledGTimer = t; }
void GamePresenter::pulseRed(uint8_t t) { _ledRTimer = t; }
void GamePresenter::pulseBoth(uint8_t t) { pulseGreen(t); pulseRed(t); }
void GamePresenter::updateLEDs() {
    if (_ledGTimer > 0) --_ledGTimer;
    if (_ledRTimer > 0) --_ledRTimer;
}
static void ledsOff() {}
#endif

void GamePresenter::activate() {
    _btnPrev   = 0;
    _ledGTimer = 0;
    _ledRTimer = 0;
    ledsOff();
    // Bắt đầu game ngay khi vào Game Screen (từ Welcome tap)
    if (model->engine().getState() == GameEngine::WELCOME)
        model->engine().flap();
}

void GamePresenter::deactivate() {}

void GamePresenter::handleTickEvent() {
    GameEngine& eng = model->engine();

    // PA0 edge detect
    bool btnNow = readPA0();
    if (btnNow && !_btnPrev) {
        GameEngine::State st = eng.getState();
        if (st == GameEngine::PLAYING || st == GameEngine::PAUSED) {
            eng.togglePause();
            pulseBoth(6);
        }
    }
    _btnPrev = btnNow ? 1 : 0;

    // Engine tick (Model::tick() đã được FrontendApplication gọi,
    // nhưng để chắc chắn tick chỉ chạy khi Game Screen active, gọi ở đây)
    eng.tick();
    eng.tickDead();

    // LED events
    if (eng.consumeScoreEvent())   pulseGreen(4);
    if (eng.consumeHitEvent())     pulseRed(20);
    if (eng.consumeLevelUpEvent()) pulseBoth(14);
    if (eng.consumeCdEvent()) {
        int n = eng.getCdNum();
        if      (n == 2) pulseRed(10);
        else if (n == 1) pulseBoth(10);
        else if (n == 0) pulseBoth(18);
        else             pulseGreen(10);
    }

    updateLEDs();
    view.updateDisplay();
}

void GamePresenter::onTouchPressed() {
    GameEngine& eng = model->engine();
    if (eng.getState() == GameEngine::COUNTDOWN) return;
    eng.flap();
}
