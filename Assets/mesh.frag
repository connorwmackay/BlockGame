#version 450
#extension GL_EXT_texture_array : enable

out vec4 fColour;
in vec2 TexCoord;
flat in int TextureAtlasIndex;
in vec3 Normal;
in vec3 FragPos;

struct DirectionalLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform sampler2DArray texture1;
uniform DirectionalLight directionalLight;
uniform vec3 viewPos;

void main() {
	// Use a generic material for now.
	Material material;
	material.ambient = vec3(1.0f, 1.0f, 1.0f);
	material.diffuse = vec3(1.0f, 0.5f, 0.31f);
	material.specular = vec3(0.5f, 0.5f, 0.5f);
	material.shininess = 32.0f;

	// Determine the light direction
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(-directionalLight.direction);
	
	// Determine the diffuse
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = directionalLight.diffuse * (diff * material.diffuse);

	// Specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = directionalLight.specular * (spec * material.specular);

	// Ambient
	vec3 ambient = directionalLight.ambient * material.ambient;

	// Resulting Colour
	vec3 lightingResult = ambient + diffuse + specular;
	vec4 result = vec4(lightingResult, 1.0);
	fColour = texture2DArray(texture1, vec3(TexCoord, TextureAtlasIndex)) * result;
}