#include "Gameplay/Components/CharacterController.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <iostream>
#include"BeatTimer.h"
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

void CharacterController::OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>&body) {
    LOG_INFO("Body has entered our trigger volume: {}", body->GetGameObject()->Name);
    if ((body->GetGameObject()->Name == "BeatGem") && (_GemJumpTimer>1.25)&&(_GemJumpTimer<1.6666)) {
        _canJump = true;
        std::cout << "jumper worked";
    }
    if ((_platform != body->GetGameObject()->Name )&&(body->GetGameObject()->Name != "BeatGem")){
        _canJump = true;
        _platform = body->GetGameObject()->Name;
    }
    //make certain things fall when touched 
    if (_platform == "Falling Platform") {
        body->SetType(RigidBodyType::Dynamic);
    }
    //rotate half circle platforms
    if (_platform == "Half Circle Platform") {
        _rotPlat = (_body->GetGameObject()->GetPosition()) - body->GetGameObject()->GetPosition();
        body->GetGameObject()->SetRotation(body->GetGameObject()->GetRotationEuler() + glm::vec3(0.0f, -20 * _rotPlat.x, 0.0f));
        LOG_INFO(_rotPlat.x);
    }
   
    //add score
    if ((_platform == "Vinyl")|| (_platform == "CD")) {
        if (_platform == "Vinyl") {
            score += 1000;
        }
        if (_platform == "CD") {
            score += 100;
        }
        body->GetGameObject()->SetPostion(glm::vec3(0.0f, -100.0f, 0.0f));
    }
  
}
void CharacterController::OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>&body) {
    LOG_INFO("Body has left our trigger volume: {}", body->GetGameObject()->Name);

    //maintain ability to jump once left trigger volume if leaving beat gem or falling platform 
    //if neither, loose the ability to jump
    if ((_platform != "BeatGem") || (_platform == "Falling Platform")) {
        _platform = "";
        _canJump = false;
    }
    if (body->GetGameObject()->Name == "Half Circle Platform") {
        LOG_INFO("functions");
        body->GetGameObject()->SetRotation(glm::vec3(-90.000f, 0.0f, 180.0f));
    }
}
void CharacterController::Update(float deltaTime) {
   // LOG_INFO(score);
    bool _A = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_A);
    bool _D = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_D);
    bool _W = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_SPACE);
    _GemJumpTimer = GetGameObject()->GetScene()->FindObjectByName("GameManager")->Get<BeatTimer>()->GetBeatTime();
    LOG_INFO(_GemJumpTimer);
    //   LOG_INFO(_canJump);
    if (_platform == "Wall Jump") {
        if (_body->GetLinearVelocity().z < 0) {
            _body->ApplyForce(glm::vec3(0.0f, 0.0f, 20.0f));
        }
    }
    glm::vec3 CurrentPosition = GetGameObject()->GetPosition();

    if (_A) {
        _body->SetLinearVelocity(glm::vec3(-3.0f, _body->GetLinearVelocity().y, _body->GetLinearVelocity().z));
    }
    if (_D) {
        _body->SetLinearVelocity(glm::vec3(3.0f, _body->GetLinearVelocity().y, _body->GetLinearVelocity().z));
    }
    if ((_W) && (_canJump == true)) {
        _body->SetLinearVelocity(glm::vec3(_body->GetLinearVelocity().x, _body->GetLinearVelocity().y, _impulse.z));
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
    _body->GetGameObject()->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));

}
