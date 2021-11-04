#include "Gameplay/Components/CharacterController.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

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
        { "impulse", _impulse }
    };
}

CharacterController::CharacterController() :
	IComponent()
{ }

CharacterController::~CharacterController() = default;

CharacterController::Sptr CharacterController::FromJson(const nlohmann::json & blob) {
	CharacterController::Sptr result = std::make_shared<CharacterController>();
	return result;
}

void CharacterController::Update(float deltaTime) {

    bool _A = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_A);
    bool _S = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_S);
    bool _D = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_D);
    bool _W = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_W);

    glm::vec3 CurrentPosition = GetGameObject()->GetPosition();

    if (_A) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(-0.02f, 0.0f, 0.0f));
    }
    if (_D) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.02f, 0.0f, 0.0f));
    }
    if (_W) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.0f, 0.02f, 0.0f));
    }
    if (_S) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.0f, -0.02f, 0.0f));
    }

    if (GetGameObject()->GetPosition().x < 0.775) {
        GetGameObject()->SetPostion(glm::vec3(0.780f, CurrentPosition.y, CurrentPosition.z));
    }
    if (GetGameObject()->GetPosition().x > 1.40) {
        GetGameObject()->SetPostion(glm::vec3(1.395f, CurrentPosition.y, CurrentPosition.z));
    }
    if (GetGameObject()->GetPosition().y > 0.4) {
        GetGameObject()->SetPostion(glm::vec3(CurrentPosition.x, 0.395f, CurrentPosition.z));
    }
    if (GetGameObject()->GetPosition().y < -0.4) {
        GetGameObject()->SetPostion(glm::vec3(CurrentPosition.x, -0.395f, CurrentPosition.z));
    }

    GetGameObject()->GetScene()->Lights[1].Position = GetGameObject()->GetPosition();
}

