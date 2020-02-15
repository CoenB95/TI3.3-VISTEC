#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_color;

uniform mat4 modelViewProjectionMatrix;
uniform float time = 0;

out vec3 color;

void main()
{
	color = a_color;
	gl_Position = modelViewProjectionMatrix * vec4(a_position,1);
}