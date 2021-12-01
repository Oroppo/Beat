#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "GLFW/glfw3.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class LevelMover : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<LevelMover> Sptr;

	LevelMover();
	virtual ~LevelMover();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;
	template<typename T>
	T Lerp(const T& p0, const T& p1, float t);
	

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(LevelMover);
	virtual nlohmann::json ToJson() const override;
	static LevelMover::Sptr FromJson(const nlohmann::json& blob);



protected:
	
	size_t _segmentIndex;
	float _timer;
	float _TravelTime;
	float _startTime;
	float _timeStored;
	float _speed;
	std::vector<float> keypoints;
	float _journeyLength;


	int keyframe;

	bool _switchIndex;

	float ObjY;
	float ObjZ;
	float ObjX;

	Gameplay::Physics::RigidBody::Sptr _body;
	Gameplay::Physics::RigidBody::Sptr parent;

};