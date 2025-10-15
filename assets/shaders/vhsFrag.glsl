#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 screenSize;
uniform float gamma = 2.2;

const float offset = 1.0 / 300.0;

vec4 applyVignette(vec4 color, vec2 uv) {
    float vignette = pow(uv.x * (1.0 - uv.x) * uv.y * (1.0 - uv.y), 0.25) * 2.2;
    return color * vignette;
}

vec3 applySharpen(vec2 texCoords) {
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset),
        vec2( 0.0f,    offset),
        vec2( offset,  offset),
        vec2(-offset,  0.0f),
        vec2( 0.0f,    0.0f),
        vec2( offset,  0.0f),
        vec2(-offset, -offset),
        vec2( 0.0f,   -offset),
        vec2( offset, -offset)
    );

    float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

    vec3 result = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        result += kernel[i] * texture(screenTexture, texCoords + offsets[i]).rgb;
    }

    return result;
}

void main() {
    vec3 sharpened = applySharpen(TexCoords);
    vec4 vignetteColor = applyVignette(vec4(sharpened, 1.0), TexCoords);

    FragColor = vignetteColor;
}