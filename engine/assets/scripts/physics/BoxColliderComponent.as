class BoxColliderComponent : ColliderComponent {
    [Editable]
    private Physics::ShapeBox shape;

    BoxColliderComponent(ICompositer@ parent = null) {
        super(parent);
    }

    void CreateBody() {
        body.InitFromShape(GetId(), shape);
    }

    void Init() {
        ColliderComponent::Init();
        OverlapCallback@ beginCallback = OverlapCallback(this.BeginOverlapTest);
        onOverlapBegin.Subscribe(beginCallback);
        OverlapCallback@ endCallback = OverlapCallback(this.EndOverlapTest);
        onOverlapEnd.Subscribe(endCallback);
    }

    void BeginOverlapTest(ColliderComponent@ caller, ColliderComponent@ other) {
        log_debug("BEGIN OVERLAP EVENT. caller: " + cast<CompositeComponent@>(caller.GetParent()).GetName() + ", other: " + cast<CompositeComponent@>(other.GetParent()).GetName());
    }

    void EndOverlapTest(ColliderComponent@ caller, ColliderComponent@ other) {
        log_debug("END OVERLAP EVENT. caller: " + cast<CompositeComponent@>(caller.GetParent()).GetName() + ", other: " + cast<CompositeComponent@>(other.GetParent()).GetName());
    }
}
