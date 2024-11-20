#pragma once

#include "resource/ResourceLoader.h"
#include "misc/Factory.h"

#include <string>

class Renderer;
struct ID3D11ShaderResourceView;

class TextureLoader : public ResourceLoader, RegisteredInFactory<ResourceLoader, TextureLoader> {
public:
	TextureLoader();

	void Load(ResTag tag, const YAML::Node& desc) override;
	void Unload(ResTag tag) override;

	static std::unique_ptr<ResourceLoader> CreateInstance()
	{
		return std::make_unique<TextureLoader>();
	}

	static const char* GetFactoryKey()
	{
		return "texture";
	}

	static bool LoadTexture(const std::wstring& path, ID3D11ShaderResourceView** res);
};
