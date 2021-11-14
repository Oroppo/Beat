#include "Gameplay/Components/LevelMover.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void LevelMover::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();
    if (_body == nullptr) {
        IsEnabled = false;
    }

}

void LevelMover::RenderImGui() {

}

nlohmann::json LevelMover::ToJson() const {
    return {
    };
}
// Constructor Initializes Values for LERP and Set Position but Only SetPosition is being used atm
LevelMover::LevelMover() :
    IComponent(),
    keypoints(2),
    _segmentIndex(0),
    _TravelTime(500.0f),
    _timer(1.0f),
    ObjY(0.0f),
    ObjZ(0.0f),
    ObjX(0.0f)
{ }

LevelMover::~LevelMover() = default;

LevelMover::Sptr LevelMover::FromJson(const nlohmann::json & blob) {
    LevelMover::Sptr result = std::make_shared<LevelMover>();
    return result;
}

void LevelMover::Update(float deltaTime)
{
    // object with behavior attached X position
    float FObjPosX = GetGameObject()->GetPosition().x;
    float BObjPosX = GetGameObject()->GetPosition().x;
    
    // Object with behavior attached Y and Z position
    ObjY = GetGameObject()->GetPosition().y;
    ObjZ = GetGameObject()->GetPosition().z;

    // Lerp Currently Works However Objects further away move faster than those closer
    /*_timer += deltaTime;

    while (_timer > _TravelTime)
    {
        _timer -= _TravelTime;

      //  _segmentIndex += 1;

      //  if (_segmentIndex >= keypoints)
        //    _segmentIndex = 0;
    }
    float t = _timer / _TravelTime;

    float p0, p1;
    p0 = GetGameObject()->GetPosition().x;
    p1 = -20.1f;
    ObjX = Lerp(p0, p1, t);

    GetGameObject()->SetPostion(glm::vec3(ObjX, ObjY, ObjZ));*/

    // Moves the Objects based on delta time and a fixed value Currently is about 6 seconds for objects
    // to move from Left to Right
   /* if (GetGameObject()->Name != "Car1")
    {
        FObjPosX = FObjPosX - 4.5 * deltaTime;
        GetGameObject()->SetPostion(glm::vec3(FObjPosX, ObjY, ObjZ));
    }
    if (GetGameObject()->GetPosition().y >= 10.0f);
    {
       BObjPosX = BObjPosX - 15.0 * deltaTime;
       GetGameObject()->SetPostion(glm::vec3(BObjPosX, ObjY, ObjZ));
    }*/
    FObjPosX = FObjPosX - 4.5 * deltaTime;
    GetGameObject()->SetPostion(glm::vec3(FObjPosX, ObjY, ObjZ));
}

// Templated LERP function returns positon at current time for LERP
template<typename T>
T LevelMover::Lerp(const T& p0, const T& p1, float t)
{
    return (1.0f - t) * p0 + t * p1;
}