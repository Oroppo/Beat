#pragma once
#include "IComponent.h"
/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class BeatTimer : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<BeatTimer> Sptr;

	BeatTimer();
	virtual ~BeatTimer();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(BeatTimer);

	virtual nlohmann::json ToJson() const override;
	static BeatTimer::Sptr FromJson(const nlohmann::json& blob);


	float GetBeatTime();
protected:

	float _BeatTime;
};