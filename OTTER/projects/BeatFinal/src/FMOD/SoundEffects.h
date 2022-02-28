#pragma once
#include "Sound/AudioEngine.h"
#include "Fmod.h"
#include "FMOD/ToneFire.h"
#include <memory>

class SoundEffects
{
public:
	typedef std::shared_ptr<SoundEffects> Sptr;

	void init()
	{
		Studio.LoadBank("Master.bank");
		Studio.LoadBank("Master.strings.bank");
		Studio.LoadBank("Level1.bank");
	}
	ToneFire::FMODStudio GetContext()
	{
		return Studio;
	}
	ToneFire::StudioSound GetContextSound()
	{
		return Test;
	}

private:
	ToneFire::FMODStudio Studio;
	ToneFire::StudioSound Test;
};

