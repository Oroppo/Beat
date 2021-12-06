#include "Gameplay/Components/LevelMover.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"


void LevelMover::OnLoad() {

    keypoints.push_back(GetGameObject()->GetPosition().x);
    keypoints.push_back(GetGameObject()->GetPosition().x - 40);
    
    _journeyLength = std::abs(keypoints[keyframe] - keypoints[keyframe + 1]);
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
    _segmentIndex(0),
    _TravelTime(5.0f),
    _startTime(0.f),
    _timeStored(0.f),
    keyframe(0.f),
    _speed(1.0f),
    _journeyLength(0.f),

    _timer(1.2f),
    ObjY(0.0f),
    ObjZ(0.0f),
    ObjX(0.0f),
    _switchIndex(true)

{ }

LevelMover::~LevelMover() = default;

LevelMover::Sptr LevelMover::FromJson(const nlohmann::json & blob) {
    LevelMover::Sptr result = std::make_shared<LevelMover>();
    return result;
}


void LevelMover::Update(float deltaTime)
{

    // Object with behavior attached Y and Z position
    ObjY = GetGameObject()->GetPosition().y;
    ObjZ = GetGameObject()->GetPosition().z;

    _timer += deltaTime;
    // Distance moved equals elapsed time times speed..
    float distCovered = (_timer - _startTime - _timeStored) * _speed;

    // Fraction of journey completed equals current distance divided by total distance.
    float fractionOfJourney = distCovered / _journeyLength;

    if (keyframe == keypoints.size() - 1)
    {
        keyframe = 0;
    }

    float sqt = (fractionOfJourney) * (fractionOfJourney);

    // float SlowInOut = sqt / (2.0f * (sqt - fractionOfJourney) + 1.0f);

    GetGameObject()->SetPostion(glm::vec3(Lerp(keypoints[keyframe], keypoints[keyframe + 1], fractionOfJourney), ObjY, ObjZ));

    if ((fractionOfJourney >= 1.f) && (keyframe != keypoints.size() - 1))
    {
        _timeStored = _timer - _startTime;
        keyframe++;
    }

    Gameplay::GameObject::Sptr context = GetGameObject()->SelfRef();
    if (GetGameObject()->GetPosition().x <= -25.f) {
        GetGameObject()->GetScene()->RemoveGameObject(context);
    }
}

// Templated LERP function returns positon at current time for LERP
template <typename T>
T LevelMover::Lerp(const T& p0, const T& p1, float t)
{
    return (1.0f - t) * p0 + t * p1;
}