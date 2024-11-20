#include "Texture.h"

#include "Renderer.h"

#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

Texture::Texture(uint32_t slot, ID3D11ShaderResourceView* srv)
	: slot(slot)
	, texView(srv)
{
}

void Texture::Activate(ID3D11DeviceContext* context)
{
	context->PSSetShaderResources(slot, 1, &texView);
}
