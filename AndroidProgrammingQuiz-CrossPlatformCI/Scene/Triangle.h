/**
 * @file    Triangle.h
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   This interface file contains the declarations (of functions) that will
 *          be defined in source file Triangle.cpp
 */

#pragma once
#include "Renderer.h"

/**
 * Triangle.h
 *
 * Self-contained rotating triangle renderer (shader, VAO, VBOs).
 * Used by Desktop (Windows/GLFW) and Emscripten builds.
 * Android uses NativeTemplate.cpp (monolithic JNI) instead.
 */

#include "Platform.h"

class Triangle : public Model {
    public:
#ifdef PLATFORM_ANDROID
        /**
         * @brief Constructs a Triangle instance on Android using the native asset manager.
         * @param assetMgr Native pointer used to query bundled asset folders.
         */
        explicit Triangle(AAssetManager* assetMgr);
#else
        /**
         * @brief Constructs a standard Triangle instance on Desktop or WebGL.
         */
        Triangle();
#endif
        /**
        * @brief Destroys the Triangle instance and unbinds allocations from the GPU context.
        */
        ~Triangle() override;

        /**
        * @brief Initializes shader pipelines via ShaderHelper, links uniform references,
        *        and buffers raw geometry attributes to VRAM.
        */
        void InitModel()            override;

        /**
        * @brief Increments rotation metrics and issues active draw calls for the geometry.
        *        Clearing logic is omitted to let the master Renderer safely handle unified frame cycles.
        */
        void Render()               override;

        /**
        * @brief Updates layout window bounds for viewport translation math.
        * @param w Width of target physical context window display.
        * @param h Height of target physical context window display.
        */
        void Resize(int w, int h)   override;

    private:
#ifdef PLATFORM_ANDROID
        AAssetManager* mgr = nullptr;
#endif
        GLuint programID    = 0;
        GLuint vao          = 0;
        GLuint vboPos       = 0;
        GLuint vboColor     = 0;
        GLint  uRadianAngle = -1;
        float  degree       = 0.0f;

};
