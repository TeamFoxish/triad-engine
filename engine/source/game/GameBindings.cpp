#include "GameBindings.h"

#include "scripts/ScriptSystem.h"
#include "shared/ResourceHandle.h"

#include "scene/SceneLoader.h"
#include "logs/Logs.h"

#include <scripthandle.h>

CScriptHandle CreateScene(const CResourceHandle& sceneRes) 
{
    CScriptHandle ref;
    if (!sceneRes.IsValid()) {
        LOG_ERROR("failed to load scene. scene reference was invalid");
        return ref;
    }
    std::unique_ptr<ScriptObject> sceneInst = 
        SceneLoader::CreateScene(sceneRes.GetTag());
    if (sceneInst) {
        ref.Set(sceneInst->GetRaw(), sceneInst->GetTypeInfo());
    }
    return ref;
}

void RegisterGameBindings()
{
    auto engine = gScriptSys->GetRawEngine();
    int r;
    
    engine->SetDefaultNamespace("Game::Impl");

    r = engine->RegisterGlobalFunction("ref@ CreateScene(const ResourceHandle &in scene)", asFUNCTION(CreateScene), asCALL_CDECL); assert(r >= 0);

    engine->SetDefaultNamespace("");
}
