#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"

MaterialSwapBehaviour::MaterialSwapBehaviour() :
	IComponent(),
	_renderer(nullptr),
	EnterMaterial(nullptr),
	ExitMaterial(nullptr)
{ }
MaterialSwapBehaviour::~MaterialSwapBehaviour() = default;


void MaterialSwapBehaviour::Awake() {
	_renderer = GetComponent<RenderComponent>();
}

void MaterialSwapBehaviour::RenderImGui() { }

nlohmann::json MaterialSwapBehaviour::ToJson() const {
	return {
		{ "enter_material", EnterMaterial != nullptr ? EnterMaterial->GetGUID().str() : "null" },
		{ "exit_material", ExitMaterial != nullptr ? ExitMaterial->GetGUID().str() : "null" }
	};
}

MaterialSwapBehaviour::Sptr MaterialSwapBehaviour::FromJson(const nlohmann::json& blob) {
	MaterialSwapBehaviour::Sptr result = std::make_shared<MaterialSwapBehaviour>();
	result->EnterMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["enter_material"]));
	result->ExitMaterial  = ResourceManager::Get<Gameplay::Material>(Guid(blob["exit_material"]));
	return result;
}
void Update(float dt) 
{

	//check if condition true

	//Swap Material here
	//_renderer->SetMaterial(EnterMaterial);

}
