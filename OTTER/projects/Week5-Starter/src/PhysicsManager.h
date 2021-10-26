#include <iostream>
#include <vector>
#include "ECS.h"
#include "coordinator.h"

extern Coordinator gCoordinator;

class PhysicsManager {
private:
    std::vector<Entity> m_objects;

public:
    void AddObject(Entity entity) {

        m_objects.push_back(entity);

    }

       
    void DestroyObject(Entity entity) {
    //empty for now, destroys entity with this specifier
    }
    
    void PhysicsManager::Update(float dt)
    {
        for (Entity entity : m_objects)
        {
            auto& rigidBody = gCoordinator.GetComponent<RigidBody>(entity);
            auto& transform = gCoordinator.GetComponent<Transform>(entity);
            auto& gravity = gCoordinator.GetComponent<Gravity>(entity);

            //gravity.applyGravity(entity);
        }

       PhysicsManager::Update(dt);
    }
};