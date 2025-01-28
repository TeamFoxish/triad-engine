class BuildingFoundationComponent : Component {
    bool isBusy = false;

    BuildingFoundationComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Init() {
        Moon3ad::gameState.foundations.insertLast(this);
    }

    void OnDestroy() {
        const int idx = Moon3ad::gameState.foundations.findByRef(this);
        if (idx >= 0) {
            Moon3ad::gameState.foundations.removeAt(idx);
        }
    }

    void MakeBusy(HealthComponent@ comp) {
        isBusy = true;
        Misc::EventHandler@ callback = Misc::EventHandler(this.HandleOnBuildingDied);
        comp.onDied += callback;
    }

    void HandleOnBuildingDied(ref@ caller) {
        isBusy = false;
    }
};
