funcdef void OverlapCallback(ColliderComponent@, ColliderComponent@);

class OverlapEvent {
    private array<OverlapCallback@> callbacks;

    OverlapEvent(const OverlapEvent& other) delete;

    OverlapEvent& opAssign(const OverlapEvent&in) delete;

    void Subscribe(OverlapCallback@ callback) {
        callbacks.insertLast(callback);
    }

    void Unsubscribe(OverlapCallback@ callback) {
        const int idx = callbacks.findByRef(callback);
        if (idx < 0) {
            log_error("failed to unsubscribe from overlap event. unable to find passed callback");
            return;
        }
        callbacks.removeAt(idx);
    }

    void Broadcast(ColliderComponent@ caller, ColliderComponent@ other) {
        for (int i = callbacks.length() - 1; i >= 0; --i) {
            callbacks[i](caller, other);
        }
    }
}

abstract class ColliderComponent : CompositeComponent {
    OverlapEvent onOverlapBegin;
    OverlapEvent onOverlapEnd;
    
    protected Physics::PhysBody body;

    ColliderComponent(ICompositer@ parent = null) {
        super(parent);
    }

    void Init() {
        CreateBody();
        CompositeComponent::Init();
    }
    
    void CreateBody() {}
}

namespace Physics::Impl{
    void CallOnOverlapBegin(ref@ compHandleA, ref@ compHandleB) {
        ColliderComponent@ compA = cast<ColliderComponent@>(compHandleA);
        ColliderComponent@ compB = cast<ColliderComponent@>(compHandleB);
        if (compA is null || compB is null) {
            log_error("failed to call OnOverlapBegin event on colliders. invalid component type or null");
            return;
        }
        compA.onOverlapBegin.Broadcast(compA, compB);
        compB.onOverlapBegin.Broadcast(compB, compA);
    }

    void CallOnOverlapEnd(ref@ compHandleA, ref@ compHandleB) {
        ColliderComponent@ compA = cast<ColliderComponent@>(compHandleA);
        ColliderComponent@ compB = cast<ColliderComponent@>(compHandleB);
        if (compA is null || compB is null) {
            log_error("failed to call OnOverlapEnd event on colliders. invalid component type or null");
            return;
        }
        compA.onOverlapEnd.Broadcast(compA, compB);
        compB.onOverlapEnd.Broadcast(compB, compA);
    }
}
