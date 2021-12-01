#pragma once
#include "IComponent.h"

/// <summary>
/// Showcases a very simple behaviour that rotates the parent gameobject at a fixed rate over time
/// </summary>
class RotatingBehaviour : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<RotatingBehaviour> Sptr;

	RotatingBehaviour() = default;
	glm::vec3 RotationSpeed = glm::vec3(0,0,30);

	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static RotatingBehaviour::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(RotatingBehaviour);
};


// ROTATION BEHAVIOURS FOR THE CDs
class RotatingBehaviourCD : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<RotatingBehaviourCD> Sptr;

	RotatingBehaviourCD() = default;
	glm::vec3 RotationCDSpeed = glm::vec3(0, 0, 60);

	virtual void Update(float deltaTime) override;

	virtual void RenderImGui() override;

	virtual nlohmann::json ToJson() const override;
	static RotatingBehaviourCD::Sptr FromJson(const nlohmann::json& data);

	MAKE_TYPENAME(RotatingBehaviourCD);
};




