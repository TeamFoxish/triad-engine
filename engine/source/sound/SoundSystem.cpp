#include "SoundSystem.h"

#include "file/FileIO.h"
#include "config/ConfigVar.h"
#include "logs/Logs.h"
#include "shared/SharedStorage.h"

#include "fmod_studio.hpp"
#include "fmod_errors.h"

static ConfigVar<int> cfgMaxConcurrentSounds("/Engine/Sound/MaxConcurrentSounds", 512);
static ConfigVar<std::string_view> cfgSoundAssetsPath("/Engine/Sound/AssetsPath", "assets/sound");
static ConfigVar<std::string_view> cfgSoundDesktopPlatformName("/Engine/Sound/DesktopPlatformName", "Desktop");

std::unique_ptr<SoundSystem> gSoundSys;

bool SoundSystem::Init()
{
#ifdef DEBUG
    FMOD::Debug_Initialize(
        FMOD_DEBUG_LEVEL_ERROR, // Log only errors
        FMOD_DEBUG_MODE_TTY // Output to stdout
    );
#endif

    // create fmod studio system
    FMOD_RESULT result;
    result = FMOD::Studio::System::create(&fmodStudioSys);
    if (result != FMOD_OK) {
        LOG_ERROR("failed to create FMOD system: {}", FMOD_ErrorString(result));
        return false;
    }


    result = fmodStudioSys->initialize(
        cfgMaxConcurrentSounds, // Max number of concurrent sounds
        FMOD_STUDIO_INIT_NORMAL, // Use default settings
        FMOD_INIT_NORMAL, // Use default settings
        nullptr // Usually null
    );
    if (result != FMOD_OK) {
        LOG_ERROR("failed to initialize FMOD system: {}", FMOD_ErrorString(result));
        return false;
    }

    fmodStudioSys->getCoreSystem(&fmodCoreSys);

#ifdef _WIN32
    const Triad::FileIO::path banksPath = Triad::FileIO::path(cfgSoundAssetsPath.GetRef()) / cfgSoundDesktopPlatformName.GetRef();
    const Triad::FileIO::path masterStringsPath = banksPath / "Master.strings.bank";
    sndRes.LoadBank(masterStringsPath);
    const Triad::FileIO::path masterPath = banksPath / "Master.bank";
    sndRes.LoadBank(masterPath);
#else
    #error sound system currenlty has only windows support
#endif

    return true;
}

void SoundSystem::Term()
{
    StopAllSounds();
    sndRes.UnloadAllBanks();
    fmodStudioSys->release();
}

void SoundSystem::Update(float deltaTime)
{
    // Find any stopped event instances
    std::vector<EventHandle> done;
    for (EventStorage::Index idx = 0; idx < events.Length(); ++idx) {
        SoundEvent& event = events[idx];

        // Get the state of this event
        FMOD_STUDIO_PLAYBACK_STATE state;
        event.pEvent->getPlaybackState(&state);
        if (state == FMOD_STUDIO_PLAYBACK_STOPPED) {
            done.push_back(events.GetHandleFromIndex(idx));
        }
    }
    // Remove done event instances from map
    for (EventHandle handle : done) {
        events.Remove(handle);
    }

    UpdateSounds(deltaTime);

    fmodStudioSys->update();
}

auto SoundSystem::PlayEvent(Strid name) -> EventHandle
{
    // Make sure event exists
    FMOD::Studio::EventDescription* eventDesc = sndRes.GetEventDesc(name);
    if (!eventDesc) {
        LOG_ERROR("failed to find loaded event {}", name.string());
        return EventHandle{};
    }

    // Create instance of event
    FMOD::Studio::EventInstance* pEvent = nullptr;
    FMOD_RESULT rc = eventDesc->createInstance(&pEvent);
    if (rc != FMOD_OK || !pEvent) {
        LOG_ERROR("failed to create event {} instance: {}", name.string(), FMOD_ErrorString(rc));
        return EventHandle{};
    }

    SoundEvent event(pEvent);

    // Start the event instance
    event.Play();

    return events.Add(std::move(event));
}

void SoundSystem::PlaySoundFile(const Triad::FileIO::path& path, float volume, bool isLoop, bool is3D, TransformStorage::Handle transform)
{
    SoundEntry entry;
    entry.transform = transform;

    // configure mode
    FMOD_MODE mode = FMOD_DEFAULT;
    mode |= isLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    mode |= is3D ? FMOD_3D : FMOD_2D;

    // create sound
    FMOD_RESULT rc = fmodCoreSys->createSound(path.string().c_str(), mode, nullptr, &entry.sound);
    if (rc != FMOD_OK) {
        LOG_ERROR("failed to load media file `{}`: {}", path.string(), FMOD_ErrorString(rc));
        return;
    }

    // create channel group
    /*if (!masterChannelGroup) {
        rc = fmodCoreSys->createChannelGroup("master", &masterChannelGroup);
        if (rc != FMOD_OK) {
            LOG_ERROR("failed to create master channel group: {}", FMOD_ErrorString(rc));
            return;
        }
    }*/

    rc = fmodCoreSys->playSound(entry.sound, nullptr, false, &entry.channel);
    if (rc != FMOD_OK) {
        LOG_ERROR("failed to play media file `{}`: {}", path.string(), FMOD_ErrorString(rc));
        entry.sound->release();
        return;
    }

    entry.channel->setVolume(volume);

    sounds.Add(std::move(entry));
}

void SoundSystem::StopAllSounds()
{
    for (EventStorage::Index idx = 0; idx < events.Length(); ++idx) {
        SoundEvent& event = events[idx];

        // Get the state of this event
        FMOD_STUDIO_PLAYBACK_STATE state;
        event.pEvent->getPlaybackState(&state);
        if (state != FMOD_STUDIO_PLAYBACK_STOPPED) {
            event.Stop(false);
        }
    }
    events.Clear();

    for (SoundStorage::Index idx = 0; idx < sounds.Length(); ++idx) {
        SoundEntry& entry = sounds[idx];

        // Get the state of this sound
        bool isPlaying = false;
        entry.channel->isPlaying(&isPlaying);
        if (isPlaying) {
            entry.channel->stop();
        }
        entry.sound->release();
    }
    sounds.Clear();
}

void SoundSystem::SetListener(const Math::Matrix& viewMatrix)
{
    // Invert the view matrix to get the correct vectors
    Math::Matrix invView = viewMatrix.Invert();
    FMOD_3D_ATTRIBUTES listener;
    listener.position = FMOD::VecToFMOD(invView.Translation());
    listener.forward = FMOD::VecToFMOD(invView.Forward());
    listener.up = FMOD::VecToFMOD(invView.Up());
    fmodStudioSys->setListenerAttributes(0, &listener);
}

void SoundSystem::UpdateSounds(float deltaTime)
{
    // Find any stopped sound instances
    std::vector<SoundHandle> done;
    for (SoundStorage::Index idx = 0; idx < sounds.Length(); ++idx) {
        SoundEntry& entry = sounds[idx];
        bool isPlaying = false;
        entry.channel->isPlaying(&isPlaying);
        if (!isPlaying) {
            done.push_back(sounds.GetHandleFromIndex(idx));
            continue;
        }
        if (entry.transform.id_ < 0 || !SharedStorage::Instance().transforms.IsValidHandle(entry.transform)) {
            continue;
        }
        const Math::Transform& transform = SharedStorage::Instance().transforms.AccessRead(entry.transform);
        const FMOD_VECTOR position = FMOD::VecToFMOD(transform.GetMatrix().Translation());
        const FMOD_VECTOR velocity = {0.0f, 0.0f, 0.0f};
        entry.channel->set3DAttributes(&position, &velocity);
    }
    // Remove done sounds from map
    for (SoundHandle handle : done) {
        sounds.Remove(handle);
    }
}

bool InitSoundSystem()
{
    gSoundSys = std::make_unique<SoundSystem>();
    return gSoundSys->Init();
}

void TermSoundSystem()
{
    gSoundSys->Term();
    gSoundSys.reset();
}
