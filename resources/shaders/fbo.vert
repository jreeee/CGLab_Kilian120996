#version 150

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoords;

out vec2 Texcoord;
void main()
{
    Texcoord = texcoord;
    gl_Position = vec4(in_Position, 0.0, 1.0);
}
