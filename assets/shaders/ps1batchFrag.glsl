#version 330 core

noperspective in vec2 vTexCoord; // <- affine interpolation in screen space
uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main(){
    FragColor = vec4(25.0 * texture(texture_diffuse1, vTexCoord).rgb, 1.0);
}