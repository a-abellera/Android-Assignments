#pragma once
#include "Model.h"

class Ground : public Model {
    public:
        void InitModel() override;
        void Render()    override;
        void Resize(int w, int h)    override;
};
