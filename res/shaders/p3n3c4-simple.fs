#version 330

uniform float time = 0;

in vec3 color;

out vec3 fragColor;

void main()
{
	fragColor = color;
}