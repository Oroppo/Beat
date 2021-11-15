#include "Gameplay/Components/VinylAnim.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void VinylAnim::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();



    if (_body == nullptr) {
        IsEnabled = false;
    }
    keypoints.push_back(GetGameObject()->GetPosition().z);
    keypoints.push_back(GetGameObject()->GetPosition().z + 2);
    keypoints.push_back(GetGameObject()->GetPosition().z);

    _journeyLength = std::abs(keypoints[keyframe] - keypoints[keyframe + 1]);

}

void VinylAnim::RenderImGui() {

}

nlohmann::json VinylAnim::ToJson() const {
    return {
    };
}
// Constructor Initializes Values for LERP and Set Position but Only SetPosition is being used atm
VinylAnim::VinylAnim() :
    IComponent(),
    _segmentIndex(0),
    _TravelTime(5.0f),
    _startTime(0.f),
    _timeStored(0.f),
    keyframe(0.f),
    _speed(1.f),
    _journeyLength(0.f),

    _timer(1.0f),
    ObjY(0.0f),
    ObjZ(0.0f),
    ObjX(0.0f)

{ }

VinylAnim::~VinylAnim() = default;

VinylAnim::Sptr VinylAnim::FromJson(const nlohmann::json & blob) {
    VinylAnim::Sptr result = std::make_shared<VinylAnim>();
    return result;
}


void VinylAnim::Update(float deltaTime)
{
    if (_body->_isChild) {
        // _body->FollowParent();
    }

    // Object with behavior attached Y and Z position
    ObjY = GetGameObject()->GetPosition().y;
    ObjX = GetGameObject()->GetPosition().x;

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

    float SlowInOut = sqt / (2.0f * (sqt - fractionOfJourney) + 1.0f);

    GetGameObject()->SetPostion(glm::vec3(ObjX, ObjY, Lerp(keypoints[keyframe], keypoints[keyframe + 1], SlowInOut)));

    if ((fractionOfJourney >= 1.f) && (keyframe != keypoints.size() - 1))
    {
        _timeStored = _timer - _startTime;
        keyframe++;
    }

}

// Templated LERP function returns positon at current time for LERP
template <typename T>
T VinylAnim::Lerp(const T & p0, const T & p1, float t)
{
    return (1.0f - t) * p0 + t * p1;
}