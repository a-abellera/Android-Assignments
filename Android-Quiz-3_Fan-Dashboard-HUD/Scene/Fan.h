/**
 * @file Fan.h
 * @brief Definition of the 3D Fan model with ray-picking and hierarchy transforms.
 */

#ifndef FAN_H
#define FAN_H

#include "Model.h"
#include "Transform.h"

#include <array>
#include <string>
#include <glm/glm.hpp>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

/**
 * @class Fan
 * @brief Renders hierarchical fan components and resolves user ray interaction.
 */
class Fan : public Model
{
public:
#ifdef PLATFORM_ANDROID
    /**
     * @brief Constructs Fan model bound to Android AssetManager.
     * @param mgr Android AAssetManager pointer.
     */
    explicit Fan( AAssetManager* mgr );
#else
    /**
     * @brief Default desktop constructor.
     */
    Fan();
#endif
    ~Fan() override;

    /**
     * @brief Loads shaders and OBJ assets for multi-part fan geometry.
     */
    void InitModel() override;

    /**
     * @brief Frees GPU buffers and resets handles.
     */
    void DeInitModel();

    /**
     * @brief Advances animation steps and renders base, pole, housing, hub, and blades.
     */
    void Render() override;

    /**
     * @brief Handles window/screen resize events.
     * @param width Viewport width in pixels.
     * @param height Viewport height in pixels.
     */
    void Resize( int width, int height ) override;

    /**
     * @brief Sets blade rotation speed multiplier.
     * @param speed Speed value in degrees/sec scale.
     */
    void SetSpeed( float speed ) { currentSpeed = speed; }

    /**
     * @brief Performs ray-sphere intersection tests against sub-parts of the fan.
     * @param rayOrigin Origin point of selection ray in world coordinates.
     * @param rayDir Normalized direction vector of selection ray.
     * @return True if a part was selected/toggled, false otherwise.
     */
    bool PickAtRay( const glm::vec3 &rayOrigin, const glm::vec3 &rayDir );

    /**
     * @brief Sets active view and projection matrices.
     * @param view View transformation matrix.
     * @param proj Projection matrix.
     */
    void SetViewProjection( const glm::mat4 &view, const glm::mat4 &proj );

    /**
     * @brief Returns human-readable label for a given internal part index.
     * @param partIndex Internal PartID index value.
     * @return String identifier of part name.
     */
    std::string GetPartName(int partIndex) const;

private:
    enum PartID {
        PART_BASE = 0,
        PART_POLE,
        PART_HUB,
        PART_HOUSING,
        PART_BLADE_0,
        PART_BLADE_1,
        PART_BLADE_2,
        PART_BLADE_3,
        PART_BLADE_4,
        PART_BLADE_5,
        PART_BLADE_6,
        PART_BLADE_7,
        PART_BLADE_8,
        PART_BLADE_9,
        PART_BLADE_10,
        PART_BLADE_11,
        PART_BLADE_12,
        PART_BLADE_13,
        PART_BLADE_14,
        PART_BLADE_15,
        PART_BLADE_16,
        PART_BLADE_17,
        PART_BLADE_18,
        PART_BLADE_19,
        PART_COUNT = 24
    };

    struct PartData {
        glm::vec3 worldPos;
        float radius;
    };

    std::array<PartData, PART_COUNT> cachedParts;
    int selectedPartIndex = -1;
    float currentSpeed = 8.0f;
    float spinAngle = 0.0f;
    float oscAngle = 0.0f;

    // Uniform locations for Phong shader
    GLint mvpLoc       = -1;
    GLint mvLoc        = -1;
    GLint normalMatLoc = -1;
    GLint colorLoc     = -1;
    GLint lightPosLoc  = -1;
    GLint ambientLoc   = -1;
    GLint diffuseLoc   = -1;
    GLint specularLoc  = -1;
    GLint shininessLoc = -1;

    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint vao = 0;

    // Hub Cylinder Geometry Buffers
    GLuint vaoHub = 0;
    GLuint vboHub = 0;
    GLuint iboHub = 0;
    GLsizei hubIndexCount = 0;

    GLuint vaoCylinder = 0;
    GLuint vboCylinder = 0;
    GLuint iboCylinder = 0;
    GLsizei cylinderIndexCount = 0;

    GLuint shaderProgram = 0;

    float aspectRatio = 1.0f;
    Transform transform;

    /**
     * @brief Renders cube-based mesh parts (Base, Pole, Blades).
     * @param partIndex Target part enumeration ID.
     * @param defaultColor Color used when part is unselected.
     */
    void DrawPart( int partIndex, const glm::vec3 &defaultColor );

    /**
     * @brief Loads sphere geometry for fan rotor hub.
     */
    void BuildSphereHub();

    /**
     * @brief Renders sphere hub mesh part.
     * @param partIndex Target part enumeration ID.
     * @param defaultColor Unselected RGB color.
     */
    void DrawHubPart( int partIndex, const glm::vec3 &defaultColor );

    /**
     * @brief Loads semi-hollow cylinder geometry for motor shroud.
     */
    void BuildSemiHollowCylinder();

    /**
     * @brief Renders cylinder mesh part.
     * @param partIndex Target part enumeration ID.
     * @param defaultColor Unselected RGB color.
     */
    void DrawCylinderPart( int partIndex, const glm::vec3 &defaultColor );

    /**
     * @brief Builds vertex and fragment shader program.
     */
    void BuildShaders();

    /**
     * @brief Loads cube OBJ geometry into VAO/VBO buffers.
     */
    void BuildCube();

#ifdef PLATFORM_ANDROID
    AAssetManager* assetMgr = nullptr;
#endif

    static const glm::vec3 kBaseColor;
    static const glm::vec3 kPoleColor;
    static const glm::vec3 kHubColor;
    static const glm::vec3 kBladeColor;
    static const glm::vec3 kSelectedColor;
    glm::mat4 viewMatrix{ 1.0f };
    glm::mat4 projMatrix{ 1.0f };
};

#endif // FAN_H