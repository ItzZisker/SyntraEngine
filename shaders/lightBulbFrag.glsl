#version 330 core

out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

uniform vec3 bulbColor = vec3(1.0, 1.0, 1.0);
uniform float hdrBoost = 1.0;

void main() {
    FragColor = vec4(bulbColor * hdrBoost, 1.0);
}