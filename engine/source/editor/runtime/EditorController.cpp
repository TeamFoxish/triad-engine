#ifdef EDITOR

#include "EditorController.h"

#include "input/InputDevice.h"

void EditorController::ProceedInput()
{
    if (!activeContext) {
        return;
    }
    activeContext->ProceedInput(globalInputDevice);
}

void EditorController::SetInputContext(const std::shared_ptr<InputContextBase>& context)
{
    activeContext = context;
}

void EditorController::ClearInputContext(const std::shared_ptr<InputContextBase>& context)
{
    if (activeContext != context) {
        return;
    }
    activeContext.reset();
}

#endif // EDITOR
