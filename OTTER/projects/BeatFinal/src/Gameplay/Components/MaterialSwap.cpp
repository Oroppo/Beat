#include "Gameplay/Components/MaterialSwap.h"
#include "Gameplay/Components/ComponentManager.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include<string.h>

MaterialSwap::MaterialSwap() :
	IComponent(),
	_renderer(nullptr),
	OnMaterial(nullptr),
	OffMaterial(nullptr)
{ 

}
MaterialSwap ::~MaterialSwap () = default;



void MaterialSwap::Swap(){
	//swaps based on BeatTime
	float beatTime = GetGameObject()->GetScene()->FindObjectByName("GameManager")->Get<BeatTimer>()->GetBeatTime();
	std::string name = GetGameObject()->Name;
	//Seriously this is Fucking Atrocious we need to do
	OnMaterial = GetGameObject()->GetScene()->FindObjectByName("Material Dummy On")->Get<RenderComponent>()->GetMaterial();
	OffMaterial = GetGameObject()->GetScene()->FindObjectByName("Material Dummy Off")->Get<RenderComponent>()->GetMaterial();
	_renderer = GetComponent<RenderComponent>();
	if ((name[0] == 'B') && (name[1] == 'e') && (name[2] == 'a') && (name[3] == 't') && (name[4] == 'G')) {
		int beatNumber = (int)name[8] - 48;
		std::cout <<"the gem is on beat number "<< beatNumber;
		if ((beatTime >= 0.6 * beatNumber - 0.6) && (beatTime <= 0.6 * beatNumber)) {
			std::cout << "the gem timer works ";
			_renderer->SetMaterial(OnMaterial);
		}
		else {
			_renderer->SetMaterial(OffMaterial);
			std::cout << "the gem timer doesnt works ";
		}
	}
	//if (beatTime >= 1.8 && beatTime <= 2.4) {
	//
	//}


}

void MaterialSwap ::Awake() {
	_renderer = GetComponent<RenderComponent>();
}

void MaterialSwap ::RenderImGui() { }

nlohmann::json MaterialSwap ::ToJson() const {
	return {
		{ "On_material", OnMaterial != nullptr ? OnMaterial->GetGUID().str() : "null" },
		{ "Off_material", OffMaterial != nullptr ? OffMaterial->GetGUID().str() : "null" }
	};
}

MaterialSwap ::Sptr MaterialSwap ::FromJson(const nlohmann::json& blob) {
	MaterialSwap ::Sptr result = std::make_shared<MaterialSwap >();
	result->OnMaterial = ResourceManager::Get<Gameplay::Material>(Guid(blob["On_material"]));
	result->OffMaterial  = ResourceManager::Get<Gameplay::Material>(Guid(blob["Off_material"]));
	return result;
}

void MaterialSwap::Update(float dt) {

	Swap();
}
