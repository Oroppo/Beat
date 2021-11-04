#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

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

protected:

	float ScalarAdjustment;
	int score;

	bool _isPressed = false;
	Gameplay::Physics::RigidBody::Sptr _body;
};