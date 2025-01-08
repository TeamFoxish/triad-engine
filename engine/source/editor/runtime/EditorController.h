#pragma once

#ifdef EDITOR

#include <memory>

class InputDevice;

class EditorController {
public:
    class InputContextBase {
    public:
        enum class InputTarget {
            Game,
            Editor
        };

        virtual void ProceedInput(InputDevice* device) = 0;
        virtual InputTarget GetInputTarget() const = 0;
    };

    void ProceedInput();

    void SetInputContext(const std::shared_ptr<InputContextBase>& context);

    void ClearInputContext(const std::shared_ptr<InputContextBase>& context);

    InputContextBase::InputTarget GetInputTarget() const { return activeContext ? activeContext->GetInputTarget() : InputContextBase::InputTarget::Game; }

private:
    std::shared_ptr<InputContextBase> activeContext;
};

#endif // EDITOR
