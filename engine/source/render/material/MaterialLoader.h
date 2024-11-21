#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"
#include "Material.h"

#include <memory>

class MaterialLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, MaterialLoader> {
public:
	MaterialLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override;

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<MaterialLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "material";
	}

private:
	static std::shared_ptr<Material> CreateMaterial(const YAML::Node& desc);

	static std::shared_ptr<Material> CreateChild(const YAML::Node& parentTagNode);

	static Material::PropList ParseProperties(const YAML::Node& properties);

	using PropType = Material::Prop::Type;
	static bool ParsePropValue(const YAML::Node& value, PropType type, Material::Prop::ValT& out);

	static Material::TexturePropList ParseTextures(const YAML::Node& textures);
};
