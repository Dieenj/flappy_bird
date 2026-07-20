#ifndef WELCOME_VIEW_HPP
#define WELCOME_VIEW_HPP

#include <gui_generated/welcome_screen/WelcomeViewBase.hpp>
#include <gui/welcome_screen/WelcomePresenter.hpp>
#include <touchgfx/Unicode.hpp>

class WelcomeView : public WelcomeViewBase {
public:
    WelcomeView() {}
    virtual ~WelcomeView() {}

    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleClickEvent(const touchgfx::ClickEvent& evt);
    virtual void handleTickEvent();

    void updateBird(int wingPhase);
    void updateBestScore(uint32_t best);

private:
    static const int BX = 46;
    static const int BY = 120;
    touchgfx::Unicode::UnicodeChar _bestBuf[24];
};

#endif
