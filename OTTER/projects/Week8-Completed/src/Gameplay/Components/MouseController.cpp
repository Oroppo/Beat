#include "Gameplay/Components/MouseController.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

#include <cstdint>
#include <algorithm>
#include <iostream>
#include <iomanip>

void MouseController::Awake()
{
    _body = GetComponent<Gameplay::Physics::RigidBody>();
    if (_body == nullptr) {
        IsEnabled = false;
    }

   // if (glfwRawMouseMotionSupported()) {
     //   glfwSetInputMode(GetGameObject()->GetScene()->Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
       // glfwSetInputMode(GetGameObject()->GetScene()->Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //}

    SetLastFrame(glm::vec2(0, 0));
}

 
void MouseController::RenderImGui() {

}

nlohmann::json MouseController::ToJson() const {
    return {
        { "impulse", _impulse }
    };
}

MouseController::MouseController() :
    IComponent()
{ }

MouseController::~MouseController() = default;

MouseController::Sptr MouseController::FromJson(const nlohmann::json & blob) {
    MouseController::Sptr result = std::make_shared<MouseController>();
    return result;
}

void MouseController::Update(float deltaTime) {

    glfwGetCursorPos(GetGameObject()->GetScene()->Window, &xPos, &yPos);
    glfwGetWindowSize(GetGameObject()->GetScene()->Window, &windowWidth, &windowHeight);

    //glfw wants doubles and glm::vec2 wants floats, 
    //casting is just messy to read so we're doing a quick type swap here:
    float x = xPos;
    if (x == 0.f)x = 0.001f;
    float y = yPos;
    if (y == 0.f)y = 0.001f;
    
    
    SetThisFrame(glm::vec2(x, y));


    //Normalizing Vector
    glm::vec2 temp(thisFrame * thisFrame + lastFrame * lastFrame);
    glm::vec2 normalized(temp/temp);

    //Calculate X and Y Distance
    float DistanceX = (thisFrame.x - lastFrame.x);
    if (DistanceX == 0.f) DistanceX = 0.001f;
    float DistanceY = (thisFrame.y - lastFrame.y);
    if (DistanceY == 0.f) DistanceY = 0.001f;

    //Divide By window height/width to make the movement proportional to % of screen moved
    DistanceX /= windowWidth;
    DistanceY /= windowHeight;

    //std::cout << "Cursor: " << x << ", " << y << std::endl;

    //sets position equal to current position times the normalized Direction
    //over the distance between 2 frames

    GetGameObject()->SetPostion(GetGameObject()->GetPosition() + 
        glm::vec3(normalized.x* DistanceX, normalized.y* -DistanceY, 0.0f));

    glm::vec3 CurrentPosition = GetGameObject()->GetPosition();

    //Boundary Cheking since Kinematic Objects Don't Interact with colliders traditionally

    if (GetGameObject()->GetPosition().x < 1.565) {
        GetGameObject()->SetPostion(glm::vec3(1.565f, CurrentPosition.y, CurrentPosition.z));
    }
    if (GetGameObject()->GetPosition().x > 2.25) {
        GetGameObject()->SetPostion(glm::vec3(2.25f, CurrentPosition.y, CurrentPosition.z));
    }
    if (GetGameObject()->GetPosition().y > 0.4) {
        GetGameObject()->SetPostion(glm::vec3(CurrentPosition.x, 0.4f, CurrentPosition.z));
    }
    if (GetGameObject()->GetPosition().y < -0.4) {
        GetGameObject()->SetPostion(glm::vec3(CurrentPosition.x, -0.4f, CurrentPosition.z));
    }

    //this frame last frame
    SetLastFrame(thisFrame);
    GetGameObject()->GetScene()->Lights[2].Position = GetGameObject()->GetPosition();
}

