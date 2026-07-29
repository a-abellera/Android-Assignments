/**
 * @file Renderer.h
 * @brief Singleton master renderer orchestrating 3D scene rendering and 2D HUD overlays.
 *
 * Coordinates execution lifecycle across desktop and Android platforms, manages
 * depth state transitions between 3D scene drawing and 2D HUD overlays, and routes touch events.
 */

#pragma once

#include "Scene3D.h"
#include "SceneHUD.h"
#include "Platform.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

/**
 * @class Renderer
 * @brief Singleton manager owning scene layers and driving the main rendering loop.
 */
class Renderer {
public:
    /**
     * @brief Accesses the global Renderer instance.
     * @return Reference to the singleton instance.
     */
    static Renderer& Instance() {
        static Renderer instance;
        return instance;
    }

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
    /**
     * @brief Assigns Android AssetManager handle.
     * @param mgr Pointer to AAssetManager instance.
     */
    void setAssetManager(AAssetManager* mgr);
#endif

    /**
     * @brief Initializes sub-scenes, shaders, and GPU resources.
     * @return True on successful initialization.
     */
    bool initializeRenderer();

    /**
     * @brief Resizes viewport bounds across all owned scene modules.
     * @param w New screen width in pixels.
     * @param h New screen height in pixels.
     */
    void resize(int w, int h);

    /**
     * @brief Clears framebuffers and drives rendering of 3D scene and 2D HUD layers.
     */
    void render();

    /**
     * @brief Processes primary touch down events with HUD priority routing.
     * @param x Touch X screen coordinate.
     * @param y Touch Y screen coordinate.
     */
    void TouchEventDown(float x, float y);

    /**
     * @brief Processes touch movement drag events.
     * @param x Current touch X screen coordinate.
     * @param y Current touch Y screen coordinate.
     */
    void TouchEventMove(float x, float y);

    /**
     * @brief Processes touch release events.
     * @param x Release touch X coordinate.
     * @param y Release touch Y coordinate.
     */
    void TouchEventRelease(float x, float y);

private:
    Renderer()  = default;
    ~Renderer();

    /**
     * @brief Instantiates Scene3D and SceneHUD objects.
     */
    void createModels();

    /**
     * @brief Invokes InitModel across allocated sub-scenes.
     */
    void initializeModels();

    /**
     * @brief Safely deletes owned scene modules.
     */
    void clearModels();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    Scene3D* scene3D = nullptr;
    SceneHUD* sceneHUD = nullptr;
    int screenWidth  = 0;
    int screenHeight = 0;
};