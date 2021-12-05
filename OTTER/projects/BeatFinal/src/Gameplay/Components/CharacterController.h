#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "GLFW/glfw3.h"

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
	//void OnTriggerEnter(const Gameplay::Physics::TriggerVolume::Sptr& trigger);
	//void OnTriggerExit(const Gameplay::Physics::TriggerVolume::Sptr& trigger);
	virtual void OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body);
	virtual void OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body);


protected:
	double xPos = 0, yPos = 0;
	float _impulse2;
	bool _canJump;
	int score = 0;
	std::string _platform;
	glm::vec3 _impulse = glm::vec3(0, 0, 13.0);
	Gameplay::Physics::RigidBody::Sptr _body;
	Gameplay::Physics::RigidBody::Sptr _curvePlatform;
	bool _onCurvePlatform;
	glm::vec3 _rotPlat = glm::vec3(0.0f, 0.0f, 0.0f);

};