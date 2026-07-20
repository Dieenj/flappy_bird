#include <gui/game_screen/GameView.hpp>
#include <gui/game_screen/GamePresenter.hpp>
#include <touchgfx/Color.hpp>

void GameView::setupScreen() {
    GameViewBase::setupScreen();

    // Array of pointers → 20 widget ống do Designer tạo
    _pipe[0] = &boxPipe0;  _pipe[1] = &boxPipe1;
    _pipe[2] = &boxPipe2;  _pipe[3] = &boxPipe3;
    _pipe[4] = &boxPipe4;

    _pipeCap[0] = &boxPipeCap0;  _pipeCap[1] = &boxPipeCap1;
    _pipeCap[2] = &boxPipeCap2;  _pipeCap[3] = &boxPipeCap3;
    _pipeCap[4] = &boxPipeCap4;

    _pipe2[0] = &boxPipe20;  _pipe2[1] = &boxPipe21;
    _pipe2[2] = &boxPipe22;  _pipe2[3] = &boxPipe23;
    _pipe2[4] = &boxPipe24;

    _pipeCap2[0] = &boxPipeCap20;  _pipeCap2[1] = &boxPipeCap21;
    _pipeCap2[2] = &boxPipeCap22;  _pipeCap2[3] = &boxPipeCap23;
    _pipeCap2[4] = &boxPipeCap24;

    containerPause.setVisible(false);
    containerDead.setVisible(false);
    containerCountdown.setVisible(false);
    txtScore.setVisible(false);
    containerPause.invalidate();
    containerDead.invalidate();
    containerCountdown.invalidate();
}

void GameView::tearDownScreen() {
    GameViewBase::tearDownScreen();
}

void GameView::handleTickEvent() {
    static_cast<GamePresenter*>(presenter)->handleTickEvent();
}

void GameView::handleClickEvent(const touchgfx::ClickEvent& evt) {
    if (evt.getType() != touchgfx::ClickEvent::PRESSED) return;

    GameEngine& eng = engine();
    GameEngine::State st = eng.getState();
    int lx = evt.getX();
    int ly = evt.getY();

    if (st == GameEngine::PAUSED) {
        if (lx>=72&&lx<=152&&ly>=134&&ly<=160) {          // Home
            eng.goHome();
            application().gotoWelcomeScreenNoTransition();
            return;
        }
        if (lx>=168&&lx<=248&&ly>=134&&ly<=160) {          // Resume
            eng.togglePause();
            return;
        }
        return;
    }

    if (st == GameEngine::DEAD) {
        if (eng.getDeathTimer() < 40) return;
        if (lx>=82&&lx<=152&&ly>=140&&ly<=164) {           // Home
            eng.goHome();
            application().gotoWelcomeScreenNoTransition();
            return;
        }
        if (lx>=168&&lx<=238&&ly>=140&&ly<=164) {          // Retry
            eng.flap();
            return;
        }
        return;
    }

    static_cast<GamePresenter*>(presenter)->onTouchPressed();
}

// ════════════════════════════════════════════════════════
void GameView::updateDisplay() {
    GameEngine& eng = engine();

    updateBird(eng.getBirdY());
    updatePipes();

    switch (eng.getState()) {
    case GameEngine::PLAYING:   showPlaying(); break;
    case GameEngine::PAUSED:    showPaused(eng.getScore()); break;
    case GameEngine::COUNTDOWN: showCountdown(eng.getCdNum()); break;
    case GameEngine::DEAD:      showDead(eng.getScore(), eng.getBest(), eng.getDeathTimer()); break;
    default: break;
    }
}

void GameView::updateBird(float birdY) {
    int by = (int)birdY;
    int bx = BIRD_X;

    boxBird.moveTo(bx - 13, by - 10);     boxBird.invalidate();
    boxBirdEye.moveTo(bx + 1,  by - 7);   boxBirdEye.invalidate();
    boxPupil.moveTo(bx + 4,    by - 5);   boxPupil.invalidate();
    boxBeak.moveTo(bx + 12,    by - 1);   boxBeak.invalidate();

    _wingPhase = (_wingPhase + 1) % 12;
    int wingOff = (_wingPhase < 4) ? -4 : (_wingPhase < 8 ? 0 : 3);
    boxWing.moveTo(bx - 10, by - 2 + wingOff);
    boxWing.invalidate();
}

void GameView::updatePipes() {
    GameEngine& eng = engine();
    int n = eng.getPipeCount();
    for (int i = 0; i < MAX_PIPES; i++) {
        if (i < n) {
            const Pipe& pp = eng.getPipe(i);
            showPipe(i, (int)pp.x, (int)pp.gapY, pp.gap, pp.type == PIPE_MOVING);
        } else {
            hidePipe(i);
        }
    }
}

void GameView::showPipe(int idx, int x, int gapY, int gap, bool moving) {
    const int PW = PIPE_W, CW = PIPE_CAP_W, CH = PIPE_CAP_H, GT = GROUND_TOP;
    int cx = x - (CW - PW) / 2;

    if (gapY < CH + 2)            gapY = CH + 2;
    if (gapY > GT - gap - CH - 2) gapY = GT - gap - CH - 2;
    if (x + CW < 0) { hidePipe(idx); return; }

    touchgfx::colortype fill = moving
        ? touchgfx::Color::getColorFromRGB(0xCA, 0x40, 0x00)
        : touchgfx::Color::getColorFromRGB(0x3D, 0xC4, 0x00);
    touchgfx::colortype cap = moving
        ? touchgfx::Color::getColorFromRGB(0xFB, 0x60, 0x00)
        : touchgfx::Color::getColorFromRGB(0x4E, 0xE4, 0x00);

    _pipe[idx]->setColor(fill);
    _pipe[idx]->setPosition(x, 0, PW, gapY - CH);
    _pipe[idx]->setVisible(gapY > CH);
    _pipe[idx]->invalidate();

    _pipeCap[idx]->setColor(cap);
    _pipeCap[idx]->setPosition(cx, gapY - CH, CW, CH);
    _pipeCap[idx]->setVisible(true);
    _pipeCap[idx]->invalidate();

    int botY = gapY + gap;
    int botH = GT - botY;

    if (botH > 0) {
        _pipeCap2[idx]->setColor(cap);
        _pipeCap2[idx]->setPosition(cx, botY, CW, CH);
        _pipeCap2[idx]->setVisible(true);
    } else {
        _pipeCap2[idx]->setVisible(false);
    }
    _pipeCap2[idx]->invalidate();

    if (botH > CH) {
        _pipe2[idx]->setColor(fill);
        _pipe2[idx]->setPosition(x, botY + CH, PW, botH - CH);
        _pipe2[idx]->setVisible(true);
    } else {
        _pipe2[idx]->setVisible(false);
    }
    _pipe2[idx]->invalidate();
}

void GameView::hidePipe(int idx) {
    _pipe[idx]->setVisible(false);     _pipe[idx]->invalidate();
    _pipeCap[idx]->setVisible(false);  _pipeCap[idx]->invalidate();
    _pipe2[idx]->setVisible(false);    _pipe2[idx]->invalidate();
    _pipeCap2[idx]->setVisible(false); _pipeCap2[idx]->invalidate();
}

void GameView::updateScore(uint32_t score) {
    Unicode::snprintf(_scoreBuf, 16, "%u", (unsigned)score);
    txtScore.setWildcard(_scoreBuf);
    txtScore.invalidate();
}

void GameView::hideAllOverlays() {
    containerPause.setVisible(false);     containerPause.invalidate();
    containerDead.setVisible(false);      containerDead.invalidate();
    containerCountdown.setVisible(false); containerCountdown.invalidate();
}

void GameView::showPlaying() {
    hideAllOverlays();
    txtScore.setVisible(true);
    updateScore(engine().getScore());
}

void GameView::showPaused(uint32_t score) {
    containerDead.setVisible(false);
    containerCountdown.setVisible(false);
    Unicode::snprintf(_pauseBuf, 24, "%u", (unsigned)score);
    txtPauseScore.setWildcard(_pauseBuf);
    txtPauseScore.invalidate();
    containerPause.setVisible(true);
    containerPause.invalidate();
    txtScore.setVisible(true);
    updateScore(score);
}

void GameView::showDead(uint32_t score, uint32_t best, uint32_t deathTimer) {
    containerPause.setVisible(false);
    containerCountdown.setVisible(false);

    Unicode::snprintf(_deadScoreBuf, 24, "%u", (unsigned)score);
    txtDeadScore.setWildcard(_deadScoreBuf);
    txtDeadScore.invalidate();

    Unicode::snprintf(_deadBestBuf, 24, "%u", (unsigned)best);
    txtDeadBest.setWildcard(_deadBestBuf);
    txtDeadBest.invalidate();

    bool showBtn = (deathTimer >= 40);
    boxDeadHome.setVisible(showBtn);   boxDeadHome.invalidate();
    txtDeadHome.setVisible(showBtn);   txtDeadHome.invalidate();
    boxDeadRetry.setVisible(showBtn);  boxDeadRetry.invalidate();
    txtDeadRetry.setVisible(showBtn);  txtDeadRetry.invalidate();

    containerDead.setVisible(true);
    containerDead.invalidate();
}

void GameView::showCountdown(int cdNum) {
    containerPause.setVisible(false);
    containerDead.setVisible(false);

    if (cdNum > 0) {
        Unicode::snprintf(_cdBuf, 8, "%d", cdNum);
        Unicode::strncpy(_cdSubBuf, "Get ready...", 16);
    } else {
        Unicode::strncpy(_cdBuf, "GO!", 8);
        Unicode::strncpy(_cdSubBuf, "Tap to fly!", 16);
    }
    txtCdNum.setWildcard(_cdBuf);
    txtCdSub.setWildcard(_cdSubBuf);
    txtCdNum.invalidate();
    txtCdSub.invalidate();
    containerCountdown.setVisible(true);
    containerCountdown.invalidate();

    txtScore.setVisible(true);
    updateScore(engine().getScore());
}
