#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#ifndef SIMULATOR
#include "stm32f4xx_hal.h"
#else
#include <ctime>
#endif

Model::Model() : modelListener(0) {
#ifndef SIMULATOR
    _engine.init(HAL_GetTick());
#else
    _engine.init((uint32_t)time(0));
#endif
}

void Model::tick() {
    // Engine tick được gọi từ GamePresenter::handleTickEvent()
    // để đảm bảo chỉ tick khi Game Screen đang active
}
