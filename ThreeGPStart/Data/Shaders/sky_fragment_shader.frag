#version 330

uniform sampler2D sampler_tex;
uniform vec4 diffuse_colour;

in vec3 varying_position;
in vec3 varying_normal;
in vec2 varying_txtrcoord;

out vec4 fragment_colour;

void main(void)
{
	vec3 N = normalize(varying_normal);
	vec3 tex_colour = texture(sampler_tex, varying_txtrcoord).rgb;
	fragment_colour = vec4(tex_colour,1.0);
}