#pragma once

#include "scene/SceneTree.h"

union EntityId {
    SceneTree::Handle handle = {};
    asQWORD id;
};

void RegisterSceneBindings();

SceneTree::Handle GetEntityHandleFromScriptObject(const class asIScriptObject* obj);

asQWORD GetEntityIdFromHandle(SceneTree::Handle handle);
