#version 330

uniform sampler2D s_texture;
uniform float time;

in vec2 texCoord;

out vec4 fragColor;

void main()
{
	float f = 0.0005;

	fragColor += texture2D(s_texture, texCoord + vec2(-3 * f, 0)) * 1.0;
	fragColor += texture2D(s_texture, texCoord + vec2(-2 * f, 0)) * 2.0;
	fragColor += texture2D(s_texture, texCoord + vec2(-1 * f, 0)) * 3.0;
	fragColor += texture2D(s_texture, texCoord + vec2( 0 * f, 0)) * 4.0;
	fragColor += texture2D(s_texture, texCoord + vec2( 1 * f, 0)) * 3.0;
	fragColor += texture2D(s_texture, texCoord + vec2( 2 * f, 0)) * 2.0;
	fragColor += texture2D(s_texture, texCoord + vec2( 3 * f, 0)) * 1.0;
	fragColor /= 16;
}