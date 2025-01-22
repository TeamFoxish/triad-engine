#include "SoundBindings.h"

#include "scripts/ScriptSystem.h"
#include "SoundSystem.h"
#include "shared/MathScriptBindings.h"
#include "shared/SharedStorage.h"
#include "logs/Logs.h"

class CSoundEvent : CNativeObject {
	friend void RegisterSoundBindings();

public:
    CSoundEvent() = default;

    CSoundEvent(SoundSystem::EventHandle _handle)
        : handle(_handle)
    {
#ifndef NDEBUG
		if (!IsValid()) {
			return;
		}
		FMOD::Studio::EventDescription* desc = GetEvent().GetDesc();
		if (desc) {
			name = gSoundSys->sndRes.GetEventName(desc);
		}
#endif
    }

	bool IsValid() const
	{
		return handle.id_ >= 0 && gSoundSys->IsValidEvent(handle);
	}

	SoundEvent& GetEvent() const { return gSoundSys->GetEvent(handle); }

	// Play this event
	void Play() 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to play invalid event `{}`", name.string());
			return;
		}
		GetEvent().Play();
	}

	// Restart event from beginning
	void Restart() { Play(); }

	// Stop this event
	void Stop(bool allowFadeOut = true)
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to stop invalid event `{}`", name.string());
			return;
		}
		GetEvent().Stop(allowFadeOut);
	}

	void Pause() { SetPaused(true); }

	void Resume() { SetPaused(false); }

	// Setters
	void SetPaused(bool pause) 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to set paused for invalid event `{}`", name.string());
			return;
		}
		GetEvent().SetPaused(pause);
	}

	void SetVolume(float value) 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to set volume for invalid event `{}`", name.string());
			return;
		}
		GetEvent().SetVolume(value);
	}

	void SetPitch(float value) 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to set pitch for invalid event `{}`", name.string());
			return;
		}
		GetEvent().SetPitch(value);
	}

	void SetParameter(const std::string& paramName, float value) 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to set parameter '{}' for invalid event `{}`", paramName, name.string());
			return;
		}
		GetEvent().SetParameter(ToStrid(paramName), value);
	}

	// Getters
	bool IsPaused() const 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to get paused state from invalid event `{}`", name.string());
			return false;
		}
		return GetEvent().IsPaused();
	}

	float GetVolume() const 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to get volume from invalid event `{}`", name.string());
			return 0.0f;
		}
		return GetEvent().GetVolume();
	}

	float GetPitch() const 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to get pitch from invalid event `{}`", name.string());
			return 0.0f;
		}
		return GetEvent().GetPitch();
	}

	float GetParameter(const std::string& paramName) 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to get parameter `{}` value from invalid event `{}`", paramName, name.string());
			return 0.0f;
		}
		return GetEvent().GetParameter(ToStrid(paramName));
	}

	// Positional
	bool Is3D() const 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to find out if event is3D from invalid event `{}`", name.string());
			return false;
		}
		return GetEvent().Is3D();
	}

	void Set3DAttributes(const CTransformHandle& trsHandle) 
	{
		if (!gSoundSys->IsValidEvent(handle)) {
			LOG_ERROR("an attempt to find out if event is3D from invalid event `{}`", name.string());
			return;
		}
		SoundEvent& event = GetEvent();
		if (!event.Is3D()) {
			LOG_ERROR("an attempt to set 3d attributes to 2d event `{}`", name.string());
			return;
		}
		const Math::Transform& transform = SharedStorage::Instance().transforms.AccessRead(trsHandle.GetHandle());
		event.Set3DAttributes(transform.GetMatrix());
	}

private:
	static void CreateDefault(CSoundEvent* self) { new(self) CSoundEvent(); }
	static void CreateCopy(CSoundEvent* self, const CSoundEvent& other) { new(self) CSoundEvent(other); }
	static void Destroy(CSoundEvent* self) { self->~CSoundEvent(); }

private:
    SoundSystem::EventHandle handle;

#ifdef LOG_ENABLED
	Strid name;
#endif
};

struct CSoundSystem {
	static CSoundEvent PlayEvent(const std::string& name)
	{
		SoundSystem::EventHandle handle = gSoundSys->PlayEvent(ToStrid(name));
		if (handle.id_ < 0) {
			return CSoundEvent();
		}
		return CSoundEvent(handle);
	}

	static void PlaySoundFile(const std::string& path, float volume, bool isLoop, bool is3D, const CTransformHandle* transform = nullptr) 
	{
		const TransformStorage::Handle trsHandle = is3D && transform ? transform->GetHandle() : TransformStorage::Handle{};
		const Triad::FileIO::path filePath(path);
		gSoundSys->PlaySoundFile(filePath, volume, isLoop, is3D, trsHandle);
	}
};

void RegisterSoundBindings()
{
	auto engine = gScriptSys->GetRawEngine();
	int r;

	r = engine->SetDefaultNamespace("Sound"); assert(r >= 0);

	r = engine->RegisterObjectType("Event", sizeof(CSoundEvent), asOBJ_VALUE | asGetTypeTraits<CSoundEvent>()); assert(r >= 0);
	CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
	r = engine->RegisterObjectBehaviour("Event", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(CSoundEvent::CreateDefault), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Event", asBEHAVE_CONSTRUCT, "void f(const Event &in)", asFUNCTION(CSoundEvent::CreateCopy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("Event", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(CSoundEvent::Destroy), asCALL_CDECL_OBJFIRST); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "Event &opAssign(const Event &in)", asMETHODPR(CSoundEvent, operator=, (const CSoundEvent&), CSoundEvent&), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "bool IsValid() const", asMETHOD(CSoundEvent, IsValid), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void Play()", asMETHOD(CSoundEvent, Play), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void Restart()", asMETHOD(CSoundEvent, Restart), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void Stop(bool allowFade = true)", asMETHOD(CSoundEvent, Stop), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void Pause()", asMETHOD(CSoundEvent, Pause), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void Resume()", asMETHOD(CSoundEvent, Resume), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void SetPaused(bool pause)", asMETHOD(CSoundEvent, SetPaused), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "float GetVolume() const", asMETHOD(CSoundEvent, GetVolume), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void SetVolume(float volume)", asMETHOD(CSoundEvent, SetVolume), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "float GetPitch() const", asMETHOD(CSoundEvent, GetPitch), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void SetPitch(float pitch)", asMETHOD(CSoundEvent, SetPitch), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "float GetParameter(const string &in name) const", asMETHOD(CSoundEvent, GetParameter), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void SetParameter(const string &in name, float value)", asMETHOD(CSoundEvent, SetParameter), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "bool Is3D() const", asMETHOD(CSoundEvent, Is3D), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("Event", "void Set3DAttributes(const Math::Transform@+ transform) const", asMETHOD(CSoundEvent, Set3DAttributes), asCALL_THISCALL); assert(r >= 0);

	r = engine->SetDefaultNamespace("Sound::System"); assert(r >= 0);

	r = engine->RegisterGlobalFunction("Event PlayEvent(const string &in name)", asFUNCTION(CSoundSystem::PlayEvent), asCALL_CDECL); assert(r >= 0);
	r = engine->RegisterGlobalFunction("void PlaySoundFile(const string &in path, float volume, bool isLoop, bool is3D, const Math::Transform@+ transform = null)", asFUNCTION(CSoundSystem::PlaySoundFile), asCALL_CDECL); assert(r >= 0);

	r = engine->SetDefaultNamespace(""); assert(r >= 0);
}
