#include <iostream>
#include <GLM/glm.hpp>
#include "Coordinator.h"

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
extern Coordinator gCoordinator;

struct Transform
{
    //once we decide to use Quaternions we'll swap rotation for a vec4
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    float dynamicFriction;
    void SetObjectToSleep() {
        position = glm::vec3(-100.0f, -100.0f, -100.0f);
    }
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

    //since the only thing that will be colliding with anythin else will be the player, check if the player is colliding 
    bool CubicCollisionDetection(Transform ColliderStats, Transform ObjectStats) {
        if (
            //check if colliding in x axis
            (ColliderStats.position.x + ColliderStats.scale.x / 2 > ObjectStats.position.x) && (ColliderStats.position.x < ObjectStats.position.x + ObjectStats.scale.x / 2) &&
            //check if colliding in y axis
            (ColliderStats.position.y + ColliderStats.scale.y / 2 > ObjectStats.position.y) && (ColliderStats.position.y < ObjectStats.position.y + ObjectStats.scale.y / 2) &&
            //check if colliding in z axis
            (ColliderStats.position.z + ColliderStats.scale.z / 2 > ObjectStats.position.z) && (ColliderStats.position.z < ObjectStats.position.z + ObjectStats.scale.z / 2)
            )
        {
            //collision is true 
            return true;
        }
        else {
            return false;
        }
    }
    //same as cubic but for balls ;)
    bool SphericalCollisionDetection(Transform ColliderStats, Transform ObjectStats) {
        return true;
    }

    //Collision type: 0.0f==Cubic, 1.0f = Spherical
    bool CollisionDetection(Transform ColliderStats, Transform ObjectStats, int CollisionType) {
        switch (CollisionType) {
        case 0:
            return CubicCollisionDetection(ColliderStats, ObjectStats);
        case 1:
            return SphericalCollisionDetection(ColliderStats, ObjectStats);
        }
    }

    //in case the object does not move
    void ApplyStaticCollision(Entity TargetEntity, Entity StaticObject) {
        auto& transformT = gCoordinator.GetComponent<Transform>(TargetEntity);
        auto& transformO = gCoordinator.GetComponent<Transform>(StaticObject);

        //default to cubic detection for the time being
        if (CollisionDetection(transformT, transformO, 0) == true) {
            glm::vec3 difference = transformT.position - transformO.position;
            transformT.position -= difference;
            //above should work but im kinda dumb so im using a cout statement to make sure im right

        }
    }
    // in case the object moves
    void ApplyDynamicCollision() {

    }
    void ApplyForce(glm::vec3 force)
    {
        velocity += force;
    }
    void StopMotion(Entity entity)
    {
        velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    }

};

struct Gravity {

    glm::vec3 grav = glm::vec3(0.0f, 9.81, 0.0f);

    void applyGravity(Entity entity, float dt) {
        auto& body = gCoordinator.GetComponent<RigidBody>(entity);
        auto& transform = gCoordinator.GetComponent<Transform>(entity);
        //force+=mass*grav
        body.force += body.mass * grav;
        //Velocity+=force*dt
        body.velocity += body.force * dt;
        //position+=velocity
        transform.position += body.velocity;

        //reset force applied by grav so it doesnt compound twice
        body.force = glm::vec3(0.0f, 0.0f, 0.0f);

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


struct ParticleEmitter {

};