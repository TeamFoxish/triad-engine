#pragma once

#ifdef EDITOR

#include "scene/SceneTree.h"
#include "scene/SceneLoader.h"

enum class FieldAttributes {
    None = 0,
    Show = 0x01,
    Editable = 0x02,
};

class EditorInspector {
    friend struct FieldTypes;

public:
    EditorInspector(SceneTree::Handle entHandle);

    void Draw();

private:
    using ObjRaw = asIScriptObject;
    using FieldIdx = asUINT;

    class FieldBase {
    public:
        FieldBase(FieldIdx _idx, FieldAttributes _flags)
            : fieldIdx(_idx)
            , flags(_flags)
        {
        }

        virtual void Show(const SceneTree::Entity& entity) = 0;

        virtual bool IsDisabled() const;

    protected:
        template<typename ValT>
        void UpdateSceneNodeValue(const SceneTree::Entity& entity, const char* fieldName, const ValT& val);

    protected:
        FieldIdx fieldIdx;
        FieldAttributes flags;
    };

    class Component {
    public:
        Component(const SceneTree::Entity& entity);

        void Show(const SceneTree::Entity& entity) const;

    private:
        std::vector<std::unique_ptr<FieldBase>> fields;
    };

    std::vector<std::pair<SceneTree::Handle, Component>> components;
};

template<typename ValT>
inline void EditorInspector::FieldBase::UpdateSceneNodeValue(const SceneTree::Entity& entity, const char* fieldName, const ValT& val)
{
    std::optional<YAML::Node> sceneNodeOpt = 
        SceneLoader::FindSpawnedComponent(entity.obj);
    if (!sceneNodeOpt) {
        return;
    }
    YAML::Node& sceneNode = *sceneNodeOpt;
    sceneNode["overrides"][fieldName] = val;
}

#endif // EDITOR
