#include <gui/welcome_screen/WelcomeView.hpp>
#include <gui/welcome_screen/WelcomePresenter.hpp>
#include <gui/common/FrontendApplication.hpp>

void WelcomeView::setupScreen() {
    WelcomeViewBase::setupScreen();
    txtBest.setVisible(false);
    txtBest.invalidate();
}

void WelcomeView::tearDownScreen() {
    WelcomeViewBase::tearDownScreen();
}

void WelcomeView::handleClickEvent(const touchgfx::ClickEvent& evt) {
    if (evt.getType() == touchgfx::ClickEvent::PRESSED)
        static_cast<FrontendApplication*>(Application::getInstance())->gotoGameScreenNoTransition();
}

void WelcomeView::handleTickEvent() {
    static_cast<WelcomePresenter*>(presenter)->handleTickEvent();
}

void WelcomeView::updateBird(int wingPhase) {
    int wingOff = (wingPhase < 4) ? -4 : (wingPhase < 8 ? 0 : 3);
    boxWing.moveTo(BX - 10, BY - 2 + wingOff);
    boxWing.invalidate();
}

void WelcomeView::updateBestScore(uint32_t best) {
    if (best > 0) {
        Unicode::snprintf(_bestBuf, 24, "%u", (unsigned)best);
        txtBest.setWildcard(_bestBuf);
        txtBest.setVisible(true);
        txtBest.invalidate();
    }
}
