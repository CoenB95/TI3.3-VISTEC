#version 330

uniform sampler2D s_texture;
uniform sampler2D s_bumpmap;

in vec3 viewLightDirection;
in vec3 viewEyeDirection;
in vec2 texCoord;
in vec3 normal2;

out vec4 fragColor;

void main()
{
	vec4 color = texture2D(s_texture, texCoord);
	
	vec3 lightDirection = normalize(viewLightDirection);
	vec3 eyeDirection = normalize(viewEyeDirection);
	//vec3 normal = normalize(vec3(texture2D(s_bumpmap, texCoord)) * 2.0 - vec3(1.0));
	vec3 normal = normal2;

	float ambient = 0.2;
	float diffuse = max(dot(normal, lightDirection), ambient);
	vec4 diffuseColor = color * diffuse;

	vec3 reflection = reflect(-lightDirection, normal);

	float shininess = 100.0;
	float specular = max(pow(dot(reflection, eyeDirection), shininess), 0.0);
	vec4 specularColor = vec4(1.0) * specular;

	fragColor = vec4(lightDirection, 1.0);//diffuseColor + specularColor;

	if (dot(normal, lightDirection) > 0.9)
		fragColor = vec4(0.0, 1.0, 1.0, 1.0);
}