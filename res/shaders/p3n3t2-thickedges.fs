#version 330

uniform sampler2D s_texture;
uniform float time;

in vec2 texCoord;

out vec4 fragColor;

void main()
{
	float f = 0.0005;
	vec4 color;
	
	// Edge detection
	color += texture2D(s_texture, texCoord + vec2( 0 * f, -1 * f)) * -1.0;
	color += texture2D(s_texture, texCoord + vec2(-1 * f,  0 * f)) * -1.0;
	color += texture2D(s_texture, texCoord + vec2( 1 * f,  0 * f)) * -1.0;
	color += texture2D(s_texture, texCoord + vec2( 0 * f,  1 * f)) * -1.0;
	color += texture2D(s_texture, texCoord + vec2( 0 * f, 0 * f)) * 4.0;
	float edge = (color.r + color.g + color.b) / 3.0;

	vec4 edgeColor = vec4(1.0);
	vec4 pixelColor = texture2D(s_texture, texCoord);

	fragColor = mix(pixelColor, edgeColor, edge > 0.2 ? 1.0 : 0.0);
}