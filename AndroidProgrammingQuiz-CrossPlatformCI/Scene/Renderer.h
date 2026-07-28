/**
 * @file    Renderer.h
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   This interface file contains the declarations (of functions) that will
 *          be defined in source file Renderer.cpp
 */

#pragma once
#include "Model.h"
#include <vector>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class Renderer {
    public:
        /**
         * @brief Thread-safe retrieval method for accessing the global Renderer instance.
         * @return Reference to the local static Renderer instance.
         */
        static Renderer& Instance();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

#ifdef PLATFORM_ANDROID
        /**
        * @brief Caches the native Android asset interface pointer for downstream asset queries.
        * @param mgr Active pointer reference pointing to the JVM bundled asset context.
        */
        void setAssetManager(AAssetManager* mgr);
#endif
        /**
        * @brief Drives initialization stages by allocating, storing, and provisioning target geometry pipelines.
        * @return True upon seamless initialization execution bounds.
        */
        bool initializeRenderer();

        /**
        * @brief Broadcasts resolution changes to every registered element within the viewport bounds.
        * @param w Width metrics provided by the active platform window wrapper.
        * @param h Height metrics provided by the active platform window wrapper.
        */
        void resize(int w, int h);

        /**
        * @brief Clears the active frame canvas exactly once and instructs all pooled graphics
        *        objects to perform their raster draw calls sequentially on top of each other.
        */
        void render();

    private:
        Renderer() = default;

        /**
        * @brief Destroys the Renderer subsystem and purges memory occupied by the Model collection.
        */
        ~Renderer();

        /**
        * @brief Instantiates discrete Model objects and appends them to the internal processing loop.
        */
        void createModels();

        /**
        * @brief Iterates over the active object pool to trigger localized GPU pipeline buffer setups.
        */
        void initializeModels();

        /**
        * @brief Deallocates memory addresses inside the object pool vector to mitigate system heap leaks.
        */
        void clearModels();

#ifdef PLATFORM_ANDROID
        AAssetManager* assetMgr = nullptr;
#endif

        std::vector<Model*> models;
};
