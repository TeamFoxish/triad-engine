dictionary componentStorage = {};

uint64 componentIdCounter = 0;

void AddToStorage(BaseComponent@ component) {
    component.id = componentIdCounter;
    componentStorage[formatInt(componentIdCounter, "l", 32)] = component;
    componentIdCounter = componentIdCounter + 1;
}

void AddToStorage(BaseComponent@ component, uint64 id) {
    componentStorage[formatInt(id, "l", 32)] = component;
    if (componentIdCounter < id) {
        componentIdCounter = id + 1;
    }
    component.id = id;
}

Component@ GetFromStorage(uint64 id) {
    return cast<Component@>(componentStorage[formatInt(id, 'l', 32)]);
}