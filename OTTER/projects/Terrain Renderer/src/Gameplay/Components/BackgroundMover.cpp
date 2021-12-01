#include "Gameplay/Components/BackgroundMover.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void BackgroundMover::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();
    if (_body == nullptr) {
        IsEnabled = false;
    }

}

void BackgroundMover::RenderImGui() {

}

nlohmann::json BackgroundMover::ToJson() const {
    return {
    };
}
// Constructor Initializes Values for LERP and Set Position but Only SetPosition is being used atm
BackgroundMover::BackgroundMover() :
    IComponent(),
    ObjY(0.0f),
    ObjZ(0.0f),
    ObjX(0.0f)
{ }

BackgroundMover::~BackgroundMover() = default;

BackgroundMover::Sptr BackgroundMover::FromJson(const nlohmann::json & blob) {
    BackgroundMover::Sptr result = std::make_shared<BackgroundMover>();
    return result;
}

void BackgroundMover::Update(float deltaTime)
{
    // object with behavior attached X position
    float BObjPosX = GetGameObject()->GetPosition().x;

    // Object with behavior attached Y and Z position
    ObjY = GetGameObject()->GetPosition().y;
    ObjZ = GetGameObject()->GetPosition().z;

    BObjPosX = BObjPosX - 15.0 * deltaTime;
    GetGameObject()->SetPostion(glm::vec3(BObjPosX, ObjY, ObjZ));

    if (GetGameObject()->GetPosition().x <= -20.0f)
    {
        GetGameObject()->SetPostion(glm::vec3(glm::vec3(14.870f, 7.80f, 2.7f)));
    }
}


// Only for Moving Buildings

void BackgroundBuildingMover::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();
    if (_body == nullptr) {
        IsEnabled = false;
    }

}

void BackgroundBuildingMover::RenderImGui() {

}

nlohmann::json BackgroundBuildingMover::ToJson() const {
    return {
    };
}
// Constructor Initializes Values for LERP and Set Position but Only SetPosition is being used atm
BackgroundBuildingMover::BackgroundBuildingMover() :
    IComponent(),
    BuildObjY(0.0f),
    BuildObjZ(0.0f),
    BuildObjX(0.0f)
{ }

BackgroundBuildingMover::~BackgroundBuildingMover() = default;

BackgroundBuildingMover::Sptr BackgroundBuildingMover::FromJson(const nlohmann::json & blob) {
    BackgroundBuildingMover::Sptr result = std::make_shared<BackgroundBuildingMover>();
    return result;
}

void BackgroundBuildingMover::Update(float deltaTime)
{
    // object with behavior attached X position
    float BuildObjPosX = GetGameObject()->GetPosition().x;

    // Object with behavior attached Y and Z position
    BuildObjY = GetGameObject()->GetPosition().y;
    BuildObjZ = GetGameObject()->GetPosition().z;

    BuildObjPosX = BuildObjPosX - 5.0 * deltaTime;
    GetGameObject()->SetPostion(glm::vec3(BuildObjPosX, BuildObjY, BuildObjZ));

    if (GetGameObject()->GetPosition().x <= -50.0f)
    {
        GetGameObject()->SetPostion(glm::vec3(glm::vec3(40.870f, 21.880f, -36.040f)));
    }
}