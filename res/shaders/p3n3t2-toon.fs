#version 330

uniform sampler2D s_texture;

in vec3 normal;
in vec2 texCoord;

float divisions = 4;

out vec4 fragColor;

void main()
{
	vec4 color = texture2D(s_texture, texCoord);
	
	vec3 lightDirection = normalize(vec3(1.0, 1.0, 1.0));
	vec3 eyeDirection = normalize(vec3(0, 0, 1));
	vec3 modelNormal = normalize(normal);

	float ambient = 0.2;
	float diffuse = max(dot(modelNormal, lightDirection), ambient);
	vec4 diffuseColor = color * ceil(diffuse * divisions) / divisions;

	vec3 reflection = reflect(-lightDirection, modelNormal);

	float shininess = 100.0;
	float specular = max(pow(dot(reflection, eyeDirection), shininess), 0.0);
	vec4 specularColor = vec4(1.0) * floor(specular * divisions) / divisions;

	fragColor = diffuseColor + specularColor;
}