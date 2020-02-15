#version 330

uniform sampler2D s_texture;

in vec3 normal;
in vec2 texCoord;

void main()
{
	vec4 color = texture2D(s_texture, texCoord);
	float gray = color.r + color.g + color.b / 3;
	vec3 grayColor = vec3(gray, gray, gray);

	gl_FragColor = vec4(grayColor, 1.0);
}