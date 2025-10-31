#version 330 core

#define HAS_SHADOWS ${HAS_SHADOWS=0}

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord0;
layout (location = 3) in vec2 aTexCoord1;
layout (location = 4) in vec4 aColor;
layout (location = 5) in vec3 aTangent;
layout (location = 6) in int aBoneIds[4];
layout (location = 7) in float aWeights[4];

out VS_OUT {
    vec3 Normal;
    vec3 FragPos;
    vec2 TexCoord0;
    vec2 TexCoord1;
    vec4 Color;
    mat3 TBN;
#if HAS_SHADOWS
    vec4 FragPosLightSpace;
#endif
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#if HAS_SHADOWS
out vec4 fragPosLightSpace;
uniform mat4 lightSpaceMatrix;
#endif

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;  
    vs_out.TexCoord0 = aTexCoord0;
    vs_out.TexCoord1 = aTexCoord1;
    vs_out.Color = aColor;
#if HAS_SHADOWS
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
#endif

    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}