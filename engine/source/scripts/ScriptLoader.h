#pragma once

#include "angelscript.h"
#include "resource/ResourceLoader.h"
#include "misc/Factory.h"
#include "scriptbuilder.h"
#include <unordered_map>

class ScriptLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, ScriptLoader> {
public:
    ScriptLoader();
	~ScriptLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void LoadAll(const std::string& scriptDirPath);
	void Unload(ResTag tag) override {};
	void Build();
	CScriptBuilder* GetBuilderByModule(const std::string& module);

	static std::vector<std::string> GetPropertyMetadata(int holderTypeId, asUINT fieldIdx);

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<ScriptLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "script";
	}

private:
	static inline std::unordered_map<ResTag, const YAML::Node> _scripts;
	static inline std::unordered_map<std::string, CScriptBuilder*> _loadedModules;
};