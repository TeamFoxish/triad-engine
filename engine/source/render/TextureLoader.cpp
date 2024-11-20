#include "TextureLoader.h"

#include "Texture.h"
#include "RenderSystem.h"
#include "RenderContext.h"
#include "RenderResources.h"
#include "file/FileIO.h"

#include <WICTextureLoader.h>
#include <wrl.h>

TextureLoader::TextureLoader()
{
    FACTORY_INIT;
}

void TextureLoader::Load(ResTag tag, const YAML::Node& desc)
{
    Triad::FileIO::FPath path;
    if (!Triad::Resource::ResolveFileTagToFile(tag, desc["file"], path)) {
        return;
    }
    ID3D11ShaderResourceView* view;
    if (!LoadTexture(path, &view)) {
        return;
    }
    RenderResources::Instance().textures.Add(tag, new Texture(0, view));
}

void TextureLoader::Unload(ResTag tag)
{
    delete(RenderResources::Instance().textures.Get(tag));
    RenderResources::Instance().textures.Remove(tag);
}

bool TextureLoader::LoadTexture(const std::wstring& path, ID3D11ShaderResourceView** res)
{
    using namespace DirectX;
    using namespace Microsoft::WRL;

    ID3D11ShaderResourceView* srv;
    RenderContext& ctx = gRenderSys->GetContext();
    HRESULT hr = CreateWICTextureFromFile(ctx.device, ctx.context, path.c_str(), nullptr, &srv);
    if (FAILED(hr)) {
        return false;
    }
    *res = srv;

    return true;
}
