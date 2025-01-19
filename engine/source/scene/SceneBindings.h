#pragma once

#include "scene/SceneTree.h"

void RegisterSceneBindings();

SceneTree::Handle GetEntityHandleFromScriptObject(const class asIScriptObject* obj);

asQWORD GetEntityIdFromHandle(SceneTree::Handle handle);
