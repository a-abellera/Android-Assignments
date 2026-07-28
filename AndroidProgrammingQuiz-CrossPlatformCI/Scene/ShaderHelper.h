/**
 * @file    ShaderHelper.h
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   Reusable utility class for compiling and linking OpenGL ES shaders.
 *          Provides support for both inline C-strings ("stringify" method)
 *          and loading raw GLSL source code files from disk or APK assets.
 */

#pragma once

#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

/**
 * @class ShaderHelper
 * @brief Reusable utility class for compiling and linking OpenGL ES shaders.
 *        Supports both inline C-strings and external file/asset loading.
 */
class ShaderHelper {
    public:
        // Core compile/link, shared by both loading methods
        static GLuint compileShader(GLenum type, const char* src);
        static GLuint linkProgram(GLuint vert, GLuint frag);

        // Method 1: "Stringify", build directly from inline source strings
        static GLuint buildProgram(const char* vertSrc, const char* fragSrc);

        // Method 2: filed based, loads shader source from disk/APK assets
#ifdef PLATFORM_ANDROID
        static std::string loadAsset(AAssetManager* mgr, const char* path);
        static GLuint buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath);
#else
        static std::string loadFile(const char* filename);
        static GLuint buildProgramFromFile(const char* vertFile, const char* fragFile);
#endif
};

/**
 * @brief Compiles an individual shader stage (Vertex or Fragment).
 * @param type The OpenGL shader stage type (e.g., GL_VERTEX_SHADER, GL_FRAGMENT_SHADER).
 * @param src The null-terminated GLSL source code string.
 * @return The compiled OpenGL shader object ID, or 0 if compilation fails.
 */
inline GLuint ShaderHelper::compileShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint status = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "[ShaderHelper] Shader Compile Error: " << log << std::endl;
    }

    return shader;
}

/**
 * @brief Links a vertex shader and a fragment shader into a complete program.
 * @param vert The compiled vertex shader object ID.
 * @param frag The compiled fragment shader object ID.
 * @return The linked OpenGL program object ID, or 0 if linking fails.
 */
inline GLuint ShaderHelper::linkProgram(GLuint vert, GLuint frag) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint status = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "[ShaderHelper] Program Link Error: " << log << std::endl;
    }

    return program;
}

// Method 1: "Stringify", build directly from inline source strings
/**
 * @brief Compiles and links a program using inline raw C-string source text.
 * @param vertSrc Null-terminated vertex shader GLSL source text.
 * @param fragSrc Null-terminated fragment shader GLSL source text.
 * @return The completely linked OpenGL program ID, or 0 on failure.
 */
inline GLuint ShaderHelper::buildProgram(const char *vertSrc, const char *fragSrc) {
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    return linkProgram(vert, frag);
}

// Method 2: filed based, loads shader source from disk/APK assets
#ifdef PLATFORM_ANDROID
    /**
     * @brief Reads an asset file from the Android APK asset manager.
     * @param mgr Pointer to the active Android native AAssetManager instance.
     * @param path Relative path inside the assets folder (e.g., "shader/Triangle.glsl").
     * @return A std::string containing the raw contents of the file, or an empty string on error.
     */
    inline std::string ShaderHelper::loadAsset(AAssetManager *mgr, const char *path) {
        if (!mgr) {
            std::cerr << "[ShaderHelper] Load Asset Error: null AAssetManager for " << path << std::endl;
            return std::string{};
        }

        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) {
            std::cerr << "[ShaderHelper] Load Asset Error: could not open asset " << path << std::endl;
            return std::string{};
        }

        size_t size = AAsset_getLength(asset);
        std::string buffer(size, '\0');
        AAsset_read(asset, &buffer[0], size);
        AAsset_close(asset);

        return buffer;
    }

    /**
    * @brief Builds an OpenGL program by loading GLSL files out of Android assets.
    * @param mgr Pointer to the active Android native AAssetManager instance.
    * @param vertPath Asset directory path to the vertex shader file.
    * @param fragPath Asset directory path to the fragment shader file.
    * @return The completely linked OpenGL program ID, or 0 on failure.
    */
    inline GLuint ShaderHelper::buildProgramFromAssets(AAssetManager *mgr, const char *vertPath,
                                                const char *fragPath) {
        std::string vertSrc = loadAsset(mgr, vertPath);
        std::string fragSrc = loadAsset(mgr, fragPath);
        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }
#else
    /**
     * @brief Searches for and loads a file from local storage using fallback path evaluation.
     * @param filename The base target file name to load.
     * @return A std::string containing the raw contents of the file, or an empty string on error.
     */
    inline std::string ShaderHelper::loadFile(const char* filename) {
        static const char* kSearchDirs[] = {
            "assets/shader/",
            "",
            "shader/",
            "assets/"
        };

        for (const char* dir : kSearchDirs) {
            std::string path = std::string(dir) + filename;
            std::ifstream file(path);
            if (file.is_open()) {
                std::stringstream ss;
                ss << file.rdbuf();
                return ss.str();
            }
        }

        std::cerr << "[ShaderHelper] Load File Error: could not find " << filename
                   << " in any of the expected locations" << std::endl;
        return "";
    }

    /**
     * @brief Builds an OpenGL program by loading local GLSL files from disk.
     * @param vertFile Local relative path or file name of the vertex shader.
     * @param fragFile Local relative path or file name of the fragment shader.
     * @return The completely linked OpenGL program ID, or 0 on failure.
     */
    inline GLuint ShaderHelper::buildProgramFromFile(const char* vertFile, const char* fragFile) {
        std::string vertSrc = loadFile(vertFile);
        std::string fragSrc = loadFile(fragFile);
        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }
#endif
