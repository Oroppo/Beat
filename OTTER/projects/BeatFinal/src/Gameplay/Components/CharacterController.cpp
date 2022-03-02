#include "Gameplay/Components/CharacterController.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <iostream>
#include"BeatTimer.h"
#include "RenderComponent.h"
#include "GUI/GuiPanel.h"
#include "GUI/GuiText.h"
#include "FMOD/SoundEffects.h"

#include<sstream>  
#include<string.h>
void CharacterController::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();
    if (_body == nullptr) {
        IsEnabled = false;
    }

    SFXS.SetVolume("event:/Coin Pickup", 0.25f);
    SFXS.SetVolume("event:/Jump", 0.25f);
    SFXS.SetVolume("event:/Death", 0.25f);
    SFXS.SetVolume("event:/Walk", 0.25f);
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

//for collectibles
void CharacterController::OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger) {

    std::string name = trigger->GetGameObject()->Name;


    if ((name[0] == 'B')&& (name[1] == 'e')&& (name[2] == 'a')&& (name[3] == 't')&& (name[4] == 'G')) {
        int beatNumber = (int)name[8] - 48;

        if ((_GemJumpTimer > 0.6 * beatNumber - 0.6) && (_GemJumpTimer < 0.6 * beatNumber)) {
            _canJump = true;
            std::cout << "jumper worked";

            trigger->GetGameObject()->Get<RenderComponent>()->IsEnabled = false;
            _BeatGemHits++;
            score += 500;
            SFXS.PlayEvent("event:/Coin Pickup");
            
        }

    }
        if (trigger->GetGameObject()->Name == "Vinyl") {
            score += 1000;
            _VinylScore++;
            SFXS.PlayEvent("event:/Coin Pickup");
        }
        if (trigger->GetGameObject()->Name == "CD") {
            score += 100;
            _CDScore++;
            SFXS.PlayEvent("event:/Coin Pickup");
        }
        std::stringstream ss;
        ss << score;
        std::string stringScore;
        ss >> stringScore;
        trigger->GetGameObject()->SetPostion(glm::vec3(0.0f, -100.0f, 0.0f));
        trigger->GetGameObject()->GetScene()->FindObjectByName("GameOver Score Text")->Get<RectTransform>()->SetPosition({425 , 100});
        GetGameObject()->GetScene()->FindObjectByName("GameOver Score Text")->Get<GuiText>()->SetText(stringScore);
    
}

//for physical platforms
void CharacterController::OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>&body) {
    LOG_INFO("Body has entered our trigger volume: {}", body->GetGameObject()->Name);
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

    bool _A = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_A);
    bool _D = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_D);
    bool _W = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_SPACE);
    _GemJumpTimer = GetGameObject()->GetScene()->FindObjectByName("GameManager")->Get<BeatTimer>()->GetBeatTime();
    std::cout << " Beat Gem timer " << _GemJumpTimer<<std::endl;
    if (_platform == "Wall Jump") {
        if (_body->GetLinearVelocity().z < 0) {
            _body->ApplyForce(glm::vec3(0.0f, 0.0f, 20.0f));
        }
    }
    glm::vec3 CurrentPosition = GetGameObject()->GetPosition();

    if (_A) {
        _body->SetLinearVelocity(glm::vec3(-3.0f, _body->GetLinearVelocity().y, _body->GetLinearVelocity().z));
        SFXS.PlayEvent("event:/Walk");
    }
    if (_D) {
        _body->SetLinearVelocity(glm::vec3(3.0f, _body->GetLinearVelocity().y, _body->GetLinearVelocity().z));
    }
    if ((_W) && (_canJump == true)) {
        _body->SetLinearVelocity(glm::vec3(_body->GetLinearVelocity().x, _body->GetLinearVelocity().y, _impulse.z));
        _canJump = false;
        SFXS.PlayEvent("event:/Jump");

    }
    if ((!_A) && (!_D) && (!_W) && (_platform != "") && (_platform != "BeatGem")) {
        _body->SetLinearVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
    }

    GetGameObject()->SetPositionY(5.61f);
    _body->GetGameObject()->SetRotation(glm::vec3(90.0f, 0.0f, 90.0f));


    if (GetGameObject()->GetPosition().z <= -14.5f)
    {
        SFXS.PlayEvent("event:/Death");
        // Activate GameOver U.I. When the player dies! 
      //GetGameObject()->GetScene()->FindObjectByName("GameOver Dimmed Background")->Get<GuiPanel>()->IsEnabled = (GetGameObject()->GetScene()->FindObjectByName("GameOver Dimmed Background")->Get<GuiPanel>()->IsEnabled) = true;
      //GetGameObject()->GetScene()->FindObjectByName("GameOver Text")->Get<GuiPanel>()->IsEnabled = (GetGameObject()->GetScene()->FindObjectByName("GameOver Text")->Get<GuiPanel>()->IsEnabled) = true;
      //GetGameObject()->GetScene()->FindObjectByName("GameOver Score Breakdown")->Get<GuiPanel>()->IsEnabled = GetGameObject()->GetScene()->FindObjectByName("GameOver Score Breakdown")->Get<GuiPanel>()->IsEnabled = true;
      //GetGameObject()->GetScene()->FindObjectByName("GameOver Quit Button")->Get<GuiPanel>()->IsEnabled = GetGameObject()->GetScene()->FindObjectByName("GameOver Quit Button")->Get<GuiPanel>()->IsEnabled = true;
      //GetGameObject()->GetScene()->FindObjectByName("GameOver Continue Button")->Get<GuiPanel>()->IsEnabled = GetGameObject()->GetScene()->FindObjectByName("GameOver Continue Button")->Get<GuiPanel>()->IsEnabled = true;
      //GetGameObject()->GetScene()->FindObjectByName("GameOver Score Text")->Get<GuiText>()->IsEnabled = GetGameObject()->GetScene()->FindObjectByName("GameOver Score Text")->Get<GuiText>()->IsEnabled = true;
    }
}
    

    //GetGameObject()->GetScene()->Lights[1].Position = GetGameObject()->GetPosition();


