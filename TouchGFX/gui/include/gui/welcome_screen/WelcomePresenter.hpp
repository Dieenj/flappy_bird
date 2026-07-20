#ifndef WELCOME_PRESENTER_HPP
#define WELCOME_PRESENTER_HPP

#include <mvp/Presenter.hpp>
#include <gui/model/ModelListener.hpp>

using namespace touchgfx;

class WelcomeView;

class WelcomePresenter : public touchgfx::Presenter, public ModelListener {
public:
    WelcomePresenter(WelcomeView& v) : view(v) {}
    virtual ~WelcomePresenter() {}

    virtual void activate();
    virtual void deactivate();
    void handleTickEvent();

    Model* getModel() { return model; }

private:
    WelcomeView& view;
    int _wingPhase = 0;
};

#endif
