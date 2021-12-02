#include "Gameplay/Components/ScoreComponent.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"

ScoreComponent::ScoreComponent() :
	IComponent(),
	_renderer(nullptr)
{ }

ScoreComponent::~ScoreComponent() = default;


void ScoreComponent::Awake() {

	_renderer = GetComponent<RenderComponent>();

}

void ScoreComponent::RenderImGui() { }

nlohmann::json ScoreComponent::ToJson() const {
	return {};
}

void ScoreComponent::Update() {
	

}

ScoreComponent::Sptr ScoreComponent::FromJson(const nlohmann::json & blob) {
	ScoreComponent::Sptr result = std::make_shared<ScoreComponent>();
	return result;
}
