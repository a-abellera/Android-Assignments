#pragma once
#include "Model.h"
#include "Fan.h"
#include "Ground.h"

class Scene3D : public Model {
    public:
        void InitModel() override;
        void Render()    override;
        void Resize(int w, int h)    override;

        void SetCameraDistance(float d); // called by SceneHUD zoom btns
        bool PickAt(float screenX, float screenY); // 3D picking - pt 8

    private:
        Fan* fan = nullptr;
        Ground* ground = nullptr;
        float cameraDistance = 8.0f;
};
