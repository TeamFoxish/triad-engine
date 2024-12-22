#pragma once

#include "SoundResources.h"
#include "SoundEvent.h"
#include "misc/Handles.h"
#include "shared/TransformStorage.h" // TEMP

#include <string>

class SoundSystem {
    friend SoundResources;

public:
    using EventStorage = HandleStorage<SoundEvent>;
    using EventHandle = EventStorage::Handle;

    // an alternative low-level sound utilized to play media files directly
    struct SoundEntry {
        FMOD::Sound* sound = nullptr;
        FMOD::Channel* channel = nullptr;
        TransformStorage::Handle transform;
    };
    using SoundStorage = HandleStorage<SoundEntry>;
    using SoundHandle = SoundStorage::Handle;

    bool Init();
    void Term();

    void Update(float deltaTime);

    EventHandle PlayEvent(Strid name);
    void PlaySoundFile(const Triad::FileIO::path& path, float volume, bool isLoop, bool is3D, TransformStorage::Handle transform = TransformStorage::Handle{});
    void StopAllSounds();

    bool IsValidEvent(EventHandle handle) const { return events.Get(handle) != nullptr; }
    SoundEvent& GetEvent(EventHandle handle) { return events[handle]; }
    const SoundEvent& GetEvent(EventHandle handle) const { return events[handle]; }

    void SetListener(const Math::Matrix& viewMatrix);

private:
    void UpdateSounds(float deltaTime);

public:
    SoundResources sndRes;

private:
    EventStorage events;

    SoundStorage sounds;

    // FMOD studio system
    FMOD::Studio::System* fmodStudioSys = nullptr;
    // FMOD Low-level system (in case needed)
    FMOD::System* fmodCoreSys = nullptr;

    // TODO: remove?
    FMOD::ChannelGroup* masterChannelGroup = nullptr;
};

bool InitSoundSystem();
void TermSoundSystem();
