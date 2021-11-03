#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class CharacterController : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<CharacterController> Sptr;

	CharacterController();
	virtual ~CharacterController();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(CharacterController);
	virtual nlohmann::json ToJson() const override;
	static CharacterController::Sptr FromJson(const nlohmann::json& blob);

protected:

	float _impulse;

	Gameplay::Physics::RigidBody::Sptr _body;
};