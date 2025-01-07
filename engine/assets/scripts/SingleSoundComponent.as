class SingleSoundComponent : Component {
    protected Sound::Event event;
    protected bool allowFadeOnDestroy = true;
    protected bool isPlaying = false;

    protected string eventName;

    SingleSoundComponent(ICompositer@ parent) {
        super(parent);
    }

    void OnDestroy() {
        if (!event.IsValid()) {
            return;
        }
        event.Stop(allowFadeOnDestroy);
    }

    void Init() {
        Play();
    }

    void Update(float deltaTime) {
        if (!event.IsValid() || !event.Is3D()) {
            return;
        }
        const Math::Transform@ parentTrs = GetParent().GetTransform();
        event.Set3DAttributes(parentTrs);
    }

    void SetEvent(const string &in evtName) {
        eventName = evtName;
    }

    void Play() {
        if (eventName.isEmpty()) {
            return;
        }
        event = Sound::System::PlayEvent(eventName);
        isPlaying = true;
    }
}
