class SoundEmitterComponent : Component {
    protected array<Sound::Event> sndEvents;
    [Editable]
    protected bool allowFadeOnDestroy = true;

    SoundEmitterComponent(ICompositer@ parent = null) {
        super(parent);
    }

    void OnDestroy() {
        for (uint i = 0; i < sndEvents.length(); ++i) {
            Sound::Event event = sndEvents[i];
            if (!event.IsValid()) {
                continue;
            }
            event.Stop(allowFadeOnDestroy);
        }
    }

    void Update(float deltaTime) {
        array<int> invalidEvents;
        for (uint i = 0; i < sndEvents.length(); ++i) {
            Sound::Event event = sndEvents[i];
            if (!event.IsValid()) {
                invalidEvents.insertLast(i);
                continue;
            }
            UpdateEvent(event);
        }
        for (int i = int(invalidEvents.length()) - 1; i >= 0; --i) {
            sndEvents.removeAt(invalidEvents[i]);
        }
    }

    void PlayEvent(const string &in name) {
        Sound::Event event = Sound::System::PlayEvent(name);
        UpdateEvent(event);
        sndEvents.insertLast(event);
    }

    protected void UpdateEvent(Sound::Event &in event) {
        if (!event.Is3D()) {
            return;
        }
        const Math::Transform@ parentTrs = GetParent().GetTransform();
        event.Set3DAttributes(parentTrs);
    }
}
