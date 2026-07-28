/**
 * @file    Square.cpp
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   This source file represents the implementation (of functions) that are
 *          specified in interface (header) file Square.h.
 */

#define LOG_TAG "Square"

#include "Square.h"
#include "Renderer.h"
#include "ShaderHelper.h"
#include <cmath>
#include <string>

// ---------------------------------------------------------------------------
// Vertex data
// ---------------------------------------------------------------------------
/**
 * @brief Scaled positions to counteract the 800x600 widescreen aspect ratio stretch,
 *        ensuring the shape renders as a true square in clip space.
 */
static const GLfloat kPositions[] = {
        0.35f,           0.55f,  // Top Left
        0.35f + 0.225f,  0.55f,  // Top Right
        0.35f,           0.25f,  // Bottom Left
        0.35f + 0.225f,  0.25f,  // Bottom Righ
};

/**
 * @brief Solid yellow vertex attributes (R=1.0, G=1.0, B=0.0) applied across all corners.
 */
static const GLfloat kColors[] = {
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
};

// ---------------------------------------------------------------------------
// Square
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID
    /**
     * @brief Constructs a Square instance on Android using the native asset manager.
     * @param assetMgr Native pointer used to query bundled asset folders.
     */
    Square::Square(AAssetManager *assetMgr) : mgr(assetMgr) {}
#else
    /**
     * @brief Constructs a standard Square instance on Desktop or WebGL.
     */
    Square::Square() {}
#endif

/**
 * @brief Destroys the Square instance and unbinds allocations from the GPU context.
 */
Square::~Square()
{
    glDeleteBuffers(1, &vboColor);
    glDeleteBuffers(1, &vboPos);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
}

/**
 * @brief Initializes shader pipelines, configures VAO mapping layouts, and
 *        buffers raw geometry metrics to VRAM.
 */
void Square::InitModel()
{
    LOGI("Square::InitModel");

#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(mgr, "shader/SquareVertex.glsl",
                                                     "shader/SquareFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile("SquareVertex.glsl",
"SquareFragment.glsl");
#endif

    if (!programID) { LOGE("Square: could not create program"); return; }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kPositions), kPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &vboColor);
    glBindBuffer(GL_ARRAY_BUFFER, vboColor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kColors), kColors, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LOGI("Square::InitModel done");
}

/**
 * @brief Draws the bounded square onto the shared frame context via triangle strip configuration.
 *        Clearing logic is omitted to avoid drawing context overwrites during multi-object loops.
 */
void Square::Render()
{

    glUseProgram(programID);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

/**
 * @brief Updates layout window bounds for viewport translation math.
 * @param w Width of target physical context window display.
 * @param h Height of target physical context window display.
 */
void Square::Resize(int w, int h)
{
    glViewport(0, 0, w, h);
}
