#version 440

#include "../fragments/fs_common_inputs.glsl"


layout(location = 7) in vec3 inTextureWeights;

layout(location = 8) in float inDisplacement;

layout(location = 0) out vec4 frag_color;

#include "../fragments/frame_uniforms.glsl"

struct Material {
	sampler2D DiffuseA;
	sampler2D DiffuseB;
	sampler2D DiffuseC;

	float     Shininess;
};
// Create a uniform for the material
uniform Material u_Material;

uniform sampler2D s_NormalMap;

#include "../fragments/multiple_point_lights.glsl"

const float LOG_MAX = 2.40823996531;

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
void main() {

	 vec3 normal = normalize(inNormal);
	vec3 lightAccumulation = CalcAllLightContribution(inWorldPos, normal, u_CamPos.xyz, u_Material.Shininess);

    vec3 texWeight = inTextureWeights / dot(inTextureWeights, vec3(1,1,1));



	if (inWorldPos.z < 0.1)
	{
		texWeight.x = 1.0f;
		texWeight.y = 0.0f;
		texWeight.z = 0.0f;
	}

	if (inWorldPos.z >= 0.1 && inWorldPos.z < 0.2) 
	{
		texWeight.x = 0.0f;
		texWeight.y = 1.0f;
		texWeight.z = 0.0f;
	}

	if (inWorldPos.z >= 2) 
	{
		texWeight.x = 0.0f;
		texWeight.y = 0.0f;
		texWeight.z = 1.0f;
	}


	// calculate all the textures added together along with weights
	vec4 textureColor = 
        texture(u_Material.DiffuseA, inUV) * texWeight.x +
        texture(u_Material.DiffuseB, inUV) * texWeight.y +
		texture(u_Material.DiffuseC, inUV) * texWeight.z;


	// combine for the final result
	vec3 result = lightAccumulation  * inColor * textureColor.rgb;

	frag_color = vec4(result, textureColor.a);
}