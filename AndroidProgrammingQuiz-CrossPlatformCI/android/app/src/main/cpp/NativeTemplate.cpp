/**
 * NativeTemplate.cpp
 *
 * Thin JNI bridge between the Java/Android layer and the shared Triangle renderer.
 * All GL logic lives in Scene/Triangle.h + Triangle.cpp.
 *
 * Native method signatures match MainActivity.java:
 *   package com.example.glpiframeworkintro
 *   class   MainActivity
 */

#define LOG_TAG "GLPIFrameworkIntroNative"

#include <jni.h>
#include "Renderer.h"
#include <android/asset_manager_jni.h>

//static Triangle Renderer;

// ---------------------------------------------------------------------------
// JNI entry points
// ---------------------------------------------------------------------------

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeInit(
        JNIEnv* env, jobject /*thiz*/, jobject assetManager) {
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    Renderer::Instance().setAssetManager(mgr);
    return Renderer::Instance().initializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeResize(
        JNIEnv* env, jobject /*thiz*/, jint width, jint height) {
    Renderer::Instance().resize(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeRender(
        JNIEnv* env, jobject /*thiz*/)
{
    Renderer::Instance().render();
}
