#ifndef MODEL_LISTENER_HPP
#define MODEL_LISTENER_HPP

#include <gui/model/Model.hpp>

class ModelListener {
public:
    ModelListener() : model(0) {}
    virtual ~ModelListener() {}

    void bind(Model* m) { model = m; }

protected:
    Model* model;
};

#endif
