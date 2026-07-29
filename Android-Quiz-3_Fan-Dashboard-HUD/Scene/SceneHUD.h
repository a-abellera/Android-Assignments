/**
 * @file SceneHUD.h
 * @brief 2D Heads-Up Display overlay for UI controls and speed/zoom manipulation.
 *
 * Defines orthographic UI button rectangles, symbol rendering utilities, and touch
 * interception handlers for controlling 3D fan speed and camera distance.
 */

#ifndef SCENE_HUD_H
#define SCENE_HUD_H

#include "Model.h"
#include "Fan.h"
#include "Scene3D.h"
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

/**
 * @struct Rect
 * @brief Bounding rectangle structure for 2D UI layout and hit testing.
 */
struct Rect {
    float x; /**< Origin X position in pixels. */
    float y; /**< Origin Y position in pixels. */
    float w; /**< Width in pixels. */
    float h; /**< Height in pixels. */

    /**
     * @brief Tests whether a screen coordinate point lies within this rectangle.
     * @param px Point X screen coordinate.
     * @param py Point Y screen coordinate.
     * @return True if point is inside rectangle bounds, false otherwise.
     */
    [[nodiscard]] bool Contains( float px, float py ) const {
        return ( px >= x && px <= x + w && py >= y && py <= y + h );
    }
};

/**
 * @class SceneHUD
 * @brief Renders 2D orthographic buttons and progress overlays, routing UI touches.
 */
class SceneHUD : public Model {
public:
#ifdef PLATFORM_ANDROID
    /**
     * @brief Constructs SceneHUD instance bound to Android asset manager and scene references.
     * @param mgr Pointer to active Android AAssetManager.
     * @param fanRef Pointer to target Fan instance for speed adjustment.
     * @param scene3DRef Pointer to target Scene3D instance for zoom control.
     */
    explicit SceneHUD( AAssetManager* mgr, Fan* fanRef, Scene3D* scene3DRef );
#else
    /**
     * @brief Constructs SceneHUD instance for desktop platforms.
     * @param fanRef Pointer to target Fan instance.
     * @param scene3DRef Pointer to target Scene3D instance.
     */
    SceneHUD( Fan* fanRef, Scene3D* scene3DRef );
#endif

    /**
     * @brief Destructor. Cleans up OpenGL buffer objects and shaders.
     */
    ~SceneHUD() override;

    /**
     * @brief Builds shaders, allocates dynamic VBOs, and synchronizes initial UI values.
     */
    void InitModel() override;

    /**
     * @brief Renders 2D UI button rects, icons, and speed progress bar.
     */
    void Render() override;

    /**
     * @brief Recomputes orthographic projection and reposition button rects on window resize.
     * @param w Viewport width in pixels.
     * @param h Viewport height in pixels.
     */
    void Resize( int w, int h ) override;

    /**
     * @brief Evaluates touch inputs against UI button bounding boxes.
     * @param screenX Touch X coordinate on screen.
     * @param screenY Touch Y coordinate on screen.
     * @return True if touch was intercepted by HUD (prevents 3D object picking behind UI).
     */
    bool OnTouch( float screenX, float screenY );

    /**
     * @brief Touch movement event handler wrapper.
     */
    static bool TouchMove( float x, float y )  { (void)x; (void)y; return false; }

    /**
     * @brief Touch release event handler wrapper.
     */
    static bool TouchRelease( float x, float y ) { (void)x; (void)y; return false; }

private:
    /**
     * @brief Compiles and links 2D HUD shader programs.
     */
    void BuildShaders();

    /**
     * @brief Allocates VAO and dynamic VBO buffers for UI quad rendering.
     */
    void BuildBuffers();

    /**
     * @brief Deallocates GPU memory and resets handles.
     */
    void DeInitModel();

    /**
     * @brief Renders a solid color rectangle using the dynamic VBO.
     * @param r Target rectangle bounds.
     * @param color Normalized RGBA color.
     */
    void DrawRect( const Rect& r, const glm::vec4& color );

    /**
     * @brief Draws a plus ('+') symbol vector glyph centered inside a button rect.
     * @param r Target button rectangle.
     * @param color Symbol color vector.
     */
    void DrawSymbolPlus( const Rect& r, const glm::vec4& color );

    /**
     * @brief Draws a minus ('-') symbol vector glyph centered inside a button rect.
     * @param r Target button rectangle.
     * @param color Symbol color vector.
     */
    void DrawSymbolMinus( const Rect& r, const glm::vec4& color );

    Fan*     fan     = nullptr;
    Scene3D* scene3D = nullptr;

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint shaderProgram = 0;

    GLint projLoc  = -1;
    GLint colorLoc = -1;

    glm::mat4 orthoProjection{ 1.0f };
    int screenWidth  = 1;
    int screenHeight = 1;

    // HUD State
    float currentSpeed   = 6.0f;
    const float minSpeed = 0.0f;
    const float maxSpeed = 20.0f;

    float cameraDistance = 8.0f;
    const float minZoom  = 3.0f;
    const float maxZoom  = 20.0f;

    // Button Bounding Rectangles
    Rect btnSpeedMinus{};
    Rect btnSpeedPlus{};
    Rect btnZoomIn{};
    Rect btnZoomOut{};

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};

#endif // SCENE_HUD_H