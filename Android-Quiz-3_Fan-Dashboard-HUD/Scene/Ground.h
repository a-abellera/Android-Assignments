/**
 * @file Ground.h
 * @brief Class declaration for rendering a lit 3D ground plane mesh.
 *
 * Provides lifecycle management, transformation matrix synchronization, and Phong
 * lighting uniform binding for a static ground floor plane.
 */

#ifndef GROUND_H
#define GROUND_H

#include "Model.h"
#include "Transform.h"
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

/**
 * @class Ground
 * @brief Manages geometry buffers, shaders, and draw calls for the ground surface.
 */
class Ground : public Model {
public:
#ifdef PLATFORM_ANDROID
    /**
     * @brief Constructs Ground instance using Android AssetManager.
     * @param mgr Pointer to active Android asset manager.
     */
    explicit Ground( AAssetManager* mgr );
#else
    /**
     * @brief Default constructor for desktop/standalone builds.
     */
    Ground( void );
#endif
    /**
     * @brief Destructor. Releases allocated OpenGL hardware buffers.
     */
    ~Ground() override;

    /**
     * @brief Compiles shaders and uploads quad geometry to VAO/VBO/EBO buffers.
     */
    void InitModel() override;

    /**
     * @brief Draws the ground surface with active view, projection, and lighting settings.
     */
    void Render() override;

    /**
     * @brief Updates the viewport bounds.
     * @param w Viewport width in pixels.
     * @param h Viewport height in pixels.
     */
    void Resize( int w, int h ) override;

    /**
     * @brief Synchronizes external camera matrices into local state.
     * @param view Matrix mapping world coordinates to eye space.
     * @param proj Matrix mapping eye coordinates to clip space.
     */
    void SetViewProjection( const glm::mat4 &view, const glm::mat4 &proj );

private:
    /**
     * @brief Compiles and links ground vertex and fragment shaders.
     */
    void BuildShaders();

    /**
     * @brief Builds vertex array buffers for the quad mesh.
     */
    void BuildQuad();

    /**
     * @brief Deallocates all GPU buffers and shader program resources.
     */
    void DeInitModel();

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0; // Element/Index Buffer Object
    GLuint shaderProgram = 0;

    GLint mvpLoc       = -1;
    GLint mvLoc        = -1;
    GLint normalMatLoc = -1;
    GLint lightPosLoc  = -1;
    GLint ambientLoc   = -1;
    GLint diffuseLoc   = -1;
    GLint specularLoc  = -1;
    GLint shininessLoc = -1;

    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projMatrix{1.0f};

    Transform transform;

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif
};

#endif // GROUND_H