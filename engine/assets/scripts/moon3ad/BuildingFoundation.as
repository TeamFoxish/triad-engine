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
};
