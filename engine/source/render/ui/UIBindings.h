#pragma once

#include "UIStorage.h"
#include "UIElement.h"
#include "shared/Shared.h"
#include "shared/MathScriptBindings.h"
#include <string>

class CUIElement : public CRef<CUIElement>, public CNativeObject {
public:
	CUIElement();
	~CUIElement();

	UIStorage::Handle GetHandle() const { return handle; }
	const UIElement& GetElement() const;
    UIElement& GetElement();

    std::string& GetText();

    void SetText(const std::string& text);

	Math::Vector3 GetPosition() const {
        Math::Vector2 pos = GetElement().position;
        return Math::Vector3(pos.x, pos.y, 0.f); 
    }
	void SetPosition(Math::Vector3 pos) {
        Math::Vector2 position = GetElement().position;
        position.x = pos.x;
        position.y = pos.y;
    }

	Math::Vector3 GetScale() const {
        Math::Vector2 scale = GetElement().scale;
        return Math::Vector3(scale.x, scale.y, 0.f);  
    }

	void SetScale(Math::Vector3 scale) {
        Math::Vector2 scl = GetElement().scale;
        scl.x = scale.x;
        scl.y = scale.y;
    }

	Math::Vector3 GetColor() const {
        Math::Vector4 color = GetElement().color;
        return Math::Vector3(color.x, color.y, color.z);  
    }
	void SetColor(Math::Vector3 color) {
        Math::Vector4 clr = GetElement().color;
        clr.x = color.x;
        clr.y = color.y;
        clr.z = color.z;
        clr.w = 0.0f;
    }

	float GetDepth() const { return GetElement().depth; }
	void SetDepth(float depth) { GetElement().depth = depth; }

	void ApplyOverrides(const YAML::Node& overrides) override;

protected:
	UIStorage::Handle handle;
};

void RegisterUI();