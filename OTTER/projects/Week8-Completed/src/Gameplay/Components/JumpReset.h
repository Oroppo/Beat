#pragma once
#include <iostream>
#include "IComponent.h"
#include "Gameplay/Physics//TriggerVolume.h"
class JumpReset : public Gameplay::IComponent {
	JumpReset();
	~JumpReset();

private: bool _canJump;
virtual void OnTriggerEnter(const Gameplay::Physics::TriggerVolume::Sptr& trigger);
virtual void OnTriggerExit(const Gameplay::Physics::TriggerVolume::Sptr& trigger);
bool get_canJump();
};