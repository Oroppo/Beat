#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "GLFW/glfw3.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class MouseController : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<MouseController> Sptr;

	MouseController();
	virtual ~MouseController();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(MouseController);
	virtual nlohmann::json ToJson() const override;
	static MouseController::Sptr FromJson(const nlohmann::json& blob);



protected:
	double xPos = 0, yPos = 0;
	int windowHeight = 0, windowWidth = 0;
	float _impulse;

	Gameplay::Physics::RigidBody::Sptr _body;
};