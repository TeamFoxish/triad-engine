
class WorldState {
    private dictionary@ state = @dictionary();
    private bool isDirty;
    
    bool GetBool(const string &in name) const {
        if (state.exists(name)) {
            return bool(state[name]);
        } else {
            return false;
        }
    }

    void SetBool(const string &in name, bool value) {
        state[name] = value;
    }

    Math::Vector3 GetVector(const string &in name) {
        if (state.exists(name)) {
            return Math::Vector3(state[name]);
        } else {
            return Math::Vector3(0, 0, 0);
        }
    }

    void SetVector(const string &in name, Math::Vector3 value) {
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