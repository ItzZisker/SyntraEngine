#version 330 core

out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 hdrBoost = vec3(1.0, 1.0, 1.0);
uniform samplerCube skybox;

void main()
{
    vec3 color = texture(skybox, TexCoords).rgb;
    FragColor = vec4(color * hdrBoost, 1.0);
}