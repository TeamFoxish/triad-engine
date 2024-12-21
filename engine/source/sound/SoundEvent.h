#pragma once

#include "Sound.h"

#include "misc/Strid.h"

class SoundEvent
{
	friend class SoundSystem;

public:
	SoundEvent() = default;
	SoundEvent(const SoundEvent& other) = delete;
	SoundEvent(SoundEvent&& other) noexcept = default;
	~SoundEvent();

	SoundEvent& operator=(SoundEvent&& other) = default; // required for std:swap operations by HandleStorage

	// Play this event
	void Play();

	// Restart event from beginning
	void Restart() { Play(); }

	// Stop this event
	void Stop(bool allowFadeOut = true);

	// Setters
	void SetPaused(bool pause);
	void SetVolume(float value);
	void SetPitch(float value);
	void SetParameter(Strid name, float value);

	// Getters
	bool IsPaused() const;
	float GetVolume() const;
	float GetPitch() const;
	float GetParameter(Strid name);
	Strid GetName() const;

	FMOD::Studio::EventDescription* GetDesc() const;

	// Positional
	bool Is3D() const;
	void Set3DAttributes(const Math::Matrix& worldTrans);

protected:
	SoundEvent(FMOD::Studio::EventInstance* pEvent);

private:
	FMOD::Studio::EventInstance* pEvent = nullptr;
};
