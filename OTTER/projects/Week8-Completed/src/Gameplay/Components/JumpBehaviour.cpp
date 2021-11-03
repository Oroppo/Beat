#include "Gameplay/Components/JumpBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void JumpBehaviour::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();
	if (_body == nullptr) {
		IsEnabled = false;
	}
}

void JumpBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
}

nlohmann::json JumpBehaviour::ToJson() const {
	return {
		{ "impulse", _impulse }
	};
}

JumpBehaviour::JumpBehaviour() :
	IComponent(),
	_impulse(1.2f)
{ }

JumpBehaviour::~JumpBehaviour() = default;

JumpBehaviour::Sptr JumpBehaviour::FromJson(const nlohmann::json& blob) {
	JumpBehaviour::Sptr result = std::make_shared<JumpBehaviour>();
	result->_impulse = blob["impulse"];
	return result;
}

void JumpBehaviour::Update(float deltaTime) {

	bool pressed = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_SPACE);
	if (pressed) {
	if (pressed) 
		if (_isPressed == false) {
			_body->ApplyImpulse(glm::vec3(0.0f, 0.0f, _impulse));
		}
		_isPressed = pressed;
	} else {
		_isPressed = false;
	}

	bool APressed = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_A);
	bool SPressed = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_S);
	bool DPressed = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_D);
	bool WPressed = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_W);
	if (APressed) {
		GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(-0.005f, 0.0f, 0.0f));
	}
	if (DPressed) {
		GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.005f, 0.0f, 0.0f));
	}
	if (WPressed) {
		GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.0f, 0.005f, 0.0f));
	}
	if (SPressed) {
		GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(0.0f, -0.005f, 0.0f));
	}

}

