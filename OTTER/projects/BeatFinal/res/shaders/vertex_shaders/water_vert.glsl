#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

uniform vec3 u_WaveDirection;
uniform float u_WaveStrength;

uniform float u_WaveSpeed;

void main() {

	// Calculate the output world position
	outWorldPos = (u_Model * vec4(inPosition, 1.0)).xyz;

    // Determine the offset based on our simple wind calcualtion
    vec3 windFactor = inPosition;
	windFactor.z = sin(windFactor.x * u_WaveSpeed + u_Time) * u_WaveStrength;	



    // Project the world position to determine the screenspace position
	gl_Position = u_ModelViewProjection * vec4(windFactor, 1);

	// Normals
	outNormal = mat3(u_NormalMatrix) * inNormal;
	// Pass our UV coords to the fragment shader
	outUV = inUV;
	outColor = inColor;
}

