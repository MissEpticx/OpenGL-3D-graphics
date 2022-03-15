#version 330

uniform mat4 combined_xform;
uniform mat4 model_xform;

layout (location=0) in vec3 vertex_position;
layout (location=1) in vec3 vertex_normal;
layout (location=2) in vec2 vertex_texcoord;

out vec3 varying_position;
out vec3 varying_normal;
out vec2 varying_txtrcoord;

void main(void)
{	
	varying_position = mat4x3(model_xform) * vec4(vertex_position, 1.0);
	varying_txtrcoord = vertex_texcoord;
	varying_normal = mat3(model_xform) * vertex_normal;

	gl_Position = combined_xform * model_xform * vec4(vertex_position, 1.0);
}