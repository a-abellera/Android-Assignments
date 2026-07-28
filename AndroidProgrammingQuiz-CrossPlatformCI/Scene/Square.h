/**
 * @file    Square.h
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   This interface file contains the declarations (of functions) that will
 *          be defined in source file Square.cpp
 */

#pragma once
#include "Model.h"
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Square : public Model {
    public:
#ifdef PLATFORM_ANDROID
    /**
     * @brief Constructs a Square instance on Android using the native asset manager.
     * @param assetMgr Native pointer used to query bundled asset folders.
     */
    explicit Square(AAssetManager* assetMgr);
#else
    /**
     * @brief Constructs a standard Square instance on Desktop or WebGL.
     */
    Square();
#endif
        /**
         * @brief Destroys the Square instance and unbinds allocations from the GPU context.
         */
        ~Square() override;

        /**
        * @brief Initializes shader pipelines, configures VAO mapping layouts, and
        *        buffers raw geometry metrics to VRAM.
        */
        void InitModel()            override;

        /**
        * @brief Draws the bounded square onto the shared frame context via triangle strip configuration.
        *        Clearing logic is omitted to avoid drawing context overwrites during multi-object loops.
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
};
