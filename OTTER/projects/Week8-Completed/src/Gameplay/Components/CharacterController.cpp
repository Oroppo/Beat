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
    _platform = "";
}

CharacterController::~CharacterController() = default;

CharacterController::Sptr CharacterController::FromJson(const nlohmann::json & blob) {
    CharacterController::Sptr result = std::make_shared<CharacterController>();
    return result;
}

void CharacterController::OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) {
        LOG_INFO("Body has entered our trigger volume: {}", body->GetGameObject()->Name);
        
        if (_platform != body->GetGameObject()->Name) {
            _canJump = true;
            _platform = body->GetGameObject()->Name;
        }
        body->GetGameObject()->SetRotationZ(90);
}
 void CharacterController::OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body) {
    LOG_INFO("Body has left our trigger volume: {}", body->GetGameObject()->Name);
    if (body->GetGameObject()->Name != "BeatGem") {
        _platform = "";
        _canJump = false;
    }
}
void CharacterController::Update(float deltaTime) {
    bool _A = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_A);
    bool _D = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_D);
    bool _W = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_SPACE);
    
    LOG_INFO(_canJump);
    if ((_platform == "Wall Jump 1")|| (_platform == "Wall Jump 2")) {
        if (_body->GetLinearVelocity().z < 0) {
            _body->ApplyForce(glm::vec3(0.0f, 0.0f, 20.0f));
        }
    }
    glm::vec3 CurrentPosition = GetGameObject()->GetPosition();

    if (_A) {
        _body->SetLinearVelocity(glm::vec3(-3.0f, _body->GetLinearVelocity().y, _body->GetLinearVelocity().z));
    }
    if (_D) {
        _body->SetLinearVelocity(glm::vec3(3.0f,_body->GetLinearVelocity().y, _body->GetLinearVelocity().z));
    }
    if ((_W) && (_canJump == true)) {
        _body->SetLinearVelocity(glm::vec3(_body->GetLinearVelocity().x, _body->GetLinearVelocity().y,_impulse.z));
        _canJump = false;
    }
    if ((!_A) && (!_D) && (!_W) && (_platform != "") && (_platform != "BeatGem")) {
        _body->SetLinearVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
    }
 
    //- glm::vec3( 0.0f,_body->GetLinearVelocity().y,0.0f)
    

    if (GetGameObject()->GetPosition().z <= -14.5f)
    {
        GetGameObject()->SetPostion(glm::vec3(-10.270f, 5.710f, -3.800f));
    }

    GetGameObject()->GetScene()->Lights[1].Position = GetGameObject()->GetPosition();

    _body->SetAngularDamping(100.f);
    _body->SetLinearDamping(0.5f);

    GetGameObject()->SetPositionY(5.61f);
    //GetGameObject()->LockYRotation(70.f);
    //GetGameObject()->LockZRotation(0.f);
    //GetGameObject()->LockXRotation(0.f);

}
