#include "PlayerController.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include <iostream>

void PlayerController::Update() {
	bool pressed = glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_SPACE);
	if (pressed) {	
		_isPressed = pressed;
		std::cout << "presss success"<<std::endl;
	}
	
}