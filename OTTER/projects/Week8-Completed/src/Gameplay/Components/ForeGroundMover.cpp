#include "Gameplay/Components/ForeGroundMover.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void ForeGroundMover::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();
    if (_body == nullptr) {
        IsEnabled = false;
    }

}

void ForeGroundMover::RenderImGui() {

}

nlohmann::json ForeGroundMover::ToJson() const {
    return {
    };
}
// Constructor Initializes Values for LERP and Set Position but Only SetPosition is being used atm
ForeGroundMover::ForeGroundMover() :
    IComponent(),
    ObjY(0.0f),
    ObjZ(0.0f),
    ObjX(0.0f)
{ }

ForeGroundMover::~ForeGroundMover() = default;

ForeGroundMover::Sptr ForeGroundMover::FromJson(const nlohmann::json & blob) {
    ForeGroundMover::Sptr result = std::make_shared<ForeGroundMover>();
    return result;
}

void ForeGroundMover::Update(float deltaTime)
{
    // object with behavior attached X position
    float BObjPosX = GetGameObject()->GetPosition().x;

    // Object with behavior attached Y and Z position
    ObjY = GetGameObject()->GetPosition().y;
    ObjZ = GetGameObject()->GetPosition().z;
    // to move from Left to Right
   /* if (GetGameObject()->Name != "Car1")
    {
        FObjPosX = FObjPosX - 4.5 * deltaTime;
        GetGameObject()->SetPostion(glm::vec3(FObjPosX, ObjY, ObjZ));
    }*/
    /*  if (GetGameObject()->GetPosition().y >= 10.0f);
      {
         BObjPosX = BObjPosX - 15.0 * deltaTime;
         GetGameObject()->SetPostion(glm::vec3(BObjPosX, ObjY, ObjZ));
      }*/

    BObjPosX = BObjPosX + 15.0 * deltaTime;
    GetGameObject()->SetPostion(glm::vec3(BObjPosX, ObjY, ObjZ));

    if (GetGameObject()->GetPosition().x >= 40.0f)
    {
        GetGameObject()->SetPostion(glm::vec3(glm::vec3(-9.970f, 0.470f, -1.90f)));
    }
}