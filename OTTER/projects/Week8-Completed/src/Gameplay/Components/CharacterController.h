#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "GLFW/glfw3.h"
#include "Gameplay/Components/JumpReset.h"
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
	void OnTriggerEnter(const Gameplay::Physics::TriggerVolume::Sptr& trigger);
	void OnTriggerExit(const Gameplay::Physics::TriggerVolume::Sptr& trigger);

	

protected:
	double xPos=0, yPos=0;
	float _impulse2;
	bool _canJump;

	glm::vec3 _impulse = glm::vec3(0,0,1);
	Gameplay::Physics::RigidBody::Sptr _body;


};