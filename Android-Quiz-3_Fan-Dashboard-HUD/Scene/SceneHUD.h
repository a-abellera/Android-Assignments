#pragma once
#include "Model.h"

class SceneHUD : public Model {
    public:
        void InitModel() override;
        void Render() override;
        void Resize(int w, int h) override;

        bool TouchDown(float x, float y); // HUD hit-testing pt 5
        void TouchRelease();

        float CurrentSpeed() const { return speed; }

    private:
        float speed = 8.0f; // 0-20 pt6
        int activeButton = -1; // which btn is held (if any)
};
