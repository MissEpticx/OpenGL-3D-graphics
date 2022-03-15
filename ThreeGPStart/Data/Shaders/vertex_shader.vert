#version 330

uniform mat4 combined_xform;
uniform mat4 model_xform;

layout (location=0) in vec3 vertex_position;
layout (location=1) in vec3 vertex_normal;
layout (location=2) in vec2 vertex_texture;

out vec3 varying_normal;
out vec3 varying_positions;
out vec2 varying_txtrcoord;

void main(void)
{	
	varying_txtrcoord = vertex_texture;
	varying_positions = (model_xform * vec4(vertex_position, 1.0)).xyz;
	varying_normal = (model_xform * vec4(vertex_normal, 0.0)).xyz;

	gl_Position = combined_xform * model_xform * vec4(vertex_position, 1.0);
}