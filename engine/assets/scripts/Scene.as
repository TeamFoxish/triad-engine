namespace Scene {
    Component@ GetComponentUnderCursor() {
        const EntityId id = GetEntityIdUnderCursor();
        if (id == EntityInvalidId) {
            return null;
        }
        return Tree::GetComponentById(id);
    }

    namespace Tree {
        Component@ GetComponentById(EntityId id) {
            ref@ entity = GetEntityById(id);
            return cast<Component@>(entity);
        }
    }
}
