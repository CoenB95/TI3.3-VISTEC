#version 330

uniform sampler2D s_texture;

in vec3 normal;
in vec2 texCoord;

float divisions = 4;

void main()
{
	vec4 color = texture2D(s_texture, texCoord);
	
	vec3 modelNormal = normalize(normal);
	vec3 lightDirection = normalize(vec3(1, 1, 1));
	vec3 viewDirection = vec3(0, 0, 1);
	float shininess = 100.0;

	float ambient = 0.2;
	float diffuse = 0.8 * dot(modelNormal, lightDirection);

	vec3 reflection = reflect(-lightDirection, modelNormal);

	float specular = pow(max(0.0, dot(reflection, viewDirection)), shininess);

	float factor = floor((ambient + diffuse + specular) * divisions) / divisions;

	gl_FragColor = color * vec4(factor, factor, factor, 1.0);
}