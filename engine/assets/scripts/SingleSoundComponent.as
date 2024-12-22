class SingleSoundComponent : Component {
    protected Sound::Event event;
    protected bool allowFadeOnDestroy = true;

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
        if (eventName.isEmpty()) {
            return;
        }
        event = Sound::System::PlayEvent(eventName);
    }

    void Update(float deltaTime) {
        if (!event.IsValid() || !event.Is3D()) {
            return;
        }
        const Math::Transform@ parentTrs = GetParent().GetTransform();
        event.Set3DAttributes(parentTrs);
    }
}
