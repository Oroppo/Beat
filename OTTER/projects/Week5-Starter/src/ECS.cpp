#include <iostream>
#include <GLM/glm.hpp>

// If you're curious or want to know more about how I built our ECS(Entity Component System) refer
// to this document: https://austinmorlan.com/posts/entity_component_system/

// We're gonna need to swap GLM/glm.hpp for our own Vec3 class for the sake of efficiency at some point

//					TO DO:
//	Finish Object Loader		[In progress...]
//	Finish the Component Manager [Done!]
//	Create a Test Environment	[In Progress...]
//	Implement Each Component we need to use
//	Build a physics system
//	Build an obfuscated Render Component
// 
//				Nice To Haves:
//	Implement a Fixed Update Loop for Physics 
//	Implement Dynamic Lighting
//	Window Event System (currently using GLFW's Event Listener glfwPollEvents() which is pretty limiting)
//	Optimize Shaders
//	Dynamic Rendering
//	Particle System (basically just uses OBJ loader + interesting use of shaders)
//

// If you want to add a component, add it here:

struct Transform
{
	Transform(glm::vec3 ) {

	}
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
	glm::vec3  force;
};

struct SoundSource {
	//fmod code here
};

struct Camera {
	glm::vec3 viewProjection;
	bool isOrtho;
};

struct CharacterController {

};

struct AnimationController {

};

struct EventListener {

};
struct ParticleEmitter {

};