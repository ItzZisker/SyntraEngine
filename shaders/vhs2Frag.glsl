#version 460

uniform sampler2D screenTexture;
uniform float time;

const float range = 0.001;
const float offsetIntensity = 0.001;
const float noiseQuality = 300.0;
const float noiseIntensity = 0.0011;
const float colorOffsetIntensity = 0.7;

in vec2 TexCoords;
out vec4 FragColor;

float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float verticalBar(float pos, float uvY, float offset) {
    float edge0 = (pos - range);
    float edge1 = (pos + range);

    float x = smoothstep(edge0, pos, uvY) * offset;
    x -= smoothstep(pos, edge1, uvY) * offset;
    return x;
}

void main() {
    vec2 uv = TexCoords.xy;
    float uvY = uv.y;

    for (float i = 0.0; i < 0.71; i += 0.1313) {
        float d = mod(time * i, 1.7);
        float o = sin(1.0 - tan(time * 0.24 * i));
        o *= offsetIntensity;
        uv.x += verticalBar(d, uv.y, o);
    }

    uvY *= noiseQuality;
    uvY = float(int(uvY)) * (1.0 / noiseQuality);

    float noise = rand(vec2(time * 0.000009, (uvY) / 1.5));

    uv.x += noise * noiseIntensity;

    vec2 offsetR = vec2(0.006 * sin(time), 0.0) * colorOffsetIntensity;
    vec2 offsetG = vec2(0.0073 * (cos(time * 0.97)), 0.0) * colorOffsetIntensity;

    float r = texture(screenTexture, uv + offsetR).r;
    float g = texture(screenTexture, uv + offsetG).g;
    float b = texture(screenTexture, uv).b;

    float vigAmt = 3.0 + 0.3 * sin(time + 5.0 * cos(time * 5.0));
    float vignette = (1.0 - vigAmt * (uv.y - 0.5) * (uv.y - 0.5)) * (1.0 - vigAmt * (uv.x - 0.5) * (uv.x - 0.5));
    float vMod = (12.0 + mod(uv.y * 30.0 + time, 1.0)) / 13.0;

    FragColor = vec4(r * vignette * vMod, g * vignette * vMod, b * vignette * vMod, 1.0);
}