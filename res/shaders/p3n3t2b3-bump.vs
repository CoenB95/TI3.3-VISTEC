#version 330

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_texcoord;
layout (location = 3) in vec3 a_tangent;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform float time;

out vec3 viewLightDirection;
out vec3 viewEyeDirection;
out vec2 texCoord;
out vec3 normal2;

void main()
{
	vec3 viewVertexPosition = mat3(modelViewMatrix) * a_position;
	vec3 viewVertexNormal = mat3(modelViewMatrix) * normalize(a_normal);
	vec3 viewVertexTangent = mat3(modelViewMatrix) * normalize(a_tangent);
	normal2 = normalMatrix * a_normal;

	vec3 viewVertexBitangent = cross(viewVertexNormal, viewVertexTangent);

	mat3 TBN = transpose(mat3(
		viewVertexTangent,
		viewVertexBitangent,
		viewVertexNormal
	));

	vec3 viewLightPosition = vec3(0.0, 0.0, 1.0); - viewVertexPosition;
	viewLightDirection = TBN * viewLightPosition;

	vec3 viewEyePosition = -viewVertexPosition;
	viewEyeDirection = TBN * viewEyePosition;

	texCoord = a_texcoord;

	gl_Position = projectionMatrix * modelViewMatrix * vec4(a_position, 1.0);
}