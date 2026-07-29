#version 300 es
precision mediump float;

in vec3 normalCoord;
in vec3 eyeCoord;
in vec3 worldPos;

uniform vec3 LightPosition; // Eye-space light position
uniform vec3 LightAmbient;
uniform vec3 LightDiffuse;
uniform vec3 LightSpecular;
uniform float ShininessFactor;

out vec4 FragColor;

void main()
{
    // 1. Procedural Checkerboard Pattern (using XZ world coordinates)
    float scale = 0.5; // Frequency of tiles
    float check = floor(worldPos.x * scale) + floor(worldPos.z * scale);
    float checkBit = mod(check, 2.0);

    vec3 colorLight = vec3(0.95, 0.96, 0.96);
    vec3 colorDark  = vec3(0.17, 0.17, 0.17);
    vec3 baseColor  = (checkBit > 0.5) ? colorLight : colorDark;

    // 2. Phong Lighting Calculation
    vec3 N = normalize(normalCoord);
    vec3 L = normalize(LightPosition - eyeCoord);

    // Diffuse
    float cosAngle = max(0.0, dot(N, L));

    // Specular
    vec3 V = normalize(-eyeCoord);
    vec3 R = reflect(-L, N);
    float sIntensity = pow(max(0.0, dot(R, V)), max(ShininessFactor, 1.0));

    // Combine Ambient + Diffuse + Specular
    vec3 ambient  = baseColor * LightAmbient;
    vec3 diffuse  = baseColor * LightDiffuse * cosAngle;
    vec3 specular = LightSpecular * sIntensity;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}