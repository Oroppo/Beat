//#include "Gameplay/Components/ScoreComponent.h"
//#include "Gameplay/Components/ComponentManager.h"
//#include "Gameplay/GameObject.h"
//
//ScoreComponent::ScoreComponent() :
//	IComponent(),
//	_renderer(nullptr),
//	EnterMaterial(nullptr),
//	ExitMaterial(nullptr)
//
//
//{ }
//ScoreComponent::~ScoreComponent() = default;
//
//void ScoreComponent::OnEnteredTrigger(const Gameplay::Physics::TriggerVolume::Sptr & trigger) {
//
//	if (_renderer && EnterMaterial) {
//		_renderer->SetMaterial(EnterMaterial);
//	}
//
//	LOG_INFO("Entered trigger: {}", trigger->GetGameObject()->Name);
//}
//
//void ScoreComponent::OnLeavingTrigger(const Gameplay::Physics::TriggerVolume::Sptr & trigger) {
//	if (_renderer && ExitMaterial) {
//		_renderer->SetMaterial(ExitMaterial);
//	}
//	LOG_INFO("Exit trigger: {}", trigger->GetGameObject()->Name);
//}
//
//void ScoreComponent::Awake() {
//	_renderer = GetComponent<RenderComponent>();
//}
//
//void ScoreComponent::RenderImGui() { }
//
//nlohmann::json ScoreComponent::ToJson() const {
//	return {
//		{ "enter_material", EnterMaterial != nullptr ? EnterMaterial->GetGUID().str() : "null" },
//		{ "exit_material", ExitMaterial != nullptr ? ExitMaterial->GetGUID().str() : "null" }
//	};
//}
//
//ScoreComponent::Sptr ScoreComponent::FromJson(const nlohmann::json & blob) {
//	ScoreComponent::Sptr result = std::make_shared<ScoreComponent>();
//	result->EnterMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["enter_material"]));
//	result->ExitMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["exit_material"]));
//	return result;
//}
