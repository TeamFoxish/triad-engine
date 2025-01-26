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

        virtual void Show(const ScriptObject& scpObj, const ScriptObject& compObj, std::vector<std::string_view>& path) = 0;

        virtual bool IsDisabled() const;

    protected:
        template<typename ValT>
        static void UpdateSceneNodeValue(const ScriptObject& entity, const char* fieldName, const ValT& val, const std::vector<std::string_view>& path);

    protected:
        FieldIdx fieldIdx;
        FieldAttributes flags;
    };

    class Component {
    public:
        Component(const SceneTree::Entity& entity);

        void Show(const SceneTree::Entity& entity) const;

        bool IsPendingRemove() const { return pendingRemove; }

    private:
        std::vector<std::unique_ptr<FieldBase>> fields;
        mutable bool pendingRemove = false;
    };

    std::vector<std::pair<SceneTree::Handle, Component>> components;

    static constexpr float INDENT = 16.0f;
};

template<typename ValT>
inline void EditorInspector::FieldBase::UpdateSceneNodeValue(const ScriptObject& obj, const char* fieldName, const ValT& val, const std::vector<std::string_view>& path)
{
    std::optional<YAML::Node> sceneNodeOpt = 
        SceneLoader::FindSpawnedComponent(obj);
    if (!sceneNodeOpt) {
        return;
    }
    if (!path.empty()) {
        YAML::Node node = (*sceneNodeOpt)["overrides"];
        if (!node) {
            node = (*sceneNodeOpt)["overrides"] = YAML::Node();
        }
        for (const std::string_view& name : path) {
            if (!node[name]) {
                node[name] = YAML::Node();
            }
            node.reset(node[name]);
        }
        node[fieldName] = val;
        return;
    }
    YAML::Node& sceneNode = *sceneNodeOpt;
    sceneNode["overrides"][fieldName] = val;
}

// TODO: create UpdateSceneNodeValue with upward search ?

#endif // EDITOR
