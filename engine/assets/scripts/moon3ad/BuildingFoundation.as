class BuildingFoundationComponent : Component {
    bool isBusy = false;
    bool isWorking = false;
    bool isMarkedForConstruction = false;
    bool isUnderConstraction = false;

    BuildingFoundationComponent(ICompositer@ parent = null) {
        super(@parent);
    }

    void Init() {
        Moon3ad::gameState.foundations.insertLast(this);
    }

    void OnDestroy() {
        isBusy = false;
        isWorking = false;
        isMarkedForConstruction = false;
        isUnderConstraction = false;
    }

    void MakeBusy(HealthComponent@ comp) {
        isBusy = true;
        Misc::EventHandler@ callback = Misc::EventHandler(this.HandleOnBuildingDied);
        Misc::EventHandler@ damagedCallback = Misc::EventHandler(this.HandleOnBuildingDamaged);
        comp.onDied += callback;
        comp.onDamaged += damagedCallback;
    }

    void SetWorking(bool isWorking) {
        this.isWorking = isWorking;
    }

    bool IsWorking() {
        return this.isWorking;
    }

    void SetMarkedForConstruction(bool _isMarkedForConstruction) {
        this.isMarkedForConstruction = _isMarkedForConstruction;
    }

    bool IsMarkedForConstruction() {
        return this.isMarkedForConstruction;
    }

    void SetIsUnderConstruction(bool _isUnderConstruction) {
        this.isUnderConstraction = _isUnderConstruction;
    }

    bool IsUnderConstruction() {
        return this.isUnderConstraction;
    }

    void HandleOnBuildingDied(ref@ caller) {
        isBusy = false;
    }

    void HandleOnBuildingDamaged(ref@ caller) {
        Moon3ad::gameState.AddDamagedConstruction(cast<HealthComponent>(caller));
    }
};
