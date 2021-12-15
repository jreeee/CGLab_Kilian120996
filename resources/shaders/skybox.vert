#version 150
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec3 inSkyboxCoords;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

out vec3 TexCoord;

void main (void) {
    TexCoord = inSkyboxCoords;
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(inSkyboxCoords, 1.0f);
}