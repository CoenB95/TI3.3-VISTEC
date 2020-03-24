#version 330

uniform sampler2D s_texture;

in vec2 texCoord;

out vec4 fragColor;

void main()
{
	vec2 divisions = vec2(320, 180);
	
	vec2 roundedTexCoord = round(texCoord * divisions) / divisions;
	vec2 positionInPixel = (texCoord - roundedTexCoord) * divisions;
	float distanceToCoord = dot(positionInPixel, positionInPixel);
	
	vec4 color = texture2D(s_texture, roundedTexCoord);

	fragColor = mix(color, vec4(0.0), distanceToCoord * 4.0);
}