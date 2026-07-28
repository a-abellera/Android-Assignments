#version 300 es

/**
 * @file    SquareFragment.glsl
 * @author  Aracel Joy Balbaira ABELLERA
 * @par     email: a.abellera@digipen.edu
 * @par     DigiPen login: a.abellera
 * @par     Course: CSD2101
 * @par     Section A
 * @data    06/29/2026
 * @brief   Fragment shader for rendering a solid-colored square.
 *          Receives interpolated vertex color data and outputs it as the final
 *          fragment screen color.
 */

precision mediump float;

in vec3 vColor;
out vec4 fragColor;

void main() {
    fragColor = vec4(vColor, 1.0);
}