#ifndef GAME_VIEW_HPP
#define GAME_VIEW_HPP

#include <gui_generated/game_screen/GameViewBase.hpp>
#include <gui/game_screen/GamePresenter.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/Unicode.hpp>

class GameView : public GameViewBase {
public:
    GameView() {}
    virtual ~GameView() {}

    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleClickEvent(const touchgfx::ClickEvent& evt);
    virtual void handleTickEvent();

    void updateDisplay();

private:
    touchgfx::Box* _pipe[5];
    touchgfx::Box* _pipeCap[5];
    touchgfx::Box* _pipe2[5];
    touchgfx::Box* _pipeCap2[5];

    int _wingPhase = 0;

    // Wildcard buffers — TouchGFX yêu cầu Unicode::UnicodeChar
    touchgfx::Unicode::UnicodeChar _scoreBuf[16];
    touchgfx::Unicode::UnicodeChar _pauseBuf[24];
    touchgfx::Unicode::UnicodeChar _deadScoreBuf[24];
    touchgfx::Unicode::UnicodeChar _deadBestBuf[24];
    touchgfx::Unicode::UnicodeChar _cdBuf[8];
    touchgfx::Unicode::UnicodeChar _cdSubBuf[16];

    void updateBird(float birdY);
    void updatePipes();
    void showPipe(int idx, int x, int gapY, int gap, bool moving);
    void hidePipe(int idx);
    void updateScore(uint32_t score);
    void showPlaying();
    void showPaused(uint32_t score);
    void showDead(uint32_t score, uint32_t best, uint32_t deathTimer);
    void showCountdown(int cdNum);
    void hideAllOverlays();

    GameEngine& engine() {
        return static_cast<GamePresenter*>(presenter)->getModel()->engine();
    }
};

#endif
