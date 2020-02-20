#version 330

uniform sampler2D s_texture;
uniform float time;

in vec3 normal;
in vec2 texCoord;

out vec4 fragColor;

void main()
{
	fragColor = mix(
		texture2D(s_texture, texCoord), 
		texture2D(s_texture, texCoord + vec2(0.5, 0.5)), 0.5 + 0.5 * sin(time));
}