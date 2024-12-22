#pragma once

#include "Sound.h"

#include "misc/Strid.h"
#include "file/FileIO.h"

#include <unordered_map>
#include <vector>

class SoundResources {
private:
    struct SoundBank;
    using BanksMap = std::unordered_map<Strid, SoundBank>;

public:
    SoundResources() = default;
    SoundResources(const SoundResources&) = delete;
    SoundResources(SoundResources&&) = delete;
    ~SoundResources() = default;

    void LoadBank(const Triad::FileIO::path& path);
    BanksMap::iterator UnloadBank(Strid name);
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
    BanksMap banksMap;
    // Map of event name to EventDescription
    std::unordered_map<Strid, FMOD::Studio::EventDescription*> eventsMap;
    // Map of buses
    std::unordered_map<Strid, FMOD::Studio::Bus*> busesMap;
};
