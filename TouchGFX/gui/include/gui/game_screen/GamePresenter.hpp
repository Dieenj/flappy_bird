#ifndef GAME_PRESENTER_HPP
#define GAME_PRESENTER_HPP

#include <mvp/Presenter.hpp>
#include <gui/model/ModelListener.hpp>

using namespace touchgfx;

class GameView;

class GamePresenter : public touchgfx::Presenter, public ModelListener {
public:
    GamePresenter(GameView& v) : view(v) {}
    virtual ~GamePresenter() {}

    virtual void activate();
    virtual void deactivate();
    void handleTickEvent();

    void onTouchPressed();
    Model* getModel() { return model; }

private:
    GameView& view;
    uint8_t   _btnPrev   = 0;
    uint8_t   _ledGTimer = 0;
    uint8_t   _ledRTimer = 0;

    bool readPA0();
    void pulseGreen(uint8_t ticks);
    void pulseRed(uint8_t ticks);
    void pulseBoth(uint8_t ticks);
    void updateLEDs();
};

#endif
