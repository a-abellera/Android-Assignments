/**
 * @file Renderer.cpp
 * @brief Master renderer loop execution, depth buffer management, and touch event routing.
 */

#include "Renderer.h"
#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
#include <GLES2/gl2.h>
#else
#include "Platform.h"
#endif

Renderer::~Renderer() {
    clearModels();
}

#ifdef PLATFORM_ANDROID
/**
 * @brief Binds asset manager pointer for Android builds.
 */
void Renderer::setAssetManager( AAssetManager* mgr ) {
    assetMgr = mgr;
}
#endif

/**
 * @brief Constructs and initializes sub-models.
 */
bool Renderer::initializeRenderer() {
    clearModels();
    createModels();
    initializeModels();
    LOGI( "Renderer: Initialized successfully" );
    return true;
}

/**
 * @brief Allocates new Scene3D and SceneHUD heap instances.
 */
void Renderer::createModels() {
#ifdef PLATFORM_ANDROID
    scene3D  = new Scene3D( assetMgr );
    sceneHUD = new SceneHUD( assetMgr, scene3D->GetFan(), scene3D );
#else
    scene3D  = new Scene3D();
    sceneHUD = new SceneHUD( scene3D->GetFan(), scene3D );
#endif
}

/**
 * @brief Triggers geometry and shader initialization on sub-models.
 */
void Renderer::initializeModels() {
    if ( scene3D )  scene3D->InitModel();
    if ( sceneHUD ) sceneHUD->InitModel();
}

/**
 * @brief Deallocates sub-model references safely.
 */
void Renderer::clearModels() {
    delete sceneHUD;
    delete scene3D;
    sceneHUD = nullptr;
    scene3D  = nullptr;
}

/**
 * @brief Dispatches screen dimensions to active sub-models.
 */
void Renderer::resize( int w, int h ) {
    screenWidth  = w;
    screenHeight = h;

    if ( scene3D )  scene3D->Resize( w, h );
    if ( sceneHUD ) sceneHUD->Resize( w, h );
}

/**
 * @brief Clears screen buffers and renders 3D world geometry followed by 2D HUD.
 */
void Renderer::render() {
    // 1. Clear both Color and Depth buffers for the new frame
    glClearColor( 0.15f, 0.15f, 0.18f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // 2. EXPLICITLY enable depth test & depth write for 3D Scene
    glEnable( GL_DEPTH_TEST );
    glDepthMask( GL_TRUE );
    glDepthFunc( GL_LEQUAL );

    if ( scene3D )  scene3D->Render();

    // 3. Disable depth test for 2D HUD so HUD buttons render cleanly on top
    glDisable( GL_DEPTH_TEST );
    glDepthMask( GL_FALSE );

    if ( sceneHUD ) sceneHUD->Render();
}

/**
 * @brief Directs touch inputs to HUD first; falls back to 3D ray picking if unhandled.
 */
void Renderer::TouchEventDown( float x, float y ) {
    bool handledByHUD = false;
    if ( sceneHUD ) {
        handledByHUD = sceneHUD->OnTouch( x, y );
    }

    if ( !handledByHUD && scene3D ) {
        scene3D->PickAt( x, y );
    }
}

/**
 * @brief Routes drag event notifications to HUD handlers.
 */
void Renderer::TouchEventMove( float x, float y ) {
    if ( sceneHUD ) {
        sceneHUD->TouchMove( x, y );
    }
}

/**
 * @brief Routes touch release events to HUD handlers.
 */
void Renderer::TouchEventRelease( float x, float y ) {
    if ( sceneHUD ) {
        sceneHUD->TouchRelease( x, y );
    }
}