#include "Gameplay/Components/MoveThings.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/GlmBulletConversions.h"

void MoveThings::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();

	if (_body == nullptr) {
		IsEnabled = false;
	}
}

void MoveThings::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Coefficient", &cumDuggery, 0.0f, 1.0f);
}

nlohmann::json MoveThings::ToJson() const {
	return {
		{ "Coefficient", cumDuggery }
	};
}

MoveThings::MoveThings() :
	IComponent(),
	cumDuggery(1.0f)
{ }

MoveThings::~MoveThings() = default;

MoveThings::Sptr MoveThings::FromJson(const nlohmann::json & blob) {
	MoveThings::Sptr result = std::make_shared<MoveThings>();
	result->cumDuggery = blob["Coefficient"];
	return result;
}

void MoveThings::Update(float deltaTime) {

	//This takes the Inertia of the Gameobject and updates postion Per Frame based on previous frame's Velocity
	btRigidBody* bullshit = _body->GetBody();
	bullshit->getLinearVelocity();
	glm::vec3 fuckery = ToGlm(bullshit->getLinearVelocity());
	
	GetGameObject()->SetPostion(
		(GetGameObject()->GetPosition() + (fuckery * cumDuggery)));

	bullshit->setAngularVelocity(ToBt(glm::vec3(0, 0, 0)));
	bullshit->setFriction(0.01);
	bullshit->setDamping(0,0);
	bullshit->setRestitution(0);

	glm::vec3 CurrentPosition = GetGameObject()->GetPosition();

	if (GetGameObject()->GetPosition().x < 0.6) {
		GetGameObject()->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
		bullshit->setLinearVelocity(ToBt(glm::vec3(0.f,0.f,0.f)));
	}
	if (GetGameObject()->GetPosition().x > 2.40) {
		GetGameObject()->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
		bullshit->setLinearVelocity(ToBt(glm::vec3(0.f, 0.f, 0.f)));
	}
	if (GetGameObject()->GetPosition().y > 0.5) {
		GetGameObject()->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
		bullshit->setLinearVelocity(ToBt(glm::vec3(0.f, 0.f, 0.f)));
	}
	if (GetGameObject()->GetPosition().y < -0.5) {
		GetGameObject()->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
		bullshit->setLinearVelocity(ToBt(glm::vec3(0.f, 0.f, 0.f)));
	}
	
}

