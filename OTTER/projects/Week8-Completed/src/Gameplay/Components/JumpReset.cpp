#pragma once
#include <iostream>
#include "IComponent.h"
#include"JumpReset.h"

	JumpReset::JumpReset() {
		_canJump = true;
	}
	JumpReset::~JumpReset() = default;

	void JumpReset::OnTriggerEnter(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
		_canJump = true;
	}
	void JumpReset::OnTriggerExit(const Gameplay::Physics::TriggerVolume::Sptr& trigger) {
		_canJump = false;
	}
	bool JumpReset::get_canJump() {
		return _canJump;
	}