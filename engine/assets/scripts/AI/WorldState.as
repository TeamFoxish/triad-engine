
class WorldState {
    private dictionary state;
    private bool isDirty;
    
    bool GetBool(const string &in name) const {
        return bool(state[name]);
    }

    void SetBool(const string &in name, bool value) {
        state[name] = value;
    }

    void ReplanningWrite(const string &in name, any@ value) {
        @state[name] = @value;
        isDirty = true;
    }

    bool IsDirty() const {
        return isDirty;
    }
}