#version 300 es
layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;

uniform mat4 ModelViewProjectionMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;

out vec3 normalCoord;
out vec3 eyeCoord;
out vec3 worldPos;

void main()
{
    worldPos = POSITION;
    vec4 eyePos = ModelViewMatrix * vec4(POSITION, 1.0);
    eyeCoord = eyePos.xyz;
    normalCoord = normalize(NormalMatrix * NORMAL);
    gl_Position = ModelViewProjectionMatrix * vec4(POSITION, 1.0);
}