#version 150

out vec4 FragColor;
in vec3 TexCoords;
uniform samplerCube sykbox;

void main() {
    FragColor = texture(skybox, TexCoords);
}