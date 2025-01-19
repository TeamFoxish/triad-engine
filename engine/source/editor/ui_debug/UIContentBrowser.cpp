#include "UIContentBrowser.h"

#include <d3d11.h>
#include <WICTextureLoader.h>
#include <fstream>

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "ImGuiFD.h"

#include "render/RenderSystem.h"
#include "render/Renderer.h"

#include "logs/Logs.h"

static const std::filesystem::path assets_dir = "assets";

static const std::filesystem::path components_dir = "assets\\components";
static const std::filesystem::path materials_dir = "assets\\materials";
static const std::filesystem::path scenes_dir = "assets\\scenes";
static const std::filesystem::path scripts_dir = "assets\\scripts";
static std::filesystem::path file_to_delete;

static bool openFileNameWnd = false;

const enum class assetsTypes
{
    None,
    Component,
    Script,
    Material
};
static assetsTypes a_type = assetsTypes::None;

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

    ImGui::Separator();

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
        std::string filenameString = relativePath.filename().generic_string();

        auto& texture = dirEntry.is_directory() ? directoryTexture : fileTexture;
        ImGui::ImageButton(filenameString.c_str(), (ImTextureID)texture, { thumbnailSize, thumbnailSize });

        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                file_to_delete = dirEntry;
                ImGui::OpenPopup("asset_popup");
            }
            else if (dirEntry.is_directory() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                curr_dir /= path.filename();
            }
        }
        else if (
            ImGui::IsMouseClicked(ImGuiMouseButton_Right)
            && !ImGui::IsPopupOpen("asset_popup")
            && ImGui::IsWindowHovered(ImGuiHoveredFlags_None)
            )
        {
            ImGui::OpenPopup("cont_brows_popup");
        }

        ImGui::TextWrapped(filenameString.c_str());
        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    DrawPopups();

    // File dialog window
    if (ImGuiFD::BeginDialog("Choose Asset")) {
        if (ImGuiFD::ActionDone()) {
            if (ImGuiFD::SelectionMade())
            {
                std::string filename = ImGuiFD::GetSelectionNameString(0);
                std::filesystem::path copy_to = curr_dir.string() + "\\" + ImGuiFD::GetSelectionNameString(0);
                TryChangeFileName(copy_to, filename);
                copy_to = curr_dir.string() + "\\" + filename;

                std::filesystem::copy_file(std::filesystem::path(ImGuiFD::GetSelectionPathString(0)), copy_to);
            }
            ImGuiFD::CloseCurrentDialog();
        }
        ImGuiFD::EndDialog();
    }

    ImGui::End();
}

ContentBrowser::~ContentBrowser()
{
    directoryTexture->Release();
    fileTexture->Release();
}

void ContentBrowser::CreateNewFile(const std::string& dirTo, const std::string& fileName, const std::string& expansion)
{
    std::string newAssetPath = dirTo + "\\" + fileName + expansion;
    std::ofstream ofs(newAssetPath);
    ofs.close();
}

void ContentBrowser::CreateComponent(const std::string& fileName)
{
    if (
        std::filesystem::exists(std::filesystem::path(components_dir.string() + "\\" + fileName + ".component"))
        || std::filesystem::exists(std::filesystem::path(components_dir.string() + "\\" + fileName + ".as"))
        || std::filesystem::exists(std::filesystem::path(components_dir.string() + "\\" + fileName + ".script"))
        )
    {
        return;
    }

    CreateNewFile(components_dir.string(), fileName, ".component");
    CreateNewFile(scripts_dir.string(), fileName, ".as");
    CreateNewFile(scripts_dir.string(), fileName, ".script");
}

void ContentBrowser::CreateScript(const std::string& fileName)
{
    if (
        std::filesystem::exists(std::filesystem::path(components_dir.string() + "\\" + fileName + ".as"))
        || std::filesystem::exists(std::filesystem::path(components_dir.string() + "\\" + fileName + ".script"))
        )
    {
        return;
    }

    CreateNewFile(scripts_dir.string(), fileName, ".as");
    CreateNewFile(scripts_dir.string(), fileName, ".script");
}

void ContentBrowser::CreateMaterial(const std::string& fileName)
{
    if (std::filesystem::exists(std::filesystem::path(components_dir.string() + "\\" + fileName + ".material")))
    {
        return;
    }

    CreateNewFile(materials_dir.string(), fileName, ".material");
}

void ContentBrowser::DrawPopups()
{
    // Content browser files manipulations
    static bool open_confirm_w = false;
    if (ImGui::BeginPopup("asset_popup"))
    {
        if (ImGui::MenuItem("Delete Item"))
        {
            open_confirm_w = true;
        }
        ImGui::EndPopup();
    }

    if (open_confirm_w)
    {
        ImGui::OpenPopup("confirm_popup");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("confirm_popup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you really want to delete\n%s?", file_to_delete.filename().string().c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            std::filesystem::remove_all(file_to_delete);
            file_to_delete.clear();
            open_confirm_w = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            open_confirm_w = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Content browser popup
    if (ImGui::BeginPopup("cont_brows_popup"))
    {
        if (ImGui::BeginMenu("Add Asset"))
        {
            if (ImGui::MenuItem("Create Component"))
            {
                a_type = assetsTypes::Component;
                openFileNameWnd = true;
            }

            if (ImGui::MenuItem("Create Script"))
            {
                a_type = assetsTypes::Script;
                openFileNameWnd = true;
            }

            if (ImGui::MenuItem("Create Material"))
            {
                a_type = assetsTypes::Material;
                openFileNameWnd = true;
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Import Asset"))
        {
            ImGuiFD::OpenDialog("Choose Asset", ImGuiFDMode_LoadFile, assets_dir.string().c_str(), "*.component,*.as,*.script");
        }
        ImGui::EndPopup();
    }

    // File name popup
    if (openFileNameWnd)
    {
        ImGui::OpenPopup("Create File");
    }

    //ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Create File", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static std::string fileName = "asset";

        ImGui::InputText("File Name", &fileName);

        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            // ToDo: check if name is correct
            switch (a_type)
            {
            case assetsTypes::Component:
                CreateComponent(fileName);
                break;

            case assetsTypes::Script:
                CreateScript(fileName);
                break;

            case assetsTypes::Material:
                CreateMaterial(fileName);
                break;

            case assetsTypes::None:
            default:
                break;
            }

            fileName = "asset";
            openFileNameWnd = false;
            a_type = assetsTypes::None;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            fileName = "asset";
            openFileNameWnd = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void ContentBrowser::TryChangeFileName(std::filesystem::path path, std::string& filename)
{
    static std::unordered_map<std::string, uint8_t> file_counter;

    if (std::filesystem::exists(path))
    {
        file_counter[filename]++;
        filename = std::format("{}_{}", filename, file_counter[filename]);
    }
}