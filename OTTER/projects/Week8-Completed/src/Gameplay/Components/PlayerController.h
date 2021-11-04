#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
class PlayerController : public Gameplay::IComponent {

	
public: 
	typedef std::shared_ptr<PlayerController> Sptr;

	void Update();
protected:
	float _impulse;

	bool _isPressed = false;
	Gameplay::Physics::RigidBody::Sptr _body;
};