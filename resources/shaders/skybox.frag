#version 150

in vec3 TexCoord;

out vec4 FragColor;

uniform samplerCube skyboxTexture;

void main() {
    FragColor = texture(skyboxTexture, TexCoord);
}