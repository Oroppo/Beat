#include "Gameplay/Components/SeekBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "SeekBehaviour.h"
#include "Gameplay/Physics/RigidBody.h"

void SeekBehaviour::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();
    if (_body == nullptr) {
        IsEnabled = false;
    }
}

void SeekBehaviour::RenderImGui() {

}

nlohmann::json SeekBehaviour::ToJson() const {
    return {
    };
}

// Constructor Initializes Values for LERP and Set Position but Only SetPosition is being used atm
SeekBehaviour::SeekBehaviour() :
    IComponent()

{ }

SeekBehaviour::~SeekBehaviour() = default;

SeekBehaviour::Sptr SeekBehaviour::FromJson(const nlohmann::json & blob) {
    SeekBehaviour::Sptr result = std::make_shared<SeekBehaviour>();
    return result;
}


void SeekBehaviour::Update(float deltaTime)
{
    glm::vec3 velocity = glm::vec3(1.0f, 1.0f, 1.0f);
    _body->SetLinearVelocity(velocity);
    LOG_INFO("update works");
}
void SeekBehaviour::seekTo(Gameplay::GameObject::Sptr& object) {
    target = object->GetPosition();
}
// Templated LERP function returns positon at current time for LERP
template <typename T>
T SeekBehaviour::Lerp(const T & p0, const T & p1, float t)
{
    return (1.0f - t) * p0 + t * p1;
}