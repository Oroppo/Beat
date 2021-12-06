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
	_BeatTime = 0;
}


BeatTimer::Sptr BeatTimer::FromJson(const nlohmann::json& blob) {
	
	BeatTimer::Sptr result = std::make_shared<BeatTimer>();
	result->_BeatTime = blob["BeatTime"];
	return result;
}
nlohmann::json BeatTimer::ToJson() const {
	return {
		{ "BeatTime", _BeatTime }
	};
}


BeatTimer::~BeatTimer() = default;


float BeatTimer::GetBeatTime() {
	return _BeatTime; 
}

void BeatTimer::Update(float deltaTime) {
	_BeatTime += deltaTime;
	//since music is in 100 bpm, there are 60 seconds in a munute so 100/60 is 1.666666 so bar resets once every 1.6666 seconds
	//reseting timer so that it may be  passed to character controller and used for the beat gem logic
	if (_BeatTime >= 2.4) {
		_BeatTime -=2.4;
	 }
}

