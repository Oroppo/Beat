#include <iostream>
#include <vector>
#include "ECS.h"
#include "coordinator.h"
#include "SystemManager.h"
extern Coordinator gCoordinator;
class PhysicsManager {
private:
    std::vector<Object> m_objects;

public:
    void AddObject(Object object) {
        m_objects.push_back(object);
    }
  // void DestroyObject(Object* object) {
  //     if (!object) return;
  //     auto itr = std::find(m_objects.begin(), m_objects.end(), object);
  //     if (itr == m_objects.end()) return;
  //     m_objects.erase(itr);
  // }
    
    void PhysicsManager::Update(Entity entity)
    {
            float dt = 0.0f;
            auto& rigidBody = gCoordinator.GetComponent<RigidBody>(entity);
            auto& transform = gCoordinator.GetComponent<Object>(entity);
            auto& gravity = gCoordinator.GetComponent<Gravity>(entity);

            RigidBody* rigidBodyp = &rigidBody;
            Gravity* gravityp = &gravity;
            Object* transformp = &transform;
            gravity.applyGravity(transformp,rigidBodyp,dt);
    }
};