#pragma once

#include <string>

class Renderer;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

class Texture
{
public:
	// TODO: remove default constructor
	Texture() = default;
	Texture(uint32_t slot, std::wstring path, Renderer* renderer);
	Texture(uint32_t slot, ID3D11ShaderResourceView* srv);

	void Activate(ID3D11DeviceContext* context);

	bool IsValid() const { return texView != nullptr; }

protected:
	ID3D11ShaderResourceView* texView = nullptr;
	uint32_t slot = 0;
};
