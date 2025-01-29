
class WorldState {
    private dictionary state;
    private bool isDirty;

    WorldState() {
    }

    WorldState(const WorldState &inout other) {
        this.state = other.state;
        this.isDirty = other.isDirty;
    }

    bool IsPropertyExists(const string &in name) const {
        return state.exists(name);
    }
    
    bool GetBool(const string &in name) const {
        if (state.exists(name)) {
            return bool(state[name]);
        } else {
            log_warn("Unexisting bool propery \"" + name + "\" acessed.");
            return false;
        }
    }



    void SetBool(const string &in name, bool value) {
        state[name] = value;
    }

    Math::Vector3 GetVector(const string &in name) const {
        if (state.exists(name)) {
            return Math::Vector3(state[name]);
        } else {
            log_warn("Unexisting Vector3 propery \"" + name + "\" acessed.");
            return Math::Vector3(0, 0, 0);
        }
    }

    void SetVector(const string &in name, Math::Vector3 value) {
        state[name] = value;
    }

    string GetString(const string &in name) const {
        if (state.exists(name)) {
            return string(state[name]);
        } else {
            log_warn("Unexisting string propery \"" + name + "\" acessed.");
            return "";
        }
    }

    void SetString(const string &in name, const string &in value) {
        state[name] = value;
    }

    float GetFloat(const string &in name) const {
        if (state.exists(name)) {
            return float(state[name]);
        } else {
            log_warn("Unexisting float propery \"" + name + "\" acessed.");
            return 0;
        }
    }

    void SetRef(const string &in name, any@ value) {
        state[name] = value;
    }

    any@ GetRef(const string &in name) const {
        if (state.exists(name)) {
            return cast<any@>(state[name]);
        } else {
            log_warn("Unexisting ref@ propery \"" + name + "\" acessed.");
            return null;
        }
    }

    void SetFloat(const string &in name, float value) {
        state[name] = value;
    }

    void ReplanningBoolWrite(const string &in name, bool value) {
        SetBool(name, value);
        isDirty = true;
    }

    void ReplanningWrite(const string &in name, any@ value) {
        @state[name] = @value;
        isDirty = true;
    }

    bool IsDirty() const {
        return isDirty;
    }
}