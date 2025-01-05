#pragma once

#include <string>
#include <filesystem>

struct ID3D11ShaderResourceView;

class ContentBrowser
{
public:
    void Init();
    void Draw();

    ~ContentBrowser();

private:
    std::filesystem::path curr_dir;

    ID3D11ShaderResourceView* directoryTexture = nullptr;
    ID3D11ShaderResourceView* fileTexture = nullptr;

};