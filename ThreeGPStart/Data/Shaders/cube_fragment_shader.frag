#version 330

uniform vec4 diffuse_colour;

in vec3 varying_colour;

out vec4 fragment_colour;

void main(void)
{
	fragment_colour = vec4(varying_colour,1.0);
}