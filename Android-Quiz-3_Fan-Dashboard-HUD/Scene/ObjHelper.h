/**
 * @file ObjHelper.h
 * @brief Header-only Wavefront OBJ parser and OpenGL ES buffer generator.
 *
 * Provides utilities to parse vertex coordinates, UV texture coordinates, and
 * normals from OBJ streams, supporting cross-platform Android assets and local filesystems.
 */

#pragma once

#if defined(PLATFORM_ANDROID) || defined(__ANDROID__)
#include <android/asset_manager.h>
#else
    struct AAssetManager;
#endif

#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

// ---------------------------------------------------------------------------
// Data Structures
// ---------------------------------------------------------------------------

/**
 * @struct OBJMeshData
 * @brief Holds OpenGL buffer handles and index count for a loaded mesh.
 */
struct OBJMeshData {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLsizei indexCount = 0;
};

/**
 * @struct OBJFaceTuple
 * @brief Stores 1-based index triples (position, texture coord, normal) parsed from OBJ faces.
 */
struct OBJFaceTuple {
    int v = -1;
    int vt = -1;
    int vn = -1;
};

// ---------------------------------------------------------------------------
// Helper Class
// ---------------------------------------------------------------------------

/**
 * @class ObjHelper
 * @brief Parsing utilities for loading Wavefront `.obj` assets into OpenGL VAOs.
 */
class ObjHelper {
public:

    /**
     * @brief Loads an OBJ model file from assets or local disk and uploads GPU buffers.
     *
     * @param filePath Model relative filename (e.g. "CubeWithNormal.obj").
     * @param normalizeCubeY Rescales mesh Y bounds from [-1, 1] to [0, 1] if set to true.
     * @param assetMgr Active AAssetManager pointer required for Android APK assets.
     * @return OBJMeshData structure populated with VAO, VBO, IBO, and element count.
     */
    static OBJMeshData loadOBJ(const std::string& filePath, bool normalizeCubeY = false, AAssetManager* assetMgr = nullptr) {
        OBJMeshData mesh;

#ifdef PLATFORM_ANDROID
        if (assetMgr) {
            // Paths to attempt within APK assets root (app/src/main/assets/)
            const std::vector<std::string> searchPaths = {
                    "Models/" + filePath,
                    filePath,
                    "assets/Models/" + filePath,
                    "assets/" + filePath
            };

            AAsset* asset = nullptr;
            for (const auto& path : searchPaths) {
                asset = AAssetManager_open(assetMgr, path.c_str(), AASSET_MODE_BUFFER);
                if (asset) {
                    LOGI("OBJHelper: Successfully opened asset: %s", path.c_str());
                    break;
                }
            }

            if (!asset) {
                LOGE("OBJHelper: Failed to open asset: %s", filePath.c_str());
                return mesh;
            }

            off_t assetLength = AAsset_getLength(asset);
            std::string fileContents(static_cast<size_t>(assetLength), '\0');
            AAsset_read(asset, &fileContents[0], static_cast<size_t>(assetLength));
            AAsset_close(asset);

            std::istringstream stream(fileContents);
            return parseOBJStream(stream, normalizeCubeY);
        } else {
            LOGE("OBJHelper: AAssetManager is null for %s", filePath.c_str());
            return mesh;
        }
#else
        (void)assetMgr;
        // Paths to attempt on Desktop / Emscripten filesystems
        const std::vector<std::string> searchPaths = {
            "assets/Models/" + filePath,
            "assets/" + filePath,
            "Models/" + filePath,
            filePath,
            "../assets/Models/" + filePath,
            "../Models/" + filePath
        };

        std::ifstream file;
        for (const auto& path : searchPaths) {
            file.open(path);
            if (file.is_open()) {
                LOGI("OBJHelper: Loaded OBJ from %s", path.c_str());
                break;
            }
        }

        if (!file.is_open()) {
            LOGE("OBJHelper: Failed to open OBJ file: %s", filePath.c_str());
            return mesh;
        }

        return parseOBJStream(file, normalizeCubeY);
#endif
    }

    /**
     * @brief Compatibility wrapper for loadOBJ.
     */
    static OBJMeshData LoadOBJFromFile(const std::string& filePath, bool normalizeCubeY = false, AAssetManager* assetMgr = nullptr) {
        return loadOBJ(filePath, normalizeCubeY, assetMgr);
    }

    /**
     * @brief Compatibility wrapper for loadOBJ.
     */
    static OBJMeshData loadOBJFromFile(const std::string& filePath, bool normalizeCubeY = false, AAssetManager* assetMgr = nullptr) {
        return loadOBJ(filePath, normalizeCubeY, assetMgr);
    }

private:

    /**
     * @brief Parses an input stream into interleaved vertex attribute data and generates VAO/VBO/IBO buffers.
     * @param stream Input stream containing OBJ text data.
     * @param normalizeCubeY Rescales Y coordinate bounds when true.
     * @return Fully initialized OBJMeshData handle container.
     */
    static OBJMeshData parseOBJStream(std::istream& stream, bool normalizeCubeY) {
        OBJMeshData mesh;

        std::vector<glm::vec3> temp_positions;
        std::vector<glm::vec2> temp_texcoords;
        std::vector<glm::vec3> temp_normals;

        std::vector<GLfloat> vertexBufferData;
        std::vector<GLushort> indexBufferData;

        std::string line;
        std::vector<OBJFaceTuple> faceTuples;

        while (std::getline(stream, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream lineSS(line);
            std::string prefix;
            lineSS >> prefix;

            if (prefix == "v") {
                glm::vec3 pos;
                lineSS >> pos.x >> pos.y >> pos.z;

                if (normalizeCubeY) {
                    pos.x *= 0.5f;
                    pos.y = (pos.y + 1.0f) * 0.5f; // Origin pivots from bottom (Y = 0)
                    pos.z *= 0.5f;
                }
                temp_positions.push_back(pos);
            }
            else if (prefix == "vt") {
                glm::vec2 uv;
                lineSS >> uv.x >> uv.y;
                temp_texcoords.push_back(uv);
            }
            else if (prefix == "vn") {
                glm::vec3 norm;
                lineSS >> norm.x >> norm.y >> norm.z;
                temp_normals.push_back(norm);
            }
            else if (prefix == "f") {
                std::string vStr;
                std::vector<OBJFaceTuple> faceIndices;
                while (lineSS >> vStr) {
                    OBJFaceTuple t;
                    if (sscanf(vStr.c_str(), "%d/%d/%d", &t.v, &t.vt, &t.vn) == 3) {}
                    else if (sscanf(vStr.c_str(), "%d//%d", &t.v, &t.vn) == 2) {}
                    else if (sscanf(vStr.c_str(), "%d/%d", &t.v, &t.vt) == 2) {}
                    else if (sscanf(vStr.c_str(), "%d", &t.v) == 1) {}

                    if (t.v > 0)  t.v--;
                    if (t.vt > 0) t.vt--;
                    if (t.vn > 0) t.vn--;

                    faceIndices.push_back(t);
                }

                // Triangulate face fan if n-gon > 3 vertices
                for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                    faceTuples.push_back(faceIndices[0]);
                    faceTuples.push_back(faceIndices[i]);
                    faceTuples.push_back(faceIndices[i + 1]);
                }
            }
        }

        // Interleave position (attr 0) and normal (attr 1)
        for (size_t i = 0; i < faceTuples.size(); i += 3) {
            OBJFaceTuple t0 = faceTuples[i];
            OBJFaceTuple t1 = faceTuples[i + 1];
            OBJFaceTuple t2 = faceTuples[i + 2];

            glm::vec3 p0 = temp_positions[t0.v];
            glm::vec3 p1 = temp_positions[t1.v];
            glm::vec3 p2 = temp_positions[t2.v];

            glm::vec3 n0, n1, n2;
            if (t0.vn >= 0 && static_cast<size_t>(t0.vn) < temp_normals.size()) {
                n0 = temp_normals[t0.vn];
                n1 = temp_normals[t1.vn];
                n2 = temp_normals[t2.vn];
            } else {
                // Compute flat surface normal if missing in OBJ file
                glm::vec3 fn = glm::cross(p1 - p0, p2 - p0);
                if (glm::length(fn) > 0.0001f) fn = glm::normalize(fn);
                else fn = glm::vec3(0.0f, 1.0f, 0.0f);
                n0 = n1 = n2 = fn;
            }

            GLushort baseIndex = static_cast<GLushort>(vertexBufferData.size() / 6);

            vertexBufferData.insert(vertexBufferData.end(), { p0.x, p0.y, p0.z, n0.x, n0.y, n0.z });
            vertexBufferData.insert(vertexBufferData.end(), { p1.x, p1.y, p1.z, n1.x, n1.y, n1.z });
            vertexBufferData.insert(vertexBufferData.end(), { p2.x, p2.y, p2.z, n2.x, n2.y, n2.z });

            indexBufferData.push_back(baseIndex);
            indexBufferData.push_back(baseIndex + 1);
            indexBufferData.push_back(baseIndex + 2);
        }

        // Upload to OpenGL VAO/VBO/IBO
        glGenVertexArrays(1, &mesh.vao);
        glBindVertexArray(mesh.vao);

        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(GLfloat), vertexBufferData.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &mesh.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferData.size() * sizeof(GLushort), indexBufferData.data(), GL_STATIC_DRAW);

        // Position attribute (0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));

        // Normal attribute (1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

        glBindVertexArray(0);

        mesh.indexCount = static_cast<GLsizei>(indexBufferData.size());
        return mesh;
    }
};