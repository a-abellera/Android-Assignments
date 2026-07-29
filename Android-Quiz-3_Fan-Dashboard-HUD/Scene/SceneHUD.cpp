/**
 * @file SceneHUD.cpp
 * @brief Implementation of 2D HUD orthographic rendering, UI button geometry, and touch hit testing.
 */

#include "SceneHUD.h"
#include "ShaderHelper.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

namespace
{
#if defined(__ANDROID__)
    constexpr const char *kVertexShaderPath   = "shader/HUDVertex.glsl";
    constexpr const char *kFragmentShaderPath = "shader/HUDFragment.glsl";
#else
    constexpr const char *kVertexShaderPath   = "HUDVertex.glsl";
    constexpr const char *kFragmentShaderPath = "HUDFragment.glsl";
#endif
}

#ifdef PLATFORM_ANDROID
SceneHUD::SceneHUD( AAssetManager* mgr, Fan* fanRef, Scene3D* scene3DRef )
        : assetMgr( mgr ), fan( fanRef ), scene3D( scene3DRef ) {}
#else
SceneHUD::SceneHUD( Fan* fanRef, Scene3D* scene3DRef )
        : fan( fanRef ), scene3D( scene3DRef ) {}
#endif

SceneHUD::~SceneHUD() {
    DeInitModel();
}

/**
 * @brief Initializes shaders, uniform locations, dynamic VBOs, and syncs initial speed/zoom state.
 */
void SceneHUD::InitModel() {
    BuildShaders();
    if ( !shaderProgram ) return;

    projLoc  = glGetUniformLocation( shaderProgram, "ProjectionMatrix" );
    colorLoc = glGetUniformLocation( shaderProgram, "uColor" );

    BuildBuffers();

    // Sync initial state values to 3D models
    if ( fan ) fan->SetSpeed( currentSpeed );
    if ( scene3D ) scene3D->SetCameraDistance( cameraDistance );
}

/**
 * @brief Loads and compiles HUD 2D vertex and fragment shaders.
 */
void SceneHUD::BuildShaders() {
#ifdef PLATFORM_ANDROID
    shaderProgram = ShaderHelper::buildProgramFromAssets( assetMgr, kVertexShaderPath, kFragmentShaderPath );
#else
    shaderProgram = ShaderHelper::buildProgramFromFile( kVertexShaderPath, kFragmentShaderPath );
#endif
}

/**
 * @brief Allocates dynamic VBO memory for streaming 2D quad geometry.
 */
void SceneHUD::BuildBuffers() {
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * 8, nullptr, GL_DYNAMIC_DRAW );

    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( float ), reinterpret_cast<void*>( 0 ) );

    glBindVertexArray( 0 );
}

/**
 * @brief Releases allocated HUD buffer and shader objects.
 */
void SceneHUD::DeInitModel() {
    if ( vao )           glDeleteVertexArrays( 1, &vao );
    if ( vbo )           glDeleteBuffers( 1, &vbo );
    if ( shaderProgram ) glDeleteProgram( shaderProgram );
    vao = vbo = shaderProgram = 0;
}

/**
 * @brief Recalculates orthographic projection matrix and updates button screen locations.
 */
void SceneHUD::Resize( int w, int h ) {
    screenWidth  = w;
    screenHeight = ( h > 0 ) ? h : 1;

    // 2D Orthographic Projection
    orthoProjection = glm::ortho( 0.0f, static_cast<float>( screenWidth ), static_cast<float>( screenHeight ), 0.0f, -1.0f, 1.0f );

    float btnW   = 120.0f; // Wide rectangular buttons
    float btnH   = 50.0f;
    float margin = 25.0f;
    float gap    = 15.0f;

    // Top-Left Stack (Cyan Buttons: +SPD on top, -SPD below)
    btnSpeedPlus  = { margin, margin, btnW, btnH };
    btnSpeedMinus = { margin, margin + btnH + gap, btnW, btnH };

    // Top-Right Stack (Blue Buttons: e.g. Zoom In / Zoom Out)
    btnZoomIn  = { screenWidth - margin - btnW, margin, btnW, btnH };
    btnZoomOut = { screenWidth - margin - btnW, margin + btnH + gap, btnW, btnH };
}

/**
 * @brief Streams vertex coordinates for a rectangle into the GPU buffer and executes draw.
 */
void SceneHUD::DrawRect( const Rect& r, const glm::vec4& color ) {
    float vertices[8] = {
            r.x,       r.y,
            r.x + r.w, r.y,
            r.x,       r.y + r.h,
            r.x + r.w, r.y + r.h
    };

    glUniform4fv( colorLoc, 1, glm::value_ptr( color ) );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), vertices );

    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    glBindVertexArray( 0 );
}

/**
 * @brief Draws horizontal and vertical bars forming a '+' symbol inside a button rect.
 */
void SceneHUD::DrawSymbolPlus( const Rect& r, const glm::vec4& color ) {
    float pad = 12.0f;
    float thickness = 4.0f;

    // Horizontal bar
    DrawRect( { r.x + pad, r.y + ( r.h - thickness ) * 0.5f, r.w - 2.0f * pad, thickness }, color );
    // Vertical bar
    DrawRect( { r.x + ( r.w - thickness ) * 0.5f, r.y + pad, thickness, r.h - 2.0f * pad }, color );
}

/**
 * @brief Draws a single horizontal bar forming a '-' symbol inside a button rect.
 */
void SceneHUD::DrawSymbolMinus( const Rect& r, const glm::vec4& color ) {
    float pad = 12.0f;
    float thickness = 4.0f;

    // Horizontal bar
    DrawRect( { r.x + pad, r.y + ( r.h - thickness ) * 0.5f, r.w - 2.0f * pad, thickness }, color );
}

/**
 * @brief Executes 2D HUD rendering pass including buttons, icons, and speed progress bar.
 */
void SceneHUD::Render() {
    if ( !shaderProgram ) return;

    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glUseProgram( shaderProgram );
    glUniformMatrix4fv( projLoc, 1, GL_FALSE, glm::value_ptr( orthoProjection ) );

    glm::vec4 cyanColor( 0.15f, 0.70f, 0.70f, 1.0f ); // Top-Left buttons
    glm::vec4 blueColor( 0.45f, 0.60f, 0.90f, 1.0f ); // Top-Right buttons
    glm::vec4 symbolColor( 1.0f, 1.0f, 1.0f, 1.0f );  // White +/- glyphs

    // 1. Render Top-Left Cyan Buttons (+SPD on top, -SPD below)
    DrawRect( btnSpeedPlus, cyanColor );
    DrawSymbolPlus( btnSpeedPlus, symbolColor );
    DrawRect( btnSpeedMinus, cyanColor );
    DrawSymbolMinus( btnSpeedMinus, symbolColor );

    // 2. Render Top-Right Light Blue Buttons (+ZOOM on top, -ZOOM below)
    DrawRect( btnZoomIn, blueColor );
    DrawSymbolPlus( btnZoomIn, symbolColor );
    DrawRect( btnZoomOut, blueColor );
    DrawSymbolMinus( btnZoomOut, symbolColor );

    // 3. Bottom-Center Progress Bar
    float barWidth  = 220.0f;
    float barHeight = 20.0f;
    float barX      = ( static_cast<float>( screenWidth ) - barWidth ) * 0.5f;
    float barY      = static_cast<float>( screenHeight ) - barHeight - 30.0f;

    float ratio = std::clamp( currentSpeed / maxSpeed, 0.0f, 1.0f );
    float filledWidth = barWidth * ratio;

    // Draw dark gray background bar
    DrawRect( { barX, barY, barWidth, barHeight }, glm::vec4( 0.35f, 0.35f, 0.35f, 1.0f ) );
    // Draw green filled active bar
    if ( filledWidth > 0.0f ) {
        DrawRect( { barX, barY, filledWidth, barHeight }, glm::vec4( 0.20f, 0.85f, 0.30f, 1.0f ) );
    }

    glEnable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );
}

/**
 * @brief Processes touch input, triggers button state changes, and updates 3D scene targets.
 */
bool SceneHUD::OnTouch( float screenX, float screenY ) {
    // 1. Speed-
    if ( btnSpeedMinus.Contains( screenX, screenY ) ) {
        currentSpeed = std::max( minSpeed, currentSpeed - 1.0f );
        if ( fan ) fan->SetSpeed( currentSpeed );
        LOGI( "SceneHUD: -SPD pressed");
        return true;
    }

    // 2. Speed+
    if ( btnSpeedPlus.Contains( screenX, screenY ) ) {
        currentSpeed = std::min( maxSpeed, currentSpeed + 1.0f );
        if ( fan ) fan->SetSpeed( currentSpeed );
        LOGI( "SceneHUD: +SPD pressed");
        return true;
    }

    // 3. Zoom In (decreases camera distance)
    if ( btnZoomIn.Contains( screenX, screenY ) ) {
        cameraDistance = std::max( minZoom, cameraDistance - 0.5f );
        if ( scene3D ) scene3D->SetCameraDistance( cameraDistance );
        LOGI( "SceneHUD: +ZOOM pressed");
        return true;
    }

    // 4. Zoom Out (increases camera distance)
    if ( btnZoomOut.Contains( screenX, screenY ) ) {
        cameraDistance = std::min( maxZoom, cameraDistance + 0.5f );
        if ( scene3D ) scene3D->SetCameraDistance( cameraDistance );
        LOGI( "SceneHUD: -ZOOM pressed");
        return true;
    }

    return false; // Point was outside all HUD buttons
}