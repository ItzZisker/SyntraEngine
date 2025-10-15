#version 300 es
precision highp float;
precision highp int;

#define HAS_SHADOWS ${HAS_SHADOWS=0}

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in ivec4 aBoneIds;
layout (location = 5) in ivec4 aWeights;

out vec3 vs_Normal;
out vec3 vs_FragPos;
out vec2 vs_TexCoords;
out mat3 vs_TBN;
#if HAS_SHADOWS
out vec4 vs_FragPosLightSpace;
#endif

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#if HAS_SHADOWS
out vec4 fragPosLightSpace;
uniform mat4 lightSpaceMatrix;
#endif

void main()
{
    vs_FragPos = vec3(model * vec4(aPos, 1.0));
    vs_Normal = mat3(transpose(inverse(model))) * aNormal;  
    vs_TexCoords = aTexCoord;
#if HAS_SHADOWS
    vs_FragPosLightSpace = lightSpaceMatrix * vec4(vs_FragPos, 1.0);
#endif

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    vs_TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(vs_FragPos, 1.0);
}