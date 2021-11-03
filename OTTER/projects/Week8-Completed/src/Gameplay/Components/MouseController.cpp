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

    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(GetGameObject()->GetScene()->Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        glfwSetInputMode(GetGameObject()->GetScene()->Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }


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

    float x = xPos / windowWidth;
    float y = yPos / windowHeight;

    if (x > 2.3) {
        x = 2.3;
    }
    else if (x < 1.9) {
        x = 1.9;
    }

    if (y > 0.3) {
        y = 0.3;
    }
    else if (y < -0.3) {
        y = -0.3;
    }
    
    std::cout << "Cursor: " << x << ", " << y << std::endl;


    GetGameObject()->SetPostion(glm::vec3(x, y, 0.0f));



}

