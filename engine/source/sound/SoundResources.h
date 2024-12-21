#pragma once

#include "Sound.h"

#include "misc/Strid.h"
#include "file/FileIO.h"

#include <unordered_map>
#include <vector>

class SoundResources {
public:
    SoundResources() = default;
    SoundResources(const SoundResources&) = delete;
    SoundResources(SoundResources&&) = delete;
    ~SoundResources() = default;

    void LoadBank(const Triad::FileIO::path& path);
    void UnloadBank(Strid name);
    void UnloadAllBanks();

    FMOD::Studio::EventDescription* GetEventDesc(Strid name) const;
    Strid GetEventName(FMOD::Studio::EventDescription* desc) const;

private:
    struct SoundBank {
        FMOD::Studio::Bank* pBank;
        std::vector<Strid> events;
        std::vector<Strid> buses;
    };

    // Map of loaded banks
    std::unordered_map<Strid, SoundBank> banksMap;
    // Map of event name to EventDescription
    std::unordered_map<Strid, FMOD::Studio::EventDescription*> eventsMap;
    // Map of buses
    std::unordered_map<Strid, FMOD::Studio::Bus*> busesMap;
};
