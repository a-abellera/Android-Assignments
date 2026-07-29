/**
 * @file Scene3D.h
 * @brief Container model managing 3D fan and ground scene objects, perspective camera, and ray unprojection.
 */

#pragma once

#include "Model.h"
#include "Fan.h"
#include "Ground.h"
#include "Transform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

/**
 * @class Scene3D
 * @brief Manages camera perspective, 3D object rendering, and screen-to-world ray conversion.
 */
class Scene3D : public Model {
public:
#ifdef PLATFORM_ANDROID
    /**
     * @brief Constructs 3D scene with Android asset manager.
     * @param mgr Pointer to active Android AAssetManager.
     */
    explicit Scene3D( AAssetManager* mgr );
#else
    /**
     * @brief Default constructor for desktop builds.
     */
    Scene3D( void );
#endif
    ~Scene3D() override;

    /**
     * @brief Initializes ground and fan models.
     */
    void InitModel() override;

    /**
     * @brief Computes perspective/view matrices and renders 3D components.
     */
    void Render()    override;

    /**
     * @brief Adjusts aspect ratio and updates child scene viewports.
     * @param w New viewport width in pixels.
     * @param h New viewport height in pixels.
     */
    void Resize( int w, int h ) override;

    /**
     * @brief Clamps and sets distance of camera along Z axis.
     * @param d Distance from target focal center.
     */
    void SetCameraDistance( float d );

    /**
     * @brief Converts screen point to world ray and performs ray collision picking on fan parts.
     * @param screenX Screen X touch coordinate.
     * @param screenY Screen Y touch coordinate.
     * @return True if a 3D component was selected or toggled.
     */
    bool PickAt( float screenX, float screenY );

    /**
     * @brief Retrieves pointer to managed Fan model instance.
     * @return Pointer to Fan instance.
     */
    Fan* GetFan() const { return fan; }

private:
    Fan* fan       = nullptr;
    Ground* ground = nullptr;

    float cameraDistance = 8.0f;
    int   viewportWidth  = 1;
    int   viewportHeight = 1;

    glm::mat4 currentProjMatrix{ 1.0f };
    glm::mat4 currentViewMatrix{ 1.0f };
    float aspectRatio{ 1.0f };
    Transform transform;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};