#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "GLFW/glfw3.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/ResourceManager/IResource.h"
#include "Utils/TypeHelpers.h"
#include "FMOD/SoundEffects.h"

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
	//virtual void OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger);
public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(CharacterController);

	virtual nlohmann::json ToJson() const override;
	static CharacterController::Sptr FromJson(const nlohmann::json& blob);
	virtual void OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger);
	virtual void OnTriggerVolumeEntered(const std::shared_ptr<Gameplay::Physics::RigidBody>& body);
	virtual void OnTriggerVolumeLeaving(const std::shared_ptr<Gameplay::Physics::RigidBody>& body);

protected:
	double xPos = 0, yPos = 0;
	float _impulse2;
	float _GemJumpTimer;
	bool _canJump;
	int score = 0, _VinylScore = 0, _CDScore=0, _BeatGemHits=0;
	std::string _platform;
	glm::vec3 _impulse = glm::vec3(0, 0, 12.0);
	Gameplay::Physics::RigidBody::Sptr _body;
	Gameplay::Physics::RigidBody::Sptr _curvePlatform;
	bool _onCurvePlatform;
	glm::vec3 _rotPlat = glm::vec3(0.0f, 0.0f, 0.0f);
	int _LoadSceneC = 0;
	SoundEffects::Sptr SoundCaller;

};