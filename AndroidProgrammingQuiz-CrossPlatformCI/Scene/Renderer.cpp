/**
 * @file    Renderer.cpp
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   This source file represents the implementation (of functions) that are
 *          specified in interface (header) file Renderer.h.
 */

#include "Renderer.h"
#include "Triangle.h"
#include "Square.h"

/**
 * @brief Thread-safe retrieval method for accessing the global Renderer instance.
 * @return Reference to the local static Renderer instance.
 */
Renderer& Renderer::Instance() {
    static Renderer instance;
    return instance;
}

/**
 * @brief Destroys the Renderer subsystem and purges memory occupied by the Model collection.
 */
Renderer::~Renderer() {
    clearModels();
}

#ifdef PLATFORM_ANDROID
/**
 * @brief Caches the native Android asset interface pointer for downstream asset queries.
 * @param mgr Active pointer reference pointing to the JVM bundled asset context.
 */
void Renderer::setAssetManager(AAssetManager* mgr) {
    assetMgr = mgr;
}
#endif

/**
 * @brief Instantiates discrete Model objects and appends them to the internal processing loop.
 */
void Renderer::createModels() {
    clearModels();
#ifdef PLATFORM_ANDROID
    models.push_back(new Triangle(assetMgr));
    models.push_back(new Square(assetMgr));
#else
    models.push_back(new Triangle());
    models.push_back(new Square());
#endif
}

/**
 * @brief Iterates over the active object pool to trigger localized GPU pipeline buffer setups.
 */
void Renderer::initializeModels() {
    for (Model* m : models) {
        m->InitModel();
    }
}

/**
 * @brief Deallocates memory addresses inside the object pool vector to mitigate system heap leaks.
 */
void Renderer::clearModels() {
    for (Model* m : models) {
        delete m;
    }
    models.clear();
}

/**
 * @brief Drives initialization stages by allocating, storing, and provisioning target geometry pipelines.
 * @return True upon seamless initialization execution bounds.
 */
bool Renderer::initializeRenderer() {
    createModels();
    initializeModels();
    return true;
}

/**
 * @brief Broadcasts resolution changes to every registered element within the viewport bounds.
 * @param w Width metrics provided by the active platform window wrapper.
 * @param h Height metrics provided by the active platform window wrapper.
 */
void Renderer::resize(int w, int h) {
    for (Model* m : models) {
        m->Resize(w, h);
    }
}

/**
 * @brief Clears the active frame canvas exactly once and instructs all pooled graphics
 *        objects to perform their raster draw calls sequentially on top of each other.
 */
void Renderer::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (Model* m : models) {
        m->Render();
    }
}

