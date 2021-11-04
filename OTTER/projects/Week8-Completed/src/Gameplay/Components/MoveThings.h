#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Components/ScoreComponent.h"
#include "Gameplay/GameObject.h"

/// <summary>
/// A simple behaviour that applies velocity per frame
/// </summary>
class MoveThings : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<MoveThings> Sptr;

	MoveThings();
	virtual ~MoveThings();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(MoveThings);
	virtual nlohmann::json ToJson() const override;
	static MoveThings::Sptr FromJson(const nlohmann::json& blob);

	float GetCoefficient() {
		return ScalarAdjustment;
	}
	void SetCoefficient(float foo) {
		ScalarAdjustment = foo;
	}

	void GrabGuid1(std::string foo) {
		_scoreP1 = Gameplay::ComponentManager::GetComponentByGUID<ScoreComponent>(Guid(foo));
	}

	void GrabGuid2(std::string foo) {
		_scoreP2 = Gameplay::ComponentManager::GetComponentByGUID<ScoreComponent>(Guid(foo));
	}

protected:

	float ScalarAdjustment;
	ScoreComponent::Sptr _scoreP1, _scoreP2;

	bool _isPressed = false;
	Gameplay::Physics::RigidBody::Sptr _body;

};