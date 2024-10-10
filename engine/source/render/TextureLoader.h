#pragma once

#include <string>

class Renderer;
struct ID3D11ShaderResourceView;

class TextureLoader {
public:
	static bool LoadTexture(const std::wstring& path, Renderer* renderer, ID3D11ShaderResourceView** res);
};
