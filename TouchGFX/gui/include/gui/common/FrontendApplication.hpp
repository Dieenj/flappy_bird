#ifndef FRONTENDAPPLICATION_HPP
#define FRONTENDAPPLICATION_HPP

#include <gui_generated/common/FrontendApplicationBase.hpp>

class FrontendHeap;

using namespace touchgfx;

class FrontendApplication : public FrontendApplicationBase
{
public:
    FrontendApplication(Model& m, FrontendHeap& heap);
    virtual ~FrontendApplication() {}

    virtual void handleTickEvent()
    {
        model.tick();
        FrontendApplicationBase::handleTickEvent();
    }

    // Tự viết — Designer không generate vì không có Interaction
    void gotoGameScreenNoTransition();

private:
    touchgfx::Callback<FrontendApplication> gameTransitionCallback;
    void gotoGameScreenNoTransitionImpl();
};

#endif // FRONTENDAPPLICATION_HPP
