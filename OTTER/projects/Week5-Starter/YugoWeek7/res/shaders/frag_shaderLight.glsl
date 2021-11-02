#version 410

layout(location = 1) in vec3 inColor;
//Lecture 5 ///////
layout(location = 0) in vec3 inPos;
layout(location = 2) in vec3 inNormal;

uniform vec3 lightPos;
uniform vec3 cameraPos;
///////////
out vec4 frag_color;

void main() { 
	
	///// lecture 5 //////// --- Ambient Component
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor * inColor;

	// Diffuse component --- need normal and light direction
	vec3 Normal = normalize(inNormal); // this is our normal direction normalized
	vec3 lightDir = normalize(lightPos - inPos); // this is our light direction 

	// d for diffuse dot product between the light direction and the normal, the angle between them will give us intensity
	float d = max(dot(Normal, lightDir), 0.0); // we don't want negative diffuse so we use max and 0.0
	vec3 diffuse = d * inColor;

	//Attenuation for lighting with distance
	// distance between light and point
	float dist = length(lightPos - inPos);
	diffuse = diffuse / dist*dist;


	//Specular
	float specularStrength  = 1.0;
	vec3 camDir = normalize(cameraPos - inPos);
	vec3 reflectedRay = reflect(-lightDir, Normal); // light direction to the point 
	// Blinn Phong Lighting Using Halfway vector between view/light
	// Make a halfway vector by getting the normal between camera/view direction + light direction
	vec3 Halfway = normalize(camDir + lightDir);
	// do the same calculation as phong but this time use the normal from surface and halfway vector in calculation
	float spec = pow(max(dot(Normal, Halfway), 0.0), 64); // 64 is the shininess coefficent
	vec3 specular = specularStrength * spec * lightColor;
	// Phong Lighting
	/*float spec = pow(max(dot(camDir, reflectedRay), 0.0), 8); // 8 is the shininess coefficent
	vec3 specular = specularStrength * spec * lightColor;*/

	vec3 result = ambient + diffuse + specular;
	frag_color = vec4(result, 1.0);

}