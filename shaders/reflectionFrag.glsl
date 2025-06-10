#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

uniform samplerCube environmentMap;
uniform vec3 cameraPos;

void main() {
    // vec3 N = normalize(Normal);
    // vec3 V = normalize(cameraPos - WorldPos);
    // vec3 R = reflect(-V, N);

    // // Basic Fresnel term (Schlick's approximation)
    // float fresnel = pow(1.0 - max(dot(N, V), 0.0), 5.0);
    // fresnel = mix(0.1, 1.0, fresnel); // 0.1 = base reflectivity

    // vec3 envColor = texture(environmentMap, R).rgb;

    // vec3 baseColor = vec3(0.2, 0.2, 0.2); // Apple’s paint color
    // vec3 color = mix(baseColor, envColor, fresnel);
    vec3 I = normalize(WorldPos - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(environmentMap, R).rgb, 1.0);

    //FragColor = vec4(color, 1.0);
}