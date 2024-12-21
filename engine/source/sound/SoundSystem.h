#pragma once

#include "SoundResources.h"
#include "SoundEvent.h"
#include "misc/Handles.h"

#include <string>

class SoundSystem {
    friend SoundResources;

public:
    using EventStorage = HandleStorage<SoundEvent>;
    using EventHandle = EventStorage::Handle;

    bool Init();
    void Term();

    void Update(float deltaTime);

    EventHandle PlayEvent(Strid name);
    void StopAllSounds();

    bool IsValidEvent(EventHandle handle) const { return events.Get(handle) != nullptr; }
    SoundEvent& GetEvent(EventHandle handle) { return events[handle]; }
    const SoundEvent& GetEvent(EventHandle handle) const { return events[handle]; }

    void SetListener(const Math::Matrix& viewMatrix);

public:
    SoundResources sndRes;

private:
    EventStorage events;

    // FMOD studio system
    FMOD::Studio::System* fmodStudioSys;
    // FMOD Low-level system (in case needed)
    FMOD::System* fmodCoreSys;
};

bool InitSoundSystem();
void TermSoundSystem();
