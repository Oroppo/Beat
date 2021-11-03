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

	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(GetGameObject()->GetScene()->Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
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

    

    if (_A) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(-0.005f, 0.0f, 0.0f));
    }
    if (_D) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.005f, 0.0f, 0.0f));
    }
    if (_W) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.0f, 0.005f, 0.0f));
    }
    if (_S) {
        GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.0f, -0.005f, 0.0f));
    }

}

