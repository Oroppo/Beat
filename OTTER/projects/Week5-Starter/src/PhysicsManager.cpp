#include <iostream>
#include <vector>
#include "ECS.h"
#include "coordinator.h"
#include "SystemManager.h"

extern Coordinator gCoordinator;

class PhysicsManager {
private:
    std::vector<Entity> m_objects;

public:
    void AddObject(Entity entity) {

        m_objects.push_back(entity);

    }

       
   //  void DestroyObject(Entity entity) {
   //
   //  }
   // 
    void PhysicsManager::Update(float dt)
    {
        for (auto& entity : m_objects)
        {
            auto& rigidBody = gCoordinator.GetComponent<RigidBody>(entity);
            auto& transform = gCoordinator.GetComponent<Transform>(entity);
            auto& gravity = gCoordinator.GetComponent<Gravity>(entity);

            gravity.applyGravity(entity, dt);
        }
       //PhysicsManager::Update(dt);
    }
};