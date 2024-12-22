#include "SoundEvent.h"

#include "SoundSystem.h"
#include "logs/Logs.h"

#include "fmod_studio.hpp"
#include "fmod_errors.h"

SoundEvent::SoundEvent(FMOD::Studio::EventInstance* _pEvent)
	: pEvent(_pEvent)
{
}

SoundEvent::~SoundEvent()
{
	if (pEvent) {
		pEvent->release();
	}
}

void SoundEvent::Play()
{
	pEvent->start();
}

void SoundEvent::Stop(bool allowFadeOut /* true */)
{
	FMOD_STUDIO_STOP_MODE mode = allowFadeOut ?
		FMOD_STUDIO_STOP_ALLOWFADEOUT :
		FMOD_STUDIO_STOP_IMMEDIATE;
	pEvent->stop(mode);
}

void SoundEvent::SetPaused(bool pause)
{
	pEvent->setPaused(pause);
}

void SoundEvent::SetVolume(float value)
{
	pEvent->setVolume(value);
}

void SoundEvent::SetPitch(float value)
{
	pEvent->setPitch(value);
}

void SoundEvent::SetParameter(Strid name, float value)
{
	pEvent->setParameterByName(name.string(), value);
}

bool SoundEvent::IsPaused() const
{
	bool retVal = false;
	pEvent->getPaused(&retVal);
	return retVal;
}

float SoundEvent::GetVolume() const
{
	float retVal = 0.0f;
	pEvent->getVolume(&retVal);
	return retVal;
}

float SoundEvent::GetPitch() const
{
	float retVal = 0.0f;
	pEvent->getPitch(&retVal);
	return retVal;
}

float SoundEvent::GetParameter(Strid name)
{
	float retVal = 0.0f;
	pEvent->getParameterByName(name.string(), &retVal);
	return retVal;
}

Strid SoundEvent::GetName() const
{
	FMOD::Studio::EventDescription* desc = GetDesc();
	return desc ? gSoundSys->sndRes.GetEventName(desc) : Strid{};
}

FMOD::Studio::EventDescription* SoundEvent::GetDesc() const
{
	FMOD::Studio::EventDescription* desc = nullptr;
	pEvent->getDescription(&desc);
	return desc;
}

bool SoundEvent::Is3D() const
{
	bool retVal = false;

	// Get the event description
	FMOD::Studio::EventDescription* desc = nullptr;
	pEvent->getDescription(&desc);
	if (desc) {
		desc->is3D(&retVal);
	}
	return retVal;
}

void SoundEvent::Set3DAttributes(const Math::Matrix& worldTrans)
{
	FMOD_3D_ATTRIBUTES attr;
	// Set position, forward, up
	attr.position = FMOD::VecToFMOD(worldTrans.Translation());
	attr.forward = FMOD::VecToFMOD(worldTrans.Forward());
	attr.up = FMOD::VecToFMOD(worldTrans.Up());
	// Set velocity to zero (fix if using Doppler effect)
	attr.velocity = { 0.0f, 0.0f, 0.0f };
	pEvent->set3DAttributes(&attr);
}
