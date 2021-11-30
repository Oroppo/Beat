#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "GLFW/glfw3.h"

/// <summary>
/// A simple behaviour that applies an impulse along the Z axis to the 
/// rigidbody of the parent when the space key is pressed
/// </summary>
class BackgroundMover : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<BackgroundMover> Sptr;

	BackgroundMover();
	virtual ~BackgroundMover();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;


public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(BackgroundMover);
	virtual nlohmann::json ToJson() const override;
	static BackgroundMover::Sptr FromJson(const nlohmann::json& blob);



protected:
	float ObjY;
	float ObjZ;
	float ObjX;

	Gameplay::Physics::RigidBody::Sptr _body;

};

class BackgroundBuildingMover : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<BackgroundBuildingMover> Sptr;

	BackgroundBuildingMover();
	virtual ~BackgroundBuildingMover();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;


public:
	virtual void RenderImGui() override;
	MAKE_TYPENAME(BackgroundBuildingMover);
	virtual nlohmann::json ToJson() const override;
	static BackgroundBuildingMover::Sptr FromJson(const nlohmann::json& blob);



protected:
	float BuildObjY;
	float BuildObjZ;
	float BuildObjX;

	Gameplay::Physics::RigidBody::Sptr _body;

};