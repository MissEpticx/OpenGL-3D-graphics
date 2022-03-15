#version 330

uniform sampler2D sampler_tex;

uniform vec3 light_intensity;

in vec3 varying_colour;
in vec3 varying_normal;
in vec2 varying_txtrcoord;
in vec3 varying_positions;

out vec4 fragment_colour;

void main(void)
{
	vec3 tex_colour = texture(sampler_tex, varying_txtrcoord).rgb;

	vec3 Norm = normalize(varying_normal);

	vec3 light_direction = vec3(0, 0.2, 1); 
	vec3 Light = normalize(-light_direction);

	float ambient_Intensity = 0.4;
	vec3 ambient_Colour = tex_colour;

	float intensity = max(dot(Light, Norm), 0);
	
	vec3 light_Colour = vec3(1, 1, 1);

	vec3 light_position = vec3(500, 60, 1200);
	vec3 pointLight_direction = light_position - varying_positions;
	Light = normalize(pointLight_direction);

	float pointLight_intensity = max(dot(Light, Norm), 0);

	vec3 pointLight_Colour = vec3(0.1, 0, 0.6);

	//Calculates all the lighting
	vec3 result = ambient_Intensity * ambient_Colour + tex_colour * (intensity * light_Colour + pointLight_intensity * pointLight_Colour);
	fragment_colour = vec4(result, 1);
}