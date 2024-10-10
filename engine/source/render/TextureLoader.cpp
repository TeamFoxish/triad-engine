#include "TextureLoader.h"

#include "Renderer.h"

#include <WICTextureLoader.h>
#include <wrl.h>

bool TextureLoader::LoadTexture(const std::wstring& path, Renderer* renderer, ID3D11ShaderResourceView** res)
{
    using namespace DirectX;
    using namespace Microsoft::WRL;

    ID3D11ShaderResourceView* srv;
    HRESULT hr = CreateWICTextureFromFile(renderer->GetDevice(), renderer->GetDeviceContext(),
        path.c_str(), nullptr, &srv);
    if (FAILED(hr)) {
        return false;
    }
    *res = srv;

    return true;
}
