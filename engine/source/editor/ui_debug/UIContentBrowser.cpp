#include "UIContentBrowser.h"

#include <d3d11.h>
#include <WICTextureLoader.h>

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include "render/RenderSystem.h"
#include "render/Renderer.h"


static const std::filesystem::path assets_dir = "assets";

void ContentBrowser::Init()
{
    curr_dir = assets_dir;

    std::string icon_path = "icons\\directory.png";
    HRESULT hr = DirectX::CreateWICTextureFromFile(gRenderSys->GetRenderer()->GetDevice(), gRenderSys->GetRenderer()->GetDeviceContext(), std::wstring(icon_path.begin(), icon_path.end()).c_str(), nullptr, &directoryTexture);

    icon_path = "icons\\file.png";
    hr = DirectX::CreateWICTextureFromFile(gRenderSys->GetRenderer()->GetDevice(), gRenderSys->GetRenderer()->GetDeviceContext(), std::wstring(icon_path.begin(), icon_path.end()).c_str(), nullptr, &fileTexture);
}

void ContentBrowser::Draw()
{
    ImGui::Begin("Content Browser");



    ImGui::Text("%s", curr_dir.relative_path().string().c_str());

    if (curr_dir != assets_dir)
    {
        ImGui::SameLine();
        if (ImGui::Button("Back"))
        {
            curr_dir = curr_dir.parent_path();
        }
    }

    ImGui::SeparatorText("");

    static float padding = 16.f;
    static float thumbnailSize = 75.f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1)
        columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    for (auto& dirEntry : std::filesystem::directory_iterator(curr_dir))
    {
        const auto& path = dirEntry.path();
        auto relativePath = std::filesystem::relative(path, assets_dir);
        std::string filenameString = relativePath.filename().string(); // ToDo: replace std::string or else it will crash on Russian names or other languages

        if (dirEntry.is_directory())
            ImGui::ImageButton(filenameString.c_str(), (ImTextureID)directoryTexture, { thumbnailSize, thumbnailSize });
        else
            ImGui::ImageButton(filenameString.c_str(), (ImTextureID)fileTexture, { thumbnailSize, thumbnailSize });

        if (dirEntry.is_directory() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            curr_dir /= path.filename();
        }

        ImGui::TextWrapped(filenameString.c_str());
        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    ImGui::End();
}

ContentBrowser::~ContentBrowser()
{
    directoryTexture->Release();
    fileTexture->Release();
}
