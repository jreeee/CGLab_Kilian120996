#version 150
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec3 skyboxCoords;

out vec3 TexCoords;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

void main () {
    TexCoords = skyboxCoords;
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(skyboxCoords, 1.0f);
}