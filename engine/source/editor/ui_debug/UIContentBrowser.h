#pragma once

#include <string>
#include <filesystem>

struct ID3D11ShaderResourceView;

class AssetManager;

class ContentBrowser
{
public:
    void Init(AssetManager* assetManager);
    void Draw();

    ~ContentBrowser();

private:
    void CreateNewFile(const std::string& dirTo, const std::string& fileName, const std::string& expansion);

    void CreateComponent(const std::string& fileName);
    void CreateScript(const std::string& fileName);
    void CreateMaterial(const std::string& fileName);

    void DrawPopups();
    void TryChangeFileName(std::filesystem::path path, std::string& filename);

private:
    std::filesystem::path curr_dir;

    ID3D11ShaderResourceView* directoryTexture = nullptr;
    ID3D11ShaderResourceView* fileTexture = nullptr;

    AssetManager* assetManager = nullptr;
};