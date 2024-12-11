class ComponentHandle {
    private uint64 _componentId;
    private Component@ _realComponent;

    Component@ Get() {
        if (_realComponent is null) {
            @_realComponent = GetFromStorage(_componentId);
        }
        return _realComponent;
    }

    void SetId(uint64 componentId) {
        if (componentId != _componentId) {
            @_realComponent = null;
        }
        _componentId = componentId;
    }

    bool IsLoaded() {
        return _realComponent !is null;
    }
};