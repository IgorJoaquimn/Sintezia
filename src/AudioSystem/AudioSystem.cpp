#include "AudioSystem.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <filesystem>
#include <algorithm>

SoundHandle SoundHandle::Invalid;

// Create the AudioSystem with specified number of channels
// (Defaults to 8 channels)
AudioSystem::AudioSystem(int numChannels)
{
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_AllocateChannels(numChannels);
    mChannels.resize(numChannels);
}

// Destroy the AudioSystem
AudioSystem::~AudioSystem()
{
    for (auto& sound : mSounds)
    {
        Mix_FreeChunk(sound.second);
    }
    mSounds.clear();

    Mix_CloseAudio();
}

// Updates the status of all the active sounds every frame
void AudioSystem::Update(float deltaTime)
{
    for(int i = 0; i < mChannels.size(); i++)
    {
        if(mChannels[i].IsValid())
        {
            if(!Mix_Playing(i)) {
                mHandleMap.erase(mChannels[i]);
                mChannels[i].Reset();
            }
        }
    }
}

// Plays the sound with the specified name and loops if looping is true
// Returns the SoundHandle which is used to perform any other actions on the
// sound when active
// NOTE: The soundName is without the "assets/sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "assets/sounds/ChompLoop.wav".
SoundHandle AudioSystem::PlaySound(const std::string& soundName, bool looping, int volume)
{
    // Get the sound with the given name
    Mix_Chunk *sound = GetSound(soundName);

    if (sound == nullptr) {
        SDL_Log("[AudioSystem] PlaySound couldn't find sound for %s", soundName.c_str());
        return SoundHandle::Invalid;
    }

    int availableChannel = -1;
    for (int i = 0; i < mChannels.size(); i++)
    {
        if (!mChannels[i].IsValid())
        {
            availableChannel = i;
            break;
        }
    }


    if (availableChannel == -1)
    {
        availableChannel = 0;

        if (mChannels[availableChannel].IsValid())
        {
            Mix_HaltChannel(availableChannel);
            mHandleMap.erase(mChannels[availableChannel]);
            mChannels[availableChannel].Reset();
        }
    }

    mLastHandle++;
    SoundHandle newHandle = mLastHandle;

    int channelPlayed = Mix_PlayChannel(availableChannel, sound, looping ? -1 : 0);

    if (channelPlayed == -1)
    {
        SDL_Log("[AudioSystem] Mix_PlayChannel failed for channel %d: %s", availableChannel, Mix_GetError());
        return SoundHandle::Invalid;
    }

    if (volume > 128) volume = 128;
    if (volume < 0) volume = 0;

    // Define o volume APENAS para este canal tocando agora
    Mix_Volume(channelPlayed, volume);

    mChannels[availableChannel] = newHandle;
    HandleInfo info;
    info.mSoundName = soundName;
    info.mChannel = availableChannel;
    info.mIsLooping = looping;
    info.mIsPaused = false;
    info.mVolume = volume;
    mHandleMap[newHandle] = info;

    // Set the volume (SDL_mixer uses 0-128 scale, we convert from 0-100)
    int mixVolume = (volume * 128) / 100;
    Mix_VolumeChunk(sound, mixVolume);

    return newHandle;
}

// Stops the sound if it is currently playing
void AudioSystem::StopSound(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        SDL_Log("[AudioSystem] StopSound couldn't find handle %s", sound.GetDebugStr());
        return;
    }

    Mix_HaltChannel(mHandleMap[sound].mChannel);
    mHandleMap.erase(sound);
    mChannels[mHandleMap[sound].mChannel].Reset();
}

// Pauses the sound if it is currently playing
void AudioSystem::PauseSound(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        SDL_Log("[AudioSystem] PauseSound couldn't find handle %s", sound.GetDebugStr());
        return;
    }

    if(!mHandleMap[sound].mIsPaused)
    {
        Mix_Pause(mHandleMap[sound].mChannel);
        mHandleMap[sound].mIsPaused = true;
    }
}

// Resumes the sound if it is currently paused
void AudioSystem::ResumeSound(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        SDL_Log("[AudioSystem] ResumeSound couldn't find handle %s", sound.GetDebugStr());
        return;
    }

    if(mHandleMap[sound].mIsPaused)
    {
        Mix_Resume(mHandleMap[sound].mChannel);
        mHandleMap[sound].mIsPaused = false;
    }
}

// Returns the current state of the sound
SoundState AudioSystem::GetSoundState(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        return SoundState::Stopped;
    }

    if(mHandleMap[sound].mIsPaused)
    {
        return SoundState::Paused;
    }

    return SoundState::Playing;
}

// Stops all sounds on all channels
void AudioSystem::StopAllSounds()
{
    Mix_HaltChannel(-1);

    for(auto & mChannel : mChannels)
    {
        mChannel.Reset();
    }

    mHandleMap.clear();
}

// Cache all sounds under Assets/Sounds
void AudioSystem::CacheAllSounds()
{
#ifndef __clang_analyzer__
    std::error_code ec{};
    for (const auto& rootDirEntry : std::filesystem::directory_iterator{"Assets/Sounds", ec})
    {
        std::string extension = rootDirEntry.path().extension().string();
        if (extension == ".ogg" || extension == ".wav")
        {
            std::string fileName = rootDirEntry.path().stem().string();
            fileName += extension;
            CacheSound(fileName);
        }
    }
#endif
}

// Used to preload the sound data of a sound
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
void AudioSystem::CacheSound(const std::string& soundName)
{
    GetSound(soundName);
}

// If the sound is already loaded, returns Mix_Chunk from the map.
// Otherwise, will attempt to load the file and save it in the map.
// Returns nullptr if sound is not found.
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
Mix_Chunk* AudioSystem::GetSound(const std::string& soundName)
{
    std::string fileName = "../assets/sounds/";
    fileName += soundName;

    Mix_Chunk* chunk = nullptr;
    auto iter = mSounds.find(fileName);
    if (iter != mSounds.end())
    {
        chunk = iter->second;
    }
    else
    {
        chunk = Mix_LoadWAV(fileName.c_str());
        if (!chunk)
        {
            SDL_Log("[AudioSystem] Failed to load sound file %s", fileName.c_str());
            return nullptr;
        }

        mSounds.emplace(fileName, chunk);
    }
    return chunk;
}

// Input for debugging purposes
void AudioSystem::ProcessInput(const Uint8* keyState)
{
    // Debugging code that outputs all active sounds on leading edge of period key
    if (keyState[SDL_SCANCODE_PERIOD] && !mLastDebugKey)
    {
        SDL_Log("[AudioSystem] Active Sounds:");
        for (size_t i = 0; i < mChannels.size(); i++)
        {
            if (mChannels[i].IsValid())
            {
                auto iter = mHandleMap.find(mChannels[i]);
                if (iter != mHandleMap.end())
                {
                    HandleInfo& hi = iter->second;
                    SDL_Log("Channel %d: %s, %s, looping = %d, paused = %d",
                            static_cast<unsigned>(i), mChannels[i].GetDebugStr(),
                            hi.mSoundName.c_str(), hi.mIsLooping, hi.mIsPaused);
                }
                else
                {
                    SDL_Log("Channel %d: %s INVALID", static_cast<unsigned>(i),
                            mChannels[i].GetDebugStr());
                }
            }
        }
    }

    mLastDebugKey = keyState[SDL_SCANCODE_PERIOD];
}