#pragma once

#include "fmod_common.h"
#include "math/Math.h"

#include <memory>

namespace FMOD {
    namespace Studio {
        class System;
        class Bank;
        class EventDescription;
        class EventInstance;
        class Bus;
    }
    class System;
    class Sound;
    class ChannelGroup;
    class Channel;

    inline FMOD_VECTOR VecToFMOD(Math::Vector3 in)
    {
        // FMOD (+z forward, +x right, +y up)
        FMOD_VECTOR v;
        v.x = in.x;
        v.y = in.y;
        v.z = -in.z;
        return v;
    }
}

extern std::unique_ptr<class SoundSystem> gSoundSys;
