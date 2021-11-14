#include "Gameplay/Components/CharacterController.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <iostream>
void CharacterController::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();
    if (_body == nullptr) {
        IsEnabled = false;
    }

}


void CharacterController::RenderImGui() {

}

nlohmann::json CharacterController::ToJson() const {
    return {
        { "impulse", _impulse2 }
    };
}

CharacterController::CharacterController() :
    IComponent()
{
    _canJump = true;
}

CharacterController::~CharacterController() = default;

CharacterController::Sptr CharacterController::FromJson(const nlohmann::json & blob) {
    CharacterController::Sptr result = std::make_shared<CharacterController>();
    return result;
}
void CharacterController::OnTriggerEnter(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
    _canJump = true;
}
void CharacterController::OnTriggerExit(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
    _canJump = false;
}
void CharacterController::Update(float deltaTime) {

    bool _A = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_A);
    bool _S = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_S);
    bool _D = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_D);
    bool _W = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_SPACE);



    glm::vec3 CurrentPosition = GetGameObject()->GetPosition();

    if (_A) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(-0.02f, 0.0f, 0.0f));
    }
    if (_D) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.02f, 0.0f, 0.0f));
    }
    if ((_W) && (_canJump == true)) {
        _body->ApplyImpulse(_impulse);

    }
    if (_S) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.0f, -0.02f, 0.0f));
    }


    GetGameObject()->GetScene()->Lights[1].Position = GetGameObject()->GetPosition();

    _body->SetAngularDamping(100.f);
    _body->SetLinearDamping(0.5f);

    GetGameObject()->LockYPosition(5.61f);
    //GetGameObject()->LockYRotation(70.f);
    //GetGameObject()->LockZRotation(0.f);
    //GetGameObject()->LockXRotation(0.f);

}
