#version 330

uniform sampler2D s_texture;

in vec3 normal;
in vec2 texCoord;

out vec4 fragColor;

void main()
{
	vec4 color = texture2D(s_texture, texCoord);
	
	vec3 lightDirection = normalize(vec3(1,1,1));
	vec3 viewDirection = vec3(0,0,1);
	float shininess = 100.0;

	float ambient = 0.2;
	float diffuse = 0.8 * dot(normalize(normal), lightDirection);

	vec3 r = reflect(-lightDirection, normalize(normal));

	float specular = pow(max(0.0, dot(r, viewDirection)), shininess);

	float factor = ambient + diffuse + specular;

	fragColor = color * vec4(factor, factor, factor, 1.0);
}