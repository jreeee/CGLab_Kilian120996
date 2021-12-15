#version 150

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

void main () {
    TexCoords = aPos;
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(aPos, 1.0f);
}