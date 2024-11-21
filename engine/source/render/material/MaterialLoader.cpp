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

template<typename Prop_T>
static void ApplyOverrides(std::vector<Prop_T>& target, const std::vector<Prop_T>& overrides) 
{
	for (const Prop_T& prop : overrides) {
		auto iter = std::find_if(target.begin(), target.end(),
			[&prop](const Prop_T& other) {
				return other.key == prop.key; // find prop in material with matching key
			});
		if (iter == target.end()) {
			// TODO: log error
			continue;
		}
		if (iter->type != prop.type) {
			// TODO: log error
			continue;
		}
		*iter = prop; // apply override
	}
}

template<typename Prop_T>
static void ApplyOverrides(const std::shared_ptr<Material>& parent, std::vector<Prop_T>& target, const std::vector<Prop_T>& overrides) 
{
	if (!parent) {
		target = overrides;
		return;
	}
	ApplyOverrides(target, overrides);
}

std::shared_ptr<Material> MaterialLoader::CreateMaterial(const YAML::Node& desc)
{
	const YAML::Node& parentMaterial = desc["parent"];
	std::shared_ptr<Material> prototype = CreateChild(parentMaterial);
	if (!prototype) {
		prototype = std::make_shared<Material>();
	}

	ApplyOverrides(prototype->parent, prototype->properties, ParseProperties(desc["properties"]));
	ApplyOverrides(prototype->parent, prototype->textures, ParseTextures(desc["textures"]));

	return prototype;
}

std::shared_ptr<Material> MaterialLoader::CreateChild(const YAML::Node& parentTagNode)
{
	using RetT = std::shared_ptr<Material>;
	if (!parentTagNode) {
		return RetT{}; // parent was unset
	}
	if (!parentTagNode.IsScalar()) {
		// TODO: log error
		return RetT{};
	}
	const std::string& parentTag = parentTagNode.Scalar();
	if (!Triad::Resource::IsTag(parentTag)) {
		// TODO: log error
		return RetT{};
	}
	const std::shared_ptr<Material> parent = 
		RenderResources::Instance().materials.Get(ToStrid(parentTag));
	return Material::CreateChild(parent);
}

Material::PropList MaterialLoader::ParseProperties(const YAML::Node& properties)
{
	Material::PropList props;
	if (!properties) {
		return props;
	}
	
	for (const auto& propEntry : properties) {
		Material::Prop prop;
		{
			const YAML::Node val = propEntry["key"];
			if (!val || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			prop.key = ToStrid(val.Scalar());
		}
		{
			const YAML::Node val = propEntry["type"];
			if (!val || !val.IsScalar()) {
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
			if (!val) {
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
	if (!textures) {
		return res;
	}

	for (const auto& texEntry : textures) {
		Material::TextureProp prop;
		{
			const YAML::Node val = texEntry["key"];
			if (!val || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			prop.key = ToStrid(val.Scalar());
		}
		{
			const YAML::Node val = texEntry["tag"];
			if (!val || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			if (Triad::Resource::IsTag(val.Scalar())) {
				ResTag tag = ToStrid(val.Scalar());
				prop.tex = RenderResources::Instance().textures.Get(tag);
			}
		}
		{
			const YAML::Node val = texEntry["slot"];
			if (!val || !val.IsScalar()) {
				// TODO: log error
				continue;
			}
			prop.slot = val.as<uint32_t>();
		}
		res.push_back(prop);
	}

	return res;
}
