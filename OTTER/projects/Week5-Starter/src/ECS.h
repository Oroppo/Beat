#include <iostream>
#include <GLM/glm.hpp>

// If you're curious or want to know more about how I built our ECS(Entity Component System) refer
// to this document: https://austinmorlan.com/posts/entity_component_system/

// We're gonna need to swap GLM/glm.hpp for our own Vec3 class for the sake of efficiency at some point

//                    TO DO:
//    Finish Object Loader        [In progress...]
//    Finish the Component Manager [Done!]
//    Create a Test Environment    [In Progress...]
//    Implement Each Component we need to use
//    Build a physics system
//    Build an obfuscated Render Component
// 
//                Nice To Haves:
//    Implement a Fixed Update Loop for Physics 
//    Implement Dynamic Lighting
//    Window Event System (currently using GLFW's Event Listener glfwPollEvents() which is pretty limiting)
//    Optimize Shaders
//    Dynamic Rendering
//    Particle System (basically just uses OBJ loader + interesting use of shaders)
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
    RigidBody() {

        force = glm::vec3(0.0f, 0.0f, 0.0f);
        velocity = glm::vec3(0.0f, 0.0f, 0.0f);
        acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
        mass = 10.0f;
    }
    glm::vec3 force;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float mass;


    //since the only thing that will be colliding with anythin else willbe the player, check if the player is colliding 
    bool CubicCollisionDetection(Transform* ColliderStats, RigidBody* ColliderBody, Transform* ObjectStats, RigidBody* ObjectBody) {

        //check if colliding in x axis
        if (
            
            (ColliderStats->position.x+ColliderStats->scale.x/2>ObjectStats->position.x)&&(ColliderStats->position.x<ObjectStats->position.x+ObjectStats->scale.x/2)&&
            //check if colliding in y axis
            (ColliderStats->position.y+ColliderStats->scale.y,2>ObjectStats->position.y)&&(ColliderStats->position.y<ObjectStats->position.y+ObjectStats->scale.y/2)&&
            //check if colliding in z axis
            (ColliderStats->position.z + ColliderStats->scale.z/2 > ObjectStats->position.z) && (ColliderStats->position.z < ObjectStats->position.z + ObjectStats->scale.z/2)
           
            )
        {
            return true;
        }
        else {
            return false;
        }
     
    }
    bool SphericalCollisionDetection() {
        return true;
    }

};

struct Gravity {

    glm::vec3  grav = glm::vec3(0.0f, 9.81, 0.0f);

     void applyGravity(Transform* transform, RigidBody* body, float dt) {

        //force+=mass*grav
        body->force += body->mass * grav;
        //Velocity+=force*dt
        body->velocity += body->force * dt;
        //position+=velocity
        transform->position += body->velocity;

        //reset force applied by grav so it doesnt compound twice
        body->force = glm::vec3(0.0f, 0.0f, 0.0f);

    }
};

struct SoundSource {
    //fmod code here
};

//struct Camera {
//    glm::vec3 viewProjection;
//    bool isOrtho;
//};

struct CharacterController {

};

struct AnimationController {

};

struct EventListener {

};
struct ParticleEmitter {

};