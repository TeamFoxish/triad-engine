#pragma once

class asIScriptFunction;
class ScriptObject;

namespace GameBindings {
    void SetDestroyComponentCB(asIScriptFunction* destroyCompCb);

    void DestroyComponent(const ScriptObject& comp);
}

void RegisterGameBindings();
