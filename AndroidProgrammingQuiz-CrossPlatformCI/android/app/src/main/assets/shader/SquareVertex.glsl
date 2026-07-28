#version 300 es

/**
 * @file    SquareVertex.glsl
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   Vertex shader for rendering a static, unrotated square.
 *          Maps raw clip-space vertex positions and forwards vertex colors
 *          directly to the fragment stage.
 */

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    vColor = aColor;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}