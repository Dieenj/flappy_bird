#ifndef MODEL_HPP
#define MODEL_HPP
#include <gui/model/GameEngine.hpp>

class ModelListener;

class Model {
public:
    Model();
    void bind(ModelListener* listener) { modelListener = listener; }
    void tick();
    GameEngine& engine() { return _engine; }

private:
    ModelListener* modelListener;
    GameEngine     _engine;
};

#endif
