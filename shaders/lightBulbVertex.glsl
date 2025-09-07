#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;
layout (location = 5) in int aBoneIds[4];
layout (location = 6) in float aWeights[4];

out vec3 WorldPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPosition = model * vec4(aPos, 1.0);

    WorldPos = worldPosition.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * worldPosition;
}