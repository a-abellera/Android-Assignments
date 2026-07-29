/**
 * @file Ground.cpp
 * @brief Implementation of ground quad setup, lighting uniforms, and OpenGL rendering.
 */

#include "Ground.h"
#include "ShaderHelper.h"
#include <glm/gtc/type_ptr.hpp>

namespace
{
    // Single large quad resting at Y = -2.725 (bottom of fan base)
    const float kQuadSize = 50.0f;
    const float kGroundY  = -2.725f;

    struct GroundVertex {
        glm::vec3 position;
        glm::vec3 normal;
    };

    const GroundVertex kQuadVertices[4] = {
            { { -kQuadSize, kGroundY,  kQuadSize }, { 0.0f, 1.0f, 0.0f } }, // Front-Left
            { {  kQuadSize, kGroundY,  kQuadSize }, { 0.0f, 1.0f, 0.0f } }, // Front-Right
            { {  kQuadSize, kGroundY, -kQuadSize }, { 0.0f, 1.0f, 0.0f } }, // Back-Right
            { { -kQuadSize, kGroundY, -kQuadSize }, { 0.0f, 1.0f, 0.0f } }  // Back-Left
    };

    const GLushort kQuadIndices[6] = {
            0, 1, 2,
            2, 3, 0
    };

#if defined(__ANDROID__)
    constexpr const char *kVertexShaderPath   = "shader/GroundVertex.glsl";
    constexpr const char *kFragmentShaderPath = "shader/GroundFragment.glsl";
#else
    constexpr const char *kVertexShaderPath   = "GroundVertex.glsl";
    constexpr const char *kFragmentShaderPath = "GroundFragment.glsl";
#endif
}

#ifdef PLATFORM_ANDROID
Ground::Ground( AAssetManager* mgr ) : assetMgr( mgr ) {}
#else
Ground::Ground() {}
#endif

Ground::~Ground() {
    DeInitModel();
}

/**
 * @brief Initializes shaders, uniform locations, and quad geometry buffers.
 */
void Ground::InitModel() {
    BuildShaders();
    if ( !shaderProgram ) return;

    mvpLoc       = glGetUniformLocation( shaderProgram, "ModelViewProjectionMatrix" );
    mvLoc        = glGetUniformLocation( shaderProgram, "ModelViewMatrix" );
    normalMatLoc = glGetUniformLocation( shaderProgram, "NormalMatrix" );
    lightPosLoc  = glGetUniformLocation( shaderProgram, "LightPosition" );
    ambientLoc   = glGetUniformLocation( shaderProgram, "LightAmbient" );
    diffuseLoc   = glGetUniformLocation( shaderProgram, "LightDiffuse" );
    specularLoc  = glGetUniformLocation( shaderProgram, "LightSpecular" );
    shininessLoc = glGetUniformLocation( shaderProgram, "ShininessFactor" );

    BuildQuad();
    transform.TransformInit();
}

/**
 * @brief Loads and constructs shader programs from platform assets or disk paths.
 */
void Ground::BuildShaders() {
#ifdef PLATFORM_ANDROID
    shaderProgram = ShaderHelper::buildProgramFromAssets( assetMgr, kVertexShaderPath, kFragmentShaderPath );
#else
    shaderProgram = ShaderHelper::buildProgramFromFile( kVertexShaderPath, kFragmentShaderPath );
#endif
}

/**
 * @brief Interleaves vertex positions and normal vectors into a VAO/VBO/EBO.
 */
void Ground::BuildQuad() {
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // VBO (Positions + Normals interleaved)
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( kQuadVertices ), kQuadVertices, GL_STATIC_DRAW );

    // Attribute 0: vec3 POSITION
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( GroundVertex ), (void*)offsetof( GroundVertex, position ) );

    // Attribute 1: vec3 NORMAL
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( GroundVertex ), (void*)offsetof( GroundVertex, normal ) );

    // EBO / IBO
    glGenBuffers( 1, &ebo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( kQuadIndices ), kQuadIndices, GL_STATIC_DRAW );

    glBindVertexArray( 0 );
}

/**
 * @brief Releases GPU memory handles safely.
 */
void Ground::DeInitModel() {
    if ( vao )           glDeleteVertexArrays( 1, &vao );
    if ( vbo )            glDeleteBuffers( 1, &vbo );
    if ( ebo )            glDeleteBuffers( 1, &ebo );
    if ( shaderProgram )  glDeleteProgram( shaderProgram );
    vao = vbo = ebo = shaderProgram = 0;
}

/**
 * @brief Updates viewport dimensions.
 */
void Ground::Resize( int w, int h ) {
    glViewport( 0, 0, w, h );
}

/**
 * @brief Sets current view and projection transformation matrices.
 */
void Ground::SetViewProjection( const glm::mat4 &view, const glm::mat4 &proj ) {
    viewMatrix = view;
    projMatrix = proj;
}

/**
 * @brief Binds shader attributes and executes geometry draw calls.
 */
void Ground::Render() {
    if ( !shaderProgram ) return;

    glUseProgram( shaderProgram );

    glm::mat4 modelMatrix( 1.0f );
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    glm::mat4 mvpMatrix       = projMatrix * modelViewMatrix;
    glm::mat3 normalMatrix    = glm::transpose( glm::inverse( glm::mat3( modelViewMatrix ) ) );

    glUniformMatrix4fv( mvLoc, 1, GL_FALSE, glm::value_ptr( modelViewMatrix ) );
    glUniformMatrix4fv( mvpLoc, 1, GL_FALSE, glm::value_ptr( mvpMatrix ) );
    glUniformMatrix3fv( normalMatLoc, 1, GL_FALSE, glm::value_ptr( normalMatrix ) );

    // Lighting uniforms
    glm::vec3 lightPosEye = glm::vec3( viewMatrix * glm::vec4( 5.0f, 10.0f, 5.0f, 1.0f ) );
    glUniform3fv( lightPosLoc, 1, glm::value_ptr( lightPosEye ) );
    glUniform3f( ambientLoc, 0.2f, 0.2f, 0.2f );
    glUniform3f( diffuseLoc, 0.8f, 0.8f, 0.8f );
    glUniform3f( specularLoc, 0.3f, 0.3f, 0.3f ); // Lower specular for ground floor
    glUniform1f( shininessLoc, 16.0f );

    glBindVertexArray( vao );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr );
    glBindVertexArray( 0 );
}