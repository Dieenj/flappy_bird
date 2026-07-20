#include <gui/common/FrontendApplication.hpp>
#include <gui/common/FrontendHeap.hpp>
#include <gui/game_screen/GameView.hpp>
#include <gui/game_screen/GamePresenter.hpp>
#include <touchgfx/transitions/NoTransition.hpp>

FrontendApplication::FrontendApplication(Model& m, FrontendHeap& heap)
    : FrontendApplicationBase(m, heap)
{
}

void FrontendApplication::gotoGameScreenNoTransition()
{
    gameTransitionCallback = touchgfx::Callback<FrontendApplication>(
        this, &FrontendApplication::gotoGameScreenNoTransitionImpl);
    pendingScreenTransitionCallback = &gameTransitionCallback;
}

void FrontendApplication::gotoGameScreenNoTransitionImpl()
{
    touchgfx::makeTransition<GameView, GamePresenter, touchgfx::NoTransition, Model>(
        &currentScreen, &currentPresenter, frontendHeap, &currentTransition, &model);
}
