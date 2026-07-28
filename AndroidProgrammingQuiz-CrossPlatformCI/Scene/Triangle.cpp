/**
 * @file    Triangle.cpp
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   This source file represents the implementation (of functions) that are
 *          specified in interface (header) file Triangle.h.
 */

#define LOG_TAG "Triangle"

#include "Triangle.h"
#include "Renderer.h"
#include "ShaderHelper.h"
#include <cmath>
#include <string>

// ---------------------------------------------------------------------------
// Vertex data
// ---------------------------------------------------------------------------
/**
 * @brief Normalized clip space positions for the three triangle corners.
 */
static const GLfloat kPositions[] = {
     0.0f,  0.5f,
    -0.5f, -0.5f,
     0.5f, -0.5f
};

/**
 * @brief Interpolated RGB vertex attributes (Red, Green, and Blue corners).
 */
static const GLfloat kColors[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    if (!shader) { LOGE("glCreateShader failed (type=0x%x)", type); return 0; }
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len), '\0');
        glGetShaderInfoLog(shader, len, nullptr, &log[0]);
        LOGE("Shader compile error:\n%s", log.c_str());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint createProgram(const char* vertSrc, const char* fragSrc)
{
    GLuint vs = compileShader(GL_VERTEX_SHADER,   vertSrc);
    if (!vs) return 0;
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!fs) { glDeleteShader(vs); return 0; }

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len), '\0');
        glGetProgramInfoLog(prog, len, nullptr, &log[0]);
        LOGE("Program link error:\n%s", log.c_str());
        glDeleteProgram(prog);
        prog = 0;
    }
    glDetachShader(prog, vs);
    glDetachShader(prog, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ---------------------------------------------------------------------------
// Triangle
// ---------------------------------------------------------------------------
#ifdef PLATFORM_ANDROID
    /**
     * @brief Constructs a Triangle instance on Android using the native asset manager.
     * @param assetMgr Native pointer used to query bundled asset folders.
     */
    Triangle::Triangle(AAssetManager *assetMgr) : mgr(assetMgr) {}
#else
    /**
     * @brief Constructs a standard Triangle instance on Desktop or WebGL.
     */
    Triangle::Triangle() {}
#endif

/**
 * @brief Destroys the Triangle instance and unbinds allocations from the GPU context.
 */
Triangle::~Triangle()
{
    glDeleteBuffers(1, &vboColor);
    glDeleteBuffers(1, &vboPos);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(programID);
}

/**
 * @brief Initializes shader pipelines via ShaderHelper, links uniform references,
 *        and buffers raw geometry attributes to VRAM.
 */
void Triangle::InitModel()
{
    LOGI("Triangle::InitModel");

#ifdef PLATFORM_ANDROID
    programID = ShaderHelper::buildProgramFromAssets(mgr, "shader/BlueTriangleVertex.glsl",
                                                   "shader/BlueTriangleFragment.glsl");
#else
    programID = ShaderHelper::buildProgramFromFile("BlueTriangleVertex.glsl",
"BlueTriangleFragment.glsl");
#endif

    if (!programID) { LOGE("Triangle: could not create program"); return; }

    uRadianAngle = glGetUniformLocation(programID, "RadianAngle");

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

    LOGI("Triangle::InitModel done");
}

/**
 * @brief Increments rotation metrics and issues active draw calls for the geometry.
 *        Clearing logic is omitted to let the master Renderer safely handle unified frame cycles.
 */
void Triangle::Render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(programID);

    float radian = degree++ / 57.2957795f;
    glUniform1f(uRadianAngle, radian);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

/**
 * @brief Updates layout window bounds for viewport translation math.
 * @param w Width of target physical context window display.
 * @param h Height of target physical context window display.
 */
void Triangle::Resize(int w, int h)
{
    glViewport(0, 0, w, h);
}
