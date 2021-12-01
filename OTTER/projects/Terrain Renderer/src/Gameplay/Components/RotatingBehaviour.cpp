#include "Gameplay/Components/RotatingBehaviour.h"

#include "Gameplay/GameObject.h"

#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"

void RotatingBehaviour::Update(float deltaTime) {
	GetGameObject()->SetRotation(GetGameObject()->GetRotationEuler() + RotationSpeed * deltaTime);
}

void RotatingBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat3, "Speed", &RotationSpeed.x);
}

nlohmann::json RotatingBehaviour::ToJson() const {
	return {
		{ "speed", GlmToJson(RotationSpeed) }
	};
}

RotatingBehaviour::Sptr RotatingBehaviour::FromJson(const nlohmann::json& data) {
	RotatingBehaviour::Sptr result = std::make_shared<RotatingBehaviour>();
	result->RotationSpeed = ParseJsonVec3(data["speed"]);
	return result;
}

// ROTATION BEHAVIOURS FOR THE CDs

void RotatingBehaviourCD::Update(float deltaTime) {
	GetGameObject()->SetRotation(GetGameObject()->GetRotationEuler() + RotationCDSpeed * deltaTime);
}

void RotatingBehaviourCD::RenderImGui() {
	LABEL_LEFT2(ImGui::DragFloat3, "Speed", &RotationCDSpeed.x);
}

nlohmann::json RotatingBehaviourCD::ToJson() const {
	return {
		{ "speed", GlmToJson(RotationCDSpeed) }
	};
}

RotatingBehaviourCD::Sptr RotatingBehaviourCD::FromJson(const nlohmann::json& data) {
	RotatingBehaviourCD::Sptr result = std::make_shared<RotatingBehaviourCD>();
	result->RotationCDSpeed = ParseJsonVec3(data["speed"]);
	return result;
}

