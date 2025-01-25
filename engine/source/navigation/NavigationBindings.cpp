#include "NavigationBindings.h"

#include "scripts/ScriptSystem.h"
#include "shared/ResourceHandle.h"
#include "shared/MathScriptBindings.h"

#include "NavMeshResources.h"
#include "NavMeshSystem.h"

#include "logs/Logs.h"

CScriptArray* FindPath(const CResourceHandle& agentHandle, const Math::Vector3& from, const Math::Vector3& to)
{
	assert(Math::typeIdVector3 >= 0);
	asITypeInfo* vecTypeInfo = gScriptSys->GetRawEngine()->GetTypeInfoByDecl("array<Math::Vector3>");
	CScriptArray* path = CScriptArray::Create(vecTypeInfo);
	if (!path) {
		// log error
		return nullptr;
	}
	const auto iter = NavMeshResources::Instance().agents.find(agentHandle.GetTag());
	if (iter == NavMeshResources::Instance().agents.end()) {
		LOG_ERROR("failed to find path for agent with tag '{}'. it wasn't loaded", agentHandle.GetTag().string());
		return path;
	}
	const NavMeshAgent& agent = iter->second;
	const std::vector<float> fltPath = gNavigation->FindPath(agent, &from.x, &to.x);
	assert(fltPath.size() % 3 == 0);
	const int vertsNum = fltPath.size() / 3;
	path->Reserve(vertsNum);
	for (int i = 0; i < fltPath.size(); i += 3) {
		Math::Vector3 v(fltPath[i], fltPath[i + 1], fltPath[i + 2]);
		path->InsertLast(&v);
	}
	return path;
}

void RegisterNavigationBindings()
{
	auto engine = gScriptSys->GetRawEngine();
	int r;

	r = engine->SetDefaultNamespace("Navigation"); assert(r >= 0);

	r = engine->RegisterGlobalFunction("array<Math::Vector3>@ FindPath(const ResourceHandle &in agent, const Math::Vector3 &in from, const Math::Vector3 &in to)", asFUNCTION(FindPath), asCALL_CDECL); assert(r >= 0);

	r = engine->SetDefaultNamespace(""); assert(r >= 0);
}
