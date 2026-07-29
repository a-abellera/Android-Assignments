#version 300 es
layout(location = 0) in vec2 POSITION;

uniform mat4 ProjectionMatrix;

void main()
{
    gl_Position = ProjectionMatrix * vec4(POSITION, 0.0, 1.0);
}