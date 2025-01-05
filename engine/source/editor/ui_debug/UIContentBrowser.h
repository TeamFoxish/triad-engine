#pragma once

#include <string>
#include <filesystem>


class ContentBrowser
{
public:
    void Init();
    void Draw();

private:
    std::filesystem::path curr_dir;
};