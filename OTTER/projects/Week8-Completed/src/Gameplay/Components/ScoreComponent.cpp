#include "Gameplay/Components/ScoreComponent.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"

ScoreComponent::ScoreComponent() :
	IComponent()
{ }

ScoreComponent::~ScoreComponent() = default;


void ScoreComponent::Awake() {
	_renderer = GetComponent<RenderComponent>();
}

void ScoreComponent::RenderImGui() { }

nlohmann::json ScoreComponent::ToJson() const {
	return {
	};
}

void ScoreComponent::Update() {
	
	SetScore(_score++);
	std::cout << _score << std::endl;

}

ScoreComponent::Sptr ScoreComponent::FromJson(const nlohmann::json & blob) {
	ScoreComponent::Sptr result = std::make_shared<ScoreComponent>();
	return result;
}
