#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "GLFW/glfw3.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class ForeGroundMover : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<ForeGroundMover> Sptr;

	ForeGroundMover();
	virtual ~ForeGroundMover();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;


public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(ForeGroundMover);
	virtual nlohmann::json ToJson() const override;
	static ForeGroundMover::Sptr FromJson(const nlohmann::json& blob);



protected:
	float ObjY;
	float ObjZ;
	float ObjX;

	Gameplay::Physics::RigidBody::Sptr _body;

};
