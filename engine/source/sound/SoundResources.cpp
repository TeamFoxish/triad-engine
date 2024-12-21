#include "SoundResources.h"

#include "SoundSystem.h"
#include "config/ConfigVar.h"
#include "logs/Logs.h"

#include "fmod_studio.hpp"
#include "fmod_errors.h"

static ConfigVar<int> cfgMaxEventPathLength("/Engine/Sound/MaxEventPathLength", 512);

void SoundResources::LoadBank(const Triad::FileIO::path& path)
{
    // TODO: fix string allocations?
    const Strid name = ToStrid(path.filename().string());

    // Prevent double-loading
    if (banksMap.find(name) != banksMap.end()) {
        return;
    }
    // Try to load bank
    FMOD::Studio::Bank* pBank = nullptr;
    FMOD_RESULT result = gSoundSys->fmodStudioSys->loadBankFile(
        path.string().c_str(), // File name of bank
        FMOD_STUDIO_LOAD_BANK_NORMAL, // Normal loading
        &pBank // Save pointer to bank
    );
    if (result != FMOD_OK) {
        LOG_ERROR("failed to load bank {}: {}", path.string(), FMOD_ErrorString(result));
        return;
    }

    // Add bank to map
    SoundBank& bank = banksMap[name];
    // Load all non-streaming sample data
    pBank->loadSampleData();
    // Get the number of events in this bank
    int numEvents = 0;
    pBank->getEventCount(&numEvents);
    if (numEvents <= 0) {
        return;
    }
    bank.events.reserve(numEvents);

    // Get list of event descriptions in this bank
    std::vector<FMOD::Studio::EventDescription*> events(numEvents);
    pBank->getEventList(events.data(), numEvents, &numEvents);
    std::vector<char> eventName(cfgMaxEventPathLength, '\0');
    for (int i = 0; i < numEvents; i++) {
        FMOD::Studio::EventDescription* eventDesc = events[i];
        // Get the path of this event (like event:/Explosion2D)
        eventDesc->getPath(eventName.data(), cfgMaxEventPathLength, nullptr);
        const Strid eventId = ToStrid(eventName.data());
        // Add to event map
        eventsMap.emplace(eventId, eventDesc);
        bank.events.push_back(eventId);
    }
}

void SoundResources::UnloadBank(Strid name)
{
    auto bankIter = banksMap.find(name);
    if (bankIter == banksMap.end()) {
        LOG_ERROR("attempt to unload bank which was never loaded");
        return;
    }

    SoundBank& bank = bankIter->second;
    for (Strid eventName : bank.events) {
        auto eventIter = eventsMap.find(eventName);
        assert(eventIter != eventsMap.end());
        eventsMap.erase(eventIter);
    }
    for (Strid busName : bank.buses) {
        auto busIter = busesMap.find(busName);
        assert(busIter != busesMap.end());
        busesMap.erase(busIter);
    }

	// Unload sample data and bank
	bank.pBank->unloadSampleData();
	bank.pBank->unload();
	// Remove from banks map
    banksMap.erase(bankIter);
}

void SoundResources::UnloadAllBanks()
{
    for (auto& [name, _] : banksMap) {
        UnloadBank(name);
    }
}

FMOD::Studio::EventDescription* SoundResources::GetEventDesc(Strid name) const
{
    auto iter = eventsMap.find(name);
    if (iter == eventsMap.end()) {
        return nullptr;
    }
    return iter->second;
}

Strid SoundResources::GetEventName(FMOD::Studio::EventDescription* desc) const
{
    const auto iter = std::find_if(eventsMap.begin(), eventsMap.end(), 
        [desc](const std::pair<const Strid, FMOD::Studio::EventDescription*> entry) { return entry.second == desc; });
    return iter != eventsMap.end() ? iter->first : Strid{};
}
