#include <iostream>

#include <chrono>
#include <thread>

#include <fmod.hpp>
#include <fmod_errors.h>
class CAudioEngine {
public:

    bool succeededOrWarn(const std::string& message, FMOD_RESULT result)
    {
        if (result != FMOD_OK) {
            std::cerr << message << ": " << result << " " << FMOD_ErrorString(result) << std::endl;
            return false;
        }
        return true;
    }

   //FMOD_RESULT F_CALLBACK channelGroupCallback(FMOD_CHANNELCONTROL* channelControl,
   //    FMOD_CHANNELCONTROL_TYPE controlType, FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
   //    void* commandData1, void* commandData2)
   //{
   //    std::cout << "Callback called for " << controlType << std::endl;
   //    return controlType;
   //}

    int DoFMOD()
    {
        FMOD_RESULT result;

        FMOD::System* system = nullptr;
        // Create the main system object.
        result = FMOD::System_Create(&system);
        if (!succeededOrWarn("FMOD: Failed to create system object", result))
            return 1;

        // Initialize FMOD.
        result = system->init(512, FMOD_INIT_NORMAL, nullptr);
        if (!succeededOrWarn("FMOD: Failed to initialise system object", result))
            return 1;

        // Create the channel group.
        FMOD::ChannelGroup* channelGroup = nullptr;
        result = system->createChannelGroup("inGameSoundEffects", &channelGroup);
        if (!succeededOrWarn("FMOD: Failed to create in-game sound effects channel group", result))
            return 1;

        // Create the sound.
        FMOD::Sound* sound = nullptr;
        system->createSound("/Users/foo/audio/bar.wav", FMOD_DEFAULT, nullptr, &sound);

        // Play the sound.
        FMOD::Channel* channel = nullptr;
        result = system->playSound(sound, nullptr, false, &channel);
        if (!succeededOrWarn("FMOD: Failed to play sound", result))
            return 1;

        // Assign the channel to the group.
        result = channel->setChannelGroup(channelGroup);
        if (!succeededOrWarn("FMOD: Failed to set channel group on", result))
            return 1;

       // // Set a callback on the channel.
       // channel->setCallback(&channelGroupCallback);
       // if (!succeededOrWarn("FMOD: Failed to set callback for sound", result))
       //     return 1;

        bool isPlaying = false;
        do {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            channel->isPlaying(&isPlaying);

            system->update();
        } while (isPlaying);


        // Clean up.
        sound->release();
        channelGroup->release();
        system->release();
    }
};