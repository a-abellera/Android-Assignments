/**
 * @file Fan.cpp
 * @brief Logic for 3D Fan model with ray-picking and hierarchy transforms.
 */

#include "Fan.h"
#include "ShaderHelper.h"
#include "ObjHelper.h"
#include "Platform.h"

#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <limits>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

namespace
{
#if defined(__ANDROID__)
    constexpr const char *kVertexShaderPath   = "shader/FanVertex.glsl";
    constexpr const char *kFragmentShaderPath = "shader/FanFragment.glsl";
#else
    constexpr const char *kVertexShaderPath   = "FanVertex.glsl";
    constexpr const char *kFragmentShaderPath = "FanFragment.glsl";
#endif
}

const glm::vec3 Fan::kBaseColor  = glm::vec3( 1.00f, 0.81f, 0.89f );
const glm::vec3 Fan::kPoleColor  = glm::vec3( 1.00f, 0.81f, 0.89f );
const glm::vec3 Fan::kHubColor   = glm::vec3( 0.01f, 0.10f, 0.33f );
const glm::vec3 Fan::kBladeColor = glm::vec3( 1.00f, 0.52f, 0.73f);
const glm::vec3 Fan::kSelectedColor = glm::vec3( 0.99f, 0.85f, 0.45f);

// -----------------------------------------------------------------------------
// Construction / Destruction
// -----------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
Fan::Fan( AAssetManager* mgr ) : assetMgr( mgr )
{
}
#else
Fan::Fan()
{
}
#endif

Fan::~Fan()
{
    DeInitModel();
}

// -----------------------------------------------------------------------------
// Initialization & Camera Matrix Synchronization
// -----------------------------------------------------------------------------
/**
 * @brief Loads shaders and OBJ assets for multi-part fan geometry.
 */
void Fan::InitModel()
{
    BuildShaders();
    if ( !shaderProgram )
    {
        LOGE( "Fan: shader program failed" );
        return;
    }

    mvpLoc        = glGetUniformLocation( shaderProgram, "ModelViewProjectionMatrix" );
    mvLoc         = glGetUniformLocation( shaderProgram, "ModelViewMatrix" );
    normalMatLoc  = glGetUniformLocation( shaderProgram, "NormalMatrix" );
    colorLoc      = glGetUniformLocation( shaderProgram, "PARTCOLOR" );

    lightPosLoc   = glGetUniformLocation( shaderProgram, "LightPosition" );
    ambientLoc    = glGetUniformLocation( shaderProgram, "LightAmbient" );
    diffuseLoc    = glGetUniformLocation( shaderProgram, "LightDiffuse" );
    specularLoc   = glGetUniformLocation( shaderProgram, "LightSpecular" );
    shininessLoc  = glGetUniformLocation( shaderProgram, "ShininessFactor" );

    BuildCube();
    BuildSphereHub();
    BuildSemiHollowCylinder();
    transform.TransformInit();
}

/**
 * @brief Sets active view and projection matrices.
 * @param view View transformation matrix.
 * @param proj Projection matrix.
 */
void Fan::SetViewProjection( const glm::mat4 &view, const glm::mat4 &proj ) {
    viewMatrix = view;
    projMatrix = proj;
}

/**
 * @brief Builds vertex and fragment shader program.
 */
void Fan::BuildShaders()
{
#ifdef PLATFORM_ANDROID
    shaderProgram = ShaderHelper::buildProgramFromAssets( assetMgr, kVertexShaderPath, kFragmentShaderPath );
#else
    shaderProgram = ShaderHelper::buildProgramFromFile( kVertexShaderPath, kFragmentShaderPath );
#endif
}

static GLsizei cubeIndexCount = 0;

/**
 * @brief Loads cube OBJ geometry into VAO/VBO buffers.
 */
void Fan::BuildCube()
{
    // Load CubeWithNormal.obj for Base and Blades (normalizing Y bounds to [0..1])
#ifdef PLATFORM_ANDROID
    OBJMeshData cubeMesh = ObjHelper::loadOBJ("CubeWithNormal.obj", true, assetMgr);
#else
    OBJMeshData cubeMesh = ObjHelper::loadOBJ("CubeWithNormal.obj", true);
#endif
    vao = cubeMesh.vao;
    vbo = cubeMesh.vbo;
    ibo = cubeMesh.ibo;
    cubeIndexCount = cubeMesh.indexCount;
}

/**
 * @brief Loads sphere geometry for fan rotor hub.
 */
void Fan::BuildSphereHub()
{
    if ( vaoHub != 0 ) return;

    // Load IsoSphere.obj for the Center Hub
#ifdef PLATFORM_ANDROID
    OBJMeshData hubMesh = ObjHelper::loadOBJ("IsoSphere.obj", false, assetMgr);
#else
    OBJMeshData hubMesh = ObjHelper::LoadOBJFromFile( "IsoSphere.obj", false );
#endif
    vaoHub = hubMesh.vao;
    vboHub = hubMesh.vbo;
    iboHub = hubMesh.ibo;
    hubIndexCount = hubMesh.indexCount;
}

/**
 * @brief Loads semi-hollow cylinder geometry for motor shroud.
 */
void Fan::BuildSemiHollowCylinder()
{
    if ( vaoCylinder != 0 ) return;

    // Load SemiHollowCylinder.obj from the Models folder
    // Set normalizeCubeY to false so it uses its original origin/pivot
#ifdef PLATFORM_ANDROID
    OBJMeshData cylinderMesh = ObjHelper::loadOBJ("SemiHollowCylinder.obj", false, assetMgr);
#else
    OBJMeshData cylinderMesh = ObjHelper::loadOBJ("SemiHollowCylinder.obj", false);
#endif
    vaoCylinder          = cylinderMesh.vao;
    vboCylinder          = cylinderMesh.vbo;
    iboCylinder          = cylinderMesh.ibo;
    cylinderIndexCount   = cylinderMesh.indexCount;
}

/**
 * @brief Frees GPU buffers and resets handles.
 */
void Fan::DeInitModel()
{
    if ( vao )           glDeleteVertexArrays( 1, &vao );
    if ( vbo )           glDeleteBuffers( 1, &vbo );
    if ( ibo )           glDeleteBuffers( 1, &ibo );
    if ( vaoHub )        glDeleteVertexArrays( 1, &vaoHub );
    if ( vboHub )        glDeleteBuffers( 1, &vboHub );
    if ( iboHub )        glDeleteBuffers( 1, &iboHub );
    if ( vaoCylinder )   glDeleteVertexArrays( 1, &vaoCylinder );
    if ( vboCylinder )   glDeleteBuffers( 1, &vboCylinder );
    if ( iboCylinder )   glDeleteBuffers( 1, &iboCylinder );
    if ( shaderProgram ) glDeleteProgram( shaderProgram );

    vao = vbo = ibo = vaoHub = vboHub = iboHub = vaoCylinder = vboCylinder = iboCylinder = shaderProgram = 0;
}

/**
* @brief Handles window/screen resize events.
* @param width Viewport width in pixels.
* @param height Viewport height in pixels.
*/
void Fan::Resize( int width, int height )
{
    glViewport( 0, 0, width, height );
    aspectRatio = ( height > 0 ) ? static_cast<float>( width ) / static_cast<float>( height ) : 1.0f;
}

// -----------------------------------------------------------------------------
// Drawing with Matrices & Red Selection Highlight
// -----------------------------------------------------------------------------
/**
 * @brief Renders cube-based mesh parts (Base, Pole, Blades).
 * @param partIndex Target part enumeration ID.
 * @param defaultColor Color used when part is unselected.
 */
void Fan::DrawPart( int partIndex, const glm::vec3 &defaultColor )
{
    glm::vec3 finalColor = ( partIndex == selectedPartIndex )
                           ? kSelectedColor
                           : defaultColor;

    glUniform3fv( colorLoc, 1, glm::value_ptr( finalColor ) );

    glm::mat4 modelMatrix = *transform.TransformGetModelViewMatrix();
    glm::mat4 mvMatrix    = viewMatrix * modelMatrix;
    glm::mat4 mvpMatrix   = projMatrix * mvMatrix;
    glm::mat3 normalMat   = glm::transpose( glm::inverse( glm::mat3( mvMatrix ) ) );

    glUniformMatrix4fv( mvpLoc, 1, GL_FALSE, glm::value_ptr( mvpMatrix ) );
    glUniformMatrix4fv( mvLoc, 1, GL_FALSE, glm::value_ptr( mvMatrix ) );
    glUniformMatrix3fv( normalMatLoc, 1, GL_FALSE, glm::value_ptr( normalMat ) );

    glBindVertexArray( vao );
    glDrawElements( GL_TRIANGLES, cubeIndexCount, GL_UNSIGNED_SHORT, nullptr );
    glBindVertexArray( 0 );
}

/**
 * @brief Renders sphere hub mesh part.
 * @param partIndex Target part enumeration ID.
 * @param defaultColor Unselected RGB color.
 */
void Fan::DrawHubPart( int partIndex, const glm::vec3 &defaultColor )
{
    glm::vec3 finalColor = ( partIndex == selectedPartIndex )
                           ? kSelectedColor
                           : defaultColor;

    glUniform3fv( colorLoc, 1, glm::value_ptr( finalColor ) );

    glm::mat4 modelMatrix = *transform.TransformGetModelViewMatrix();
    glm::mat4 mvMatrix    = viewMatrix * modelMatrix;
    glm::mat4 mvpMatrix   = projMatrix * mvMatrix;
    glm::mat3 normalMat   = glm::transpose( glm::inverse( glm::mat3( mvMatrix ) ) );

    glUniformMatrix4fv( mvpLoc, 1, GL_FALSE, glm::value_ptr( mvpMatrix ) );
    glUniformMatrix4fv( mvLoc, 1, GL_FALSE, glm::value_ptr( mvMatrix ) );
    glUniformMatrix3fv( normalMatLoc, 1, GL_FALSE, glm::value_ptr( normalMat ) );

    glBindVertexArray( vaoHub );
    glDrawElements( GL_TRIANGLES, hubIndexCount, GL_UNSIGNED_SHORT, nullptr );
    glBindVertexArray( 0 );
}

/**
 * @brief Renders cylinder mesh part.
 * @param partIndex Target part enumeration ID.
 * @param defaultColor Unselected RGB color.
 */
void Fan::DrawCylinderPart( int partIndex, const glm::vec3 &defaultColor )
{
    glm::vec3 finalColor = ( partIndex == selectedPartIndex )
                           ? kSelectedColor
                           : defaultColor;

    glUniform3fv( colorLoc, 1, glm::value_ptr( finalColor ) );

    glm::mat4 modelMatrix = *transform.TransformGetModelViewMatrix();
    glm::mat4 mvMatrix    = viewMatrix * modelMatrix;
    glm::mat4 mvpMatrix   = projMatrix * mvMatrix;
    glm::mat3 normalMat   = glm::transpose( glm::inverse( glm::mat3( mvMatrix ) ) );

    glUniformMatrix4fv( mvpLoc, 1, GL_FALSE, glm::value_ptr( mvpMatrix ) );
    glUniformMatrix4fv( mvLoc, 1, GL_FALSE, glm::value_ptr( mvMatrix ) );
    glUniformMatrix3fv( normalMatLoc, 1, GL_FALSE, glm::value_ptr( normalMat ) );

    glBindVertexArray( vaoCylinder );
    glDrawElements( GL_TRIANGLES, cylinderIndexCount, GL_UNSIGNED_SHORT, nullptr );
    glBindVertexArray( 0 );
}

// -----------------------------------------------------------------------------
// Render Loop & World Transform Caching
// -----------------------------------------------------------------------------
/**
 * @brief Advances animation steps and renders base, pole, housing, hub, and blades.
 */
void Fan::Render()
{
    // Blade spin
    spinAngle += currentSpeed * 0.005f;
    if ( spinAngle >= 360.0f ) spinAngle -= 360.0f;

    // Entire fan rotation (Showcase spin)
    oscAngle += currentSpeed * 0.002f;
    if ( oscAngle >= 360.0f ) oscAngle -= 360.0f;

    glUseProgram( shaderProgram );

    glm::vec3 lightPosWorld( 5.0f, 10.0f, 5.0f );
    glm::vec3 lightPosEye = glm::vec3( viewMatrix * glm::vec4( lightPosWorld, 1.0f ) );
    glUniform3f( lightPosLoc, lightPosEye.x, lightPosEye.y, lightPosEye.z );

    glUniform3f( ambientLoc, 0.2f, 0.2f, 0.2f );
    glUniform3f( diffuseLoc, 0.8f, 0.8f, 0.8f );
    glUniform3f( specularLoc, 1.0f, 1.0f, 1.0f );
    glUniform1f( shininessLoc, 32.0f );

    transform.TransformSetMatrixMode( MODEL_MATRIX );
    transform.TransformLoadIdentity();

    // Rotate entire fan showcase style
    transform.TransformRotate( oscAngle, 0.0f, 1.0f, 0.0f );

    // 1. Base (Fixed Proportions & Bounding Radius)
    transform.TransformPushMatrix();
    transform.TransformTranslate( 0.0f, -2.0f, 0.0f );
    cachedParts[ PART_BASE ].worldPos = glm::vec3( (*transform.TransformGetModelViewMatrix())[3] );
    cachedParts[ PART_BASE ].radius   = 1.50f;
    transform.TransformScale( 1.0f , 0.2f, 1.0f );
    DrawPart( PART_BASE, kBaseColor );
    transform.TransformPopMatrix();

    // 2. Pole (Fixed Proportions)
    transform.TransformPushMatrix();
    transform.TransformTranslate( 0.0f, -2.0f, 0.0f );
    cachedParts[ PART_POLE ].worldPos = glm::vec3( (*transform.TransformGetModelViewMatrix())[3] );
    cachedParts[ PART_POLE ].radius   = 0.0f;
    transform.TransformScale( 0.15f, 2.2f, 0.15f );
    DrawPart( PART_POLE, kPoleColor );
    transform.TransformPopMatrix();

    // 4. Housing / Shroud
    transform.TransformPushMatrix();
    transform.TransformTranslate( 0.0f, 0.2f, 0.25f );
    cachedParts[ PART_HOUSING ].worldPos = glm::vec3( (*transform.TransformGetModelViewMatrix())[3] );
    cachedParts[ PART_HOUSING ].radius   = 0.25f;
    transform.TransformScale( 0.25f, 0.15f, 0.45f );
    DrawCylinderPart( PART_HOUSING, kPoleColor );
    transform.TransformPopMatrix();

    // 3 & 5. Rotor assembly
    transform.TransformPushMatrix();
    transform.TransformTranslate( 0.0f, 0.2f, 0.75f );          // Shared rotor pivot matches top of pole
    transform.TransformRotate( spinAngle, 0.0f, 0.0f, 1.0f );  // Blades spin

    // 3. Hub Sphere
    transform.TransformPushMatrix();
    transform.TransformTranslate( 0.0f, 0.0f, 0.07f );
    cachedParts[ PART_HUB ].worldPos = glm::vec3( (*transform.TransformGetModelViewMatrix())[3] );
    cachedParts[ PART_HUB ].radius   = 0.25f;
    transform.TransformScale( 0.35f, 0.35f, 0.35f );
    DrawHubPart( PART_HUB, kHubColor );
    transform.TransformPopMatrix();

    // 5. 20 Alternating Blades
    const int kNumBlades = 20;
    const float kAngleStep = 360.0f / static_cast<float>( kNumBlades );

    for ( int i = 0; i < kNumBlades; ++i )
    {
        int bladePartID = PART_BLADE_0 + i;

        transform.TransformPushMatrix();
        transform.TransformRotate( static_cast<float>( i ) + kAngleStep, 0.0f, 0.0f, 0.5f );
        transform.TransformTranslate( 0.0f, 0.35f, 0.04f ); // Pivot slightly offset from center

        // Temporarily move out to the center of the blade geometry for accurate picking
        transform.TransformPushMatrix();
        transform.TransformTranslate( 0.0f, 0.6f, 0.0f );
        cachedParts[ bladePartID ].worldPos = glm::vec3( (*transform.TransformGetModelViewMatrix())[3] );
        transform.TransformPopMatrix();

        cachedParts[ bladePartID ].radius = 0.40f; // Proper radius per blade

        transform.TransformScale( 0.12f, 1.2f, 0.1f );

        // Alternate colors between Orange and Yellow to match the reference
        glm::vec3 color = ((i % 1) == 0) ? kBladeColor : kHubColor;
        DrawPart( bladePartID, color );

        transform.TransformPopMatrix();
    }

    transform.TransformPopMatrix(); // end rotor
}

// -----------------------------------------------------------------------------
// Ray-Sphere Picking Logic
// -----------------------------------------------------------------------------
/**
 * @brief Performs ray-sphere intersection tests against sub-parts of the fan.
 * @param rayOrigin Origin point of selection ray in world coordinates.
 * @param rayDir Normalized direction vector of selection ray.
 * @return True if a part was selected/toggled, false otherwise.
 */
bool Fan::PickAtRay( const glm::vec3 &rayOrigin, const glm::vec3 &rayDir )
{
    float closestT = 1e9f;
    int hitIndex   = -1;

    for ( int i = 0; i < PART_COUNT; ++i ) {
        if (i == PART_POLE || i == PART_HOUSING) continue;

        glm::vec3 oc = rayOrigin - cachedParts[i].worldPos;
        float b = glm::dot( oc, rayDir );
        float c = glm::dot( oc, oc ) - ( cachedParts[i].radius * cachedParts[i].radius );
        float discriminant = b * b - c;

        if ( discriminant >= 0.0f ) {
            float t = -b - sqrt( discriminant );
            if ( t > 0.0f && t < closestT ) {
                closestT = t;
                hitIndex = i;
            }
        }
    }

    if ( hitIndex != -1 ) {
        std::string partName = GetPartName( hitIndex );

        // Toggling OFF currently selected part
        if ( hitIndex == selectedPartIndex ) {
            LOGI( "%s picked - highlight off", partName.c_str() );
            selectedPartIndex = -1;
            return false;
        }

        // Toggling ON new selected part
        selectedPartIndex = hitIndex;
        LOGI( "%s picked - highlight on", partName.c_str() );
        return true;
    }

    selectedPartIndex = -1;
    return false;
}

/**
 * @brief Returns human-readable label for a given internal part index.
 * @param partIndex Internal PartID index value.
 * @return String identifier of part name.
 */
std::string Fan::GetPartName(int partIndex) const
{
    if (partIndex == PART_BASE)    return "Base";
    if (partIndex == PART_POLE)    return "Pole";
    if (partIndex == PART_HOUSING) return "MotorEngine";
    if (partIndex == PART_HUB)     return "Hub";

    // Assuming 20 blades starting from PART_BLADE_0
    if (partIndex >= PART_BLADE_0 && partIndex < PART_BLADE_0 + 20) {
        int bladeNum = (partIndex - PART_BLADE_0) + 1; // 1-based index (Blade1 to Blade20)
        return "Blade" + std::to_string(bladeNum);
    }

    return "Unknown";
}