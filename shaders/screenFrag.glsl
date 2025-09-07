#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 screenSize;
uniform float gamma = 2.2;

uniform bool hdrEnabled = false;
uniform float hdrExposure = 3.25;

uniform bool fxaaEnabled = false;
uniform float fxaaReduceMin = 1.0 / 128.0;
uniform float fxaaReduceMul = 1.0 / 8.0;
uniform float fxaaSpanMax = 8.0;

vec4 applyFXAA(vec2 texCoords) {
    vec2 texel = 1.0 / screenSize;

    vec3 rgbNW = texture(screenTexture, texCoords + texel * vec2(-1.0, -1.0)).rgb;
    vec3 rgbNE = texture(screenTexture, texCoords + texel * vec2( 1.0, -1.0)).rgb;
    vec3 rgbSW = texture(screenTexture, texCoords + texel * vec2(-1.0,  1.0)).rgb;
    vec3 rgbSE = texture(screenTexture, texCoords + texel * vec2( 1.0,  1.0)).rgb;
    vec3 rgbM  = texture(screenTexture, texCoords).rgb;

    float lumaNW = dot(rgbNW, vec3(0.299, 0.587, 0.114));
    float lumaNE = dot(rgbNE, vec3(0.299, 0.587, 0.114));
    float lumaSW = dot(rgbSW, vec3(0.299, 0.587, 0.114));
    float lumaSE = dot(rgbSE, vec3(0.299, 0.587, 0.114));
    float lumaM  = dot(rgbM,  vec3(0.299, 0.587, 0.114));

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * fxaaReduceMul), fxaaReduceMin);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = clamp(dir * rcpDirMin, -fxaaSpanMax, fxaaSpanMax) * texel;

    vec3 rgbA = 0.5 * (
        texture(screenTexture, texCoords + dir * (1.0 / 3.0 - 0.5)).rgb +
        texture(screenTexture, texCoords + dir * (2.0 / 3.0 - 0.5)).rgb
    );

    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(screenTexture, texCoords + dir * -0.5).rgb +
        texture(screenTexture, texCoords + dir * 0.5).rgb
    );

    float lumaB = dot(rgbB, vec3(0.299, 0.587, 0.114));
    if (lumaB < lumaMin || lumaB > lumaMax)
        return vec4(rgbA, 1.0);
    else
        return vec4(rgbB, 1.0);
}

vec3 applyGamma(vec3 color) {
    return pow(color.rgb, vec3(1.0 / gamma));
}

vec3 applyToneMapping(vec3 color) {
    return vec3(1.0) - exp(-color * hdrExposure);
}

void main() {
    vec4 finalColor = fxaaEnabled ? applyFXAA(TexCoords) : texture(screenTexture, TexCoords);

    if (hdrEnabled) {
        finalColor.rgb = applyToneMapping(finalColor.rgb);
    }
    finalColor.rgb = applyGamma(finalColor.rgb);
    FragColor = finalColor;
}
