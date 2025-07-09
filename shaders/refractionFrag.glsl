#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

#define DYNAMIC_OPACITY ${DYNAMIC_OPACITY=0}

uniform samplerCube environmentMap;
uniform vec3 cameraPos;
uniform vec3 ior = vec3(1.515, 1.520, 1.525);

#if DYNAMIC_OPACITY
uniform float minOpacity = 0.7;
uniform float maxOpacity = 1.0;
#else
uniform float opacity = 1.0;
#endif

uniform float F0 = 0.04;

float fresnelSchlick(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 I = normalize(WorldPos - cameraPos);
    vec3 N = normalize(Normal);

    vec3 reflectDir = reflect(I, N);

    float cosTheta = clamp(dot(-I, N), 0.0, 1.0);
    float fresnel = fresnelSchlick(cosTheta, F0);

    vec3 reflectColor = texture(environmentMap, reflectDir).rgb;

    vec3 refractDirR = refract(I, N, 1.0 / ior.x);
    vec3 refractDirG = refract(I, N, 1.0 / ior.y);
    vec3 refractDirB = refract(I, N, 1.0 / ior.z);

    vec3 refractColor;
    refractColor.r = texture(environmentMap, refractDirR).r;
    refractColor.g = texture(environmentMap, refractDirG).g;
    refractColor.b = texture(environmentMap, refractDirB).b;

    vec3 finalColor = mix(refractColor, reflectColor, fresnel);

#if DYNAMIC_OPACITY
    float viewAngle = clamp(dot(-I, N), 0.0, 1.0);
    FragColor = vec4(finalColor, mix(minOpacity, maxOpacity, viewAngle));
#else
    FragColor = vec4(finalColor, opacity);
#endif
}