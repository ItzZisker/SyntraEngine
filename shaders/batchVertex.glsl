#version 330 core

#define HAS_SHADOWS ${HAS_SHADOWS=0}

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;
layout (location = 5) in int aBoneIds[4];
layout (location = 6) in float aWeights[4];

out vec3 normal;
out vec3 fragPos;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#if HAS_SHADOWS
out vec4 fragPosLightSpace;
uniform mat4 lightSpaceMatrix;
#endif

void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;  
    texCoord = aTexCoord;
#if HAS_SHADOWS
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
#endif
    
    gl_Position = projection * view * vec4(fragPos, 1.0);
}