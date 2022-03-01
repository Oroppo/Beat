#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Gameplay/Components/RenderComponent.h"
#include "FMOD/SoundEffects.h"
#include "FMOD/ToneFire.h"

/// <summary>
/// Provides an example behaviour that uses some of the trigger interface to change the material
/// of the game object the component is attached to when entering or leaving a trigger
/// </summary>
class ScoreComponent : public Gameplay::IComponent {
	
public:
	typedef std::shared_ptr<ScoreComponent> Sptr;
	ScoreComponent();
	virtual ~ScoreComponent();

	// Inherited from IComponent
	virtual void Awake() override;
	virtual void RenderImGui() override;
	virtual nlohmann::json ToJson() const override;
	static ScoreComponent::Sptr FromJson(const nlohmann::json& blob);
	MAKE_TYPENAME(ScoreComponent);

	virtual void Update();

	int GetScore(){
		return _score;
	}

	void IncrementScore() {
		_score++;
		SFXS.PlayEvent("event:/Coin Pickup");
	}

	void SetScore(int foo) {
		_score = foo;
	}


protected:
	int _score = 0;
	RenderComponent::Sptr _renderer;

	SoundEffects SoundCaller;
	ToneFire::StudioSound SFXS = SoundCaller.SoundEffects::GetContextSound();
};