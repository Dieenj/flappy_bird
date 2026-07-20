#include <gui/welcome_screen/WelcomeView.hpp>
#include <gui/welcome_screen/WelcomePresenter.hpp>

void WelcomePresenter::activate() {
    _wingPhase = 0;
    view.updateBestScore(model->engine().getBest());
}

void WelcomePresenter::deactivate() {}

void WelcomePresenter::handleTickEvent() {
    _wingPhase = (_wingPhase + 1) % 12;
    view.updateBird(_wingPhase);
}
