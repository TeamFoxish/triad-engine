#pragma once

#include "SimpleMath.h"
#include <string>

struct UIElement {
    std::wstring text;
    DirectX::SimpleMath::Vector2 position;
    DirectX::SimpleMath::Vector2 scale;
    DirectX::SimpleMath::Vector4 color;
    float depth;
};