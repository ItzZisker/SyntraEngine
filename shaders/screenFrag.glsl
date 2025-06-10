#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform vec2 resolution;
uniform sampler2D screenTexture;

const float FINAL_BLUR_BIAS = 1.0;
const float offset = 1.0 / 300.0;

vec4 Televisionfy(in vec4 pixel, const in vec2 uv)
{
    float vignette = pow(uv.x * (1.0 - uv.x) * uv.y * (1.0 - uv.y), 0.25) * 2.2;
    return pixel * vignette;
}

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right
    );

    float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++) {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        col += sampleTex[i] * kernel[i];
    }

    //FragColor = Televisionfy(vec4(col, 1.0), TexCoords);
    FragColor = Televisionfy(texture(screenTexture, TexCoords), TexCoords);
}