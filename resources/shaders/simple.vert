#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 aTexCoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 Normal;
out vec3 Position;
out vec2 TexCoord;

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0f);
	//using the normal in the render Method did not work, so here it is again
	Normal = (inverse(transpose(ModelMatrix)) * vec4(in_Normal, 0.0f)).xyz;
	Position = vec3(ModelMatrix * vec4(in_Position, 1.0f));
	TexCoord = aTexCoord;
}