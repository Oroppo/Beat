#include <iostream>
#include <GLM/glm.hpp>

// If you're curious or want to know more about how I built our ECS(Entity Component System) refer
// to this document: https://austinmorlan.com/posts/entity_component_system/

// We're gonna need to swap GLM/glm.hpp for our own Vec3 class for the sake of efficiency at some point

//					TO DO:
//	Finish Object Loader
//	Finish the Component Manager
//	Create a Test Environment
//	Implement Each Component we need to use
// 
//				Nice To Haves:
//	Implement a Fixed Update Loop for Physics 
//	Implement Dynamic Lighting
//	Optimize Shaders
//	Dynamic Rendering
//	
//

// If you want to add a component, add it here:

struct Transform
{
	//once we decide to use Quaternions we'll swap rotation for a vec4
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

struct RigidBody {

	glm::vec3 velocity;
	glm::vec3 acceleration;

};

struct Gravity {
	int gravityConstant;
};

struct SoundSource {
	
};

struct Camera {

};

struct CharacterController {

};

struct AnimationController {

};

struct EventListener {

};
struct ParticleEmitter {

};