namespace Scene {
    namespace Tree {
        Component@ GetComponentById(EntityId id) {
            ref@ entity = GetEntityById(id);
            return cast<Component@>(entity);
        }
    }
}
