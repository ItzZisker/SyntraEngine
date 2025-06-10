#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

uniform samplerCube environmentMap;
uniform vec3 cameraPos;
uniform float ior;

void main() {
    vec3 I = normalize(WorldPos - cameraPos);
    vec3 N = normalize(Normal);

    vec3 refractDir = refract(I, N, 1.0 / ior);

    if (length(refractDir) == 0.0) {
        refractDir = reflect(I, N);
    }

    vec3 refractColor = texture(environmentMap, refractDir).rgb;
    FragColor = vec4(refractColor, 1.0);
}