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

  //if (glfwRawMouseMotionSupported()) {
  //    glfwSetInputMode(GetGameObject()->GetScene()->Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  //    glfwSetInputMode(GetGameObject()->GetScene()->Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

    float x = xPos;
    float y = yPos;
    
    SetThisFrame(glm::vec2(x, y));


    //Normalizing Vector
    glm::vec2 temp(thisFrame * thisFrame + lastFrame * lastFrame);
    glm::vec2 normalized(temp/temp);

    float thisMagnitude = (thisFrame.x + thisFrame.y) / (thisFrame.x + thisFrame.y);
    float lastMagnitude = (lastFrame.x + lastFrame.y) / (lastFrame.x + lastFrame.y);


    normalized *=1;

    GetThisFrame();
    GetLastFrame();

    

    std::cout << "Cursor: " << x << ", " << y << std::endl;


    GetGameObject()->SetPostion(GetGameObject()->GetPosition() + glm::vec3(normalized, 0.0f));


   SetLastFrame(thisFrame);
}

