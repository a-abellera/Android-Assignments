/**
 * @file Scene3D.cpp
 * @brief 3D scene execution including camera look-at transformations, viewport updates, and screen-to-world ray picking.
 */

#include "Scene3D.h"
#include "Platform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#ifdef PLATFORM_ANDROID
Scene3D::Scene3D( AAssetManager* mgr ) : assetMgr( mgr ) {
    fan    = new Fan( mgr );
    ground = new Ground( mgr );
}
#else
Scene3D::Scene3D( void ) {
    fan    = new Fan();
    ground = new Ground();
}
#endif

Scene3D::~Scene3D() {
    delete fan;
    delete ground;
    fan    = nullptr;
    ground = nullptr;
}

/**
 * @brief Initializes ground floor and fan models and transforms.
 */
void Scene3D::InitModel() {
    if ( ground ) ground->InitModel();
    if ( fan )    fan->InitModel();

    transform.TransformInit();
}

/**
 * @brief Calculates perspective projection, look-at view matrix, and invokes child render calls.
 */
void Scene3D::Render() {
    glClearColor( 0.1f, 0.1f, 0.15f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // 1. Calculate perspective projection & view matrix with current cameraDistance
    glm::mat4 projMatrix = glm::perspective( glm::radians( 45.0f ), aspectRatio, 0.1f, 100.0f );
    glm::mat4 viewMatrix = glm::lookAt(
            glm::vec3( 0.0f, 0.8f, cameraDistance ), // Camera position moves back/forth along Z
            glm::vec3( 0.0f, 0.0f, 0.0f ),           // Target look-at point
            glm::vec3( 0.0f, 1.0f, 0.0f )            // Up vector
    );

    // 2. Pass camera matrices to Fan BEFORE rendering
    if ( fan ) {
        fan->SetViewProjection( viewMatrix, projMatrix );
        fan->Render();
    }

    // 3. Render Ground
    if ( ground ) {
        ground->SetViewProjection( viewMatrix, projMatrix );
        ground->Render();
    }
}

/**
 * @brief Resizes viewport and updates aspect ratio for projection matrices.
 */
void Scene3D::Resize( int w, int h ) {
    glViewport( 0, 0, w, h );

    viewportWidth  = w;
    viewportHeight = ( h > 0 ) ? h : 1;
    aspectRatio = static_cast<float>( viewportWidth ) / static_cast<float>( viewportHeight );

    if ( ground ) ground->Resize( w, h );
    if ( fan )    fan->Resize( w, h );
}

/**
 * @brief Sets camera Z-distance clamped within operational range [3.0, 20.0].
 */
void Scene3D::SetCameraDistance( float d ) {
    cameraDistance = std::clamp( d, 3.0f, 20.0f );
}

/**
 * @brief Converts screen pixel coordinates into a 3D ray via unprojection and tests ray-picking against fan parts.
 */
bool Scene3D::PickAt( float screenX, float screenY ) {
    if ( !fan ) return false;

    int viewport[4] = { 0, 0, viewportWidth, viewportHeight };

    // Flip Y coordinate (Touch Y=0 is top, OpenGL Y=0 is bottom)
    float glY = static_cast<float>( viewportHeight ) - screenY;

    glm::vec3 nearPoint( 0.0f );
    glm::vec3 farPoint( 0.0f );

    transform.TransformUnproject(
            screenX, glY, 0.0f,
            &currentViewMatrix, &currentProjMatrix, viewport,
            &nearPoint.x, &nearPoint.y, &nearPoint.z
    );

    transform.TransformUnproject(
            screenX, glY, 1.0f,
            &currentViewMatrix, &currentProjMatrix, viewport,
            &farPoint.x, &farPoint.y, &farPoint.z
    );

    glm::vec3 rayOrigin = nearPoint;
    glm::vec3 rayDir    = glm::normalize( farPoint - nearPoint );

    return fan->PickAtRay( rayOrigin, rayDir );
}