#version 330

uniform sampler2D s_texture;
uniform float time;

in vec2 texCoord;

out vec4 fragColor;

void main()
{
	fragColor = texture2D(s_texture, texCoord + vec2(time,0));
}