
#pragma once
#include <iostream>
#include "IComponent.h"
#include "Gameplay/Physics//TriggerVolume.h"
 class JumpReset  {
	JumpReset();
	~JumpReset();

public: bool _canJump;
	    void OnTriggerEnter(const Gameplay::Physics::TriggerVolume::Sptr& trigger);
	    void OnTriggerExit(const Gameplay::Physics::TriggerVolume::Sptr& trigger);
	   bool get_canJump();
};