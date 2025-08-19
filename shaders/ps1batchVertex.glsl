#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBiTangent;
layout (location = 5) in int aBoneIds[4];
layout (location = 6) in float aWeights[4];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec2 screenSize = vec2(320.0, 240.0);

noperspective out vec2 vTexCoord;

void main() {
    vec4 clip = projection * view * model * vec4(aPos,1.0);

    // Snap to integer pixel centers in screen space (PS1 wobble)
    vec3 ndc = clip.xyz / clip.w; // -1..1
    vec2 screen = (ndc.xy * 0.5 + 0.5) * screenSize;   // pixels
    screen = floor(screen + 0.5);                     // snap to pixel
    vec2 snappedNDC = (screen / screenSize) * 2.0 - 1.0;

    // Reconstruct clip space with snapped NDC but original w
    gl_Position = vec4(snappedNDC * clip.w, clip.z, clip.w);

    vTexCoord = aTexCoord; // affine
}