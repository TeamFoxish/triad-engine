#include "EditorController.h"

#include "editor/ui_debug/UIDebug.h"
#include "game/GameBindings.h"
#include "input/InputDevice.h"

void EditorController::ProceedInput()
{
    if (globalInputDevice->IsKeyDown(Keys::Delete)) {
        SceneTree::Handle selectedEnt = UIDebug::GetOutliner().GetSelectedNode();
        if (selectedEnt.id_ == 0) {
            return; // TEMP. do not destroy scene root
        }
        if (selectedEnt.id_ >= 0 && gSceneTree->IsValidHandle(selectedEnt)) {
            UIDebug::GetOutliner().ClearSelectedNode();
            SceneTree::Entity& entity = gSceneTree->Get(selectedEnt);
            assert(entity.obj.GetRaw()); // check if entity has a valid script object attached
            GameBindings::DestroyComponent(entity.obj);
        }
    }
}
