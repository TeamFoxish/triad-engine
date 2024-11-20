#include "MaterialLoader.h"

#include "render/RenderResources.h"

MaterialLoader::MaterialLoader()
{
	FACTORY_INIT;
}

void MaterialLoader::Load(ResTag tag, const YAML::Node& desc)
{
	RenderResources::Instance().materials.Add(tag, CreateMaterial(desc));
}

void MaterialLoader::Unload(ResTag tag)
{
	RenderResources::Instance().materials.Remove(tag);
}

std::shared_ptr<Material> MaterialLoader::CreateMaterial(const YAML::Node& desc)
{
	std::shared_ptr<Material> res = std::make_shared<Material>();
	// TODO: get parent material and override values
	res->properties = ParseProperties(desc["properties"]);
	res->textures = ParseTextures(desc["textures"]);
	return res;
}

Material::PropList MaterialLoader::ParseProperties(const YAML::Node& properties)
{
	Material::PropList props;
	if (!properties.IsDefined()) {
		return props;
	}
	
	for (const auto& propEntry : properties) {
		Material::Prop prop;
		{
			const YAML::Node val = propEntry["key"];
			if (!val.IsDefined() || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			prop.key = ToStrid(val.Scalar());
		}
		{
			const YAML::Node val = propEntry["type"];
			if (!val.IsDefined() || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			prop.type = Material::Prop::ParseType(ToStrid(val.Scalar()));
			if (prop.type == Material::Prop::Invalid) {
				// TODO: log error
				continue;
			}
		}
		{
			const YAML::Node val = propEntry["value"];
			if (!val.IsDefined()) {
				// TODO: log error
				continue;
			}
			if (!ParsePropValue(val, prop.type, prop.val)) {
				// TODO: log error
				continue;
			}
		}
		props.push_back(prop);
	}

	return props;
}

bool MaterialLoader::ParsePropValue(const YAML::Node& value, PropType type, Material::Prop::ValT& out)
{
	switch (type) {
		case PropType::Integer:
			out = value.as<int>();
			break;
		case PropType::Float:
			out = value.as<float>();
			break;
		case PropType::Vector4:
		{
			if (!value.IsSequence()) {
				return false;
			}
			assert(value.size() == 4);
			out = Math::Vector4 {
				value[0].as<float>(), 
				value[1].as<float>(), 
				value[2].as<float>(), 
				value[3].as<float>() 
			};
			break;
		}
		default:
			assert(false);
	}
	return true;
}

Material::TexturePropList MaterialLoader::ParseTextures(const YAML::Node& textures)
{
	Material::TexturePropList res;
	if (!textures.IsDefined()) {
		return res;
	}

	for (const auto& texEntry : textures) {
		Material::TextureProp prop;
		{
			const YAML::Node val = texEntry["key"];
			if (!val.IsDefined() || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			prop.key = ToStrid(val.Scalar());
		}
		{
			const YAML::Node val = texEntry["tag"];
			if (!val.IsDefined() || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			if (!Triad::Resource::IsTag(val.Scalar())) {
				continue; // unset texture
			}
			ResTag tag = ToStrid(val.Scalar());
			prop.tex = RenderResources::Instance().textures.Get(tag);
		}
		{
			const YAML::Node val = texEntry["slot"];
			if (!val.IsDefined() || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			prop.slot = val.as<uint32_t>();
		}
		res.push_back(prop);
	}

	return res;
}
