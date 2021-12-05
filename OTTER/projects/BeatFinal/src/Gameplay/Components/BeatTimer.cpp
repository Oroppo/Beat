#include "Gameplay/Components/BeatTimer.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void BeatTimer::Awake()
{

}

void BeatTimer::RenderImGui() {

}


BeatTimer::BeatTimer() :
	IComponent(){
	BeatTime = 0;
}





BeatTimer::~BeatTimer() = default;


float BeatTimer::GetBeatTime() {
	return BeatTime; 
}

void BeatTimer::Update(float deltaTime) {
	
	if (BeatTime > 1.66666) {
		BeatTime -=1.66666;
	 }
	else {
		BeatTime += deltaTime;
	}
}

