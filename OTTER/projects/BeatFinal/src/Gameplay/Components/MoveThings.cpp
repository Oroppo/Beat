#include "Gameplay/Components/MoveThings.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/GlmBulletConversions.h"
#include "ComponentManager.h"
#include "ScoreComponent.h"
#include <string>


void MoveThings::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();

	if (_body == nullptr) {
		IsEnabled = false;
	}

	
}

void MoveThings::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Coefficient", &ScalarAdjustment, 0.0f, 1.0f);
}

nlohmann::json MoveThings::ToJson() const {
	return {
		{ "Coefficient", ScalarAdjustment }
	};
}

MoveThings::MoveThings() :
	IComponent(),
	ScalarAdjustment(1.0f)
{ }

MoveThings::~MoveThings() = default;

MoveThings::Sptr MoveThings::FromJson(const nlohmann::json & blob) {
	MoveThings::Sptr result = std::make_shared<MoveThings>();
	result->ScalarAdjustment = blob["Coefficient"];
	return result;
}

void MoveThings::Update(float deltaTime) {

	//This takes the Inertia of the Gameobject and updates postion Per Frame based on previous frame's Velocity
	btRigidBody* currentVel = _body->GetBody();
	currentVel->getLinearVelocity();
	glm::vec3 Inertia = ToGlm(currentVel->getLinearVelocity());
	
	GetGameObject()->SetPostion(
		(GetGameObject()->GetPosition() + (Inertia * ScalarAdjustment)));

	currentVel->setAngularVelocity(ToBt(glm::vec3(0, 0, 0)));
	currentVel->setFriction(0.01);
	currentVel->setDamping(0,0);
	currentVel->setRestitution(0);

	glm::vec3 CurrentPosition = GetGameObject()->GetPosition();

	if (GetGameObject()->GetPosition().x < 0.7) {
		GetGameObject()->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
		currentVel->setLinearVelocity(ToBt(glm::vec3(0.f,0.f,0.f)));

		_scoreP2->IncrementScore();
		std::cout << "Player 2 Score: "<< _scoreP2->GetScore() << std::endl;

	}
	if (GetGameObject()->GetPosition().x > 2.30) {
		GetGameObject()->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
		currentVel->setLinearVelocity(ToBt(glm::vec3(0.f, 0.f, 0.f)));

		_scoreP1->IncrementScore();
		std::cout << "Player 1 Score: " << _scoreP1->GetScore() << std::endl;

	}
	if (GetGameObject()->GetPosition().y > 0.5) {
		GetGameObject()->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
		currentVel->setLinearVelocity(ToBt(glm::vec3(0.f, 0.f, 0.f)));
	}
	if (GetGameObject()->GetPosition().y < -0.5) {
		GetGameObject()->SetPostion(glm::vec3(1.5f, 0.01f, 1.255f));
		currentVel->setLinearVelocity(ToBt(glm::vec3(0.f, 0.f, 0.f)));
	}
	GetGameObject()->GetScene()->Lights[0].Position = GetGameObject()->GetPosition();
}

