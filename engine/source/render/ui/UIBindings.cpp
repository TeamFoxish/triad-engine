#include "UIBindings.h"

#include "scripts/ScriptSystem.h"

#include <string>
#include <locale>
#include <codecvt>

CUIElement::CUIElement()
{
    UIElement element;
    element.text = L"";
    element.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    element.position = { 0.5f, 0.5f };
    element.scale = { 1.0f, 1.0f };
    element.depth = 0.0f;

    handle = UIStorage::Instance().Add(element);
}

CUIElement::~CUIElement()
{
    UIStorage::Instance().Remove(handle);
}

const UIElement &CUIElement::GetElement() const
{
    return UIStorage::Instance().Get(handle);
}

UIElement &CUIElement::GetElement()
{
    return UIStorage::Instance().Get(handle);
}

// TODO move to StringUtils (linkage error)
std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

std::wstring s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

std::string &CUIElement::GetText()
{
    std::string sstring = ws2s(GetElement().text);
    return sstring;
}

void CUIElement::SetText(const std::string &text)
{
    GetElement().text = s2ws(text);
}

void CUIElement::ApplyOverrides(const YAML::Node &overrides)
{
	if (const YAML::Node& textVal = overrides["text"]) {
        SetText(textVal.Scalar());
	}
	if (const YAML::Node& posVal = overrides["postition"]) {
        Math::Vector3 position;
		position.x = posVal["x"] ? posVal["x"].as<float>() : position.x;
		position.y = posVal["y"] ? posVal["y"].as<float>() : position.y;
        position.z = 0.0f;
		SetPosition(position);
	}
    if (const YAML::Node& scaleVal = overrides["scale"]) {
        Math::Vector3 scale;
		scale.x = scaleVal["x"] ? scaleVal["x"].as<float>() : scale.x;
		scale.y = scaleVal["y"] ? scaleVal["y"].as<float>() : scale.y;
        scale.z = 0.0f;
		SetPosition(scale);
	}
    if (const YAML::Node& colorVal = overrides["color"]) {
        Math::Vector3 color;
		color.x = colorVal["r"] ? colorVal["r"].as<float>() : color.x;
		color.y = colorVal["g"] ? colorVal["g"].as<float>() : color.y;
        color.z = colorVal["b"] ? colorVal["b"].as<float>() : color.z;
		SetColor(color);
	}
	if (const YAML::Node& depthVal = overrides["depth"]) {
        SetDepth(depthVal.as<float>());
	}
}

CUIElement* UIElementFactory() {
    return new CUIElement();
}

CUIElement* CUIElementHandleAssign(const CUIElement* other, CUIElement* self)
{
	self->GetElement() = other->GetElement();
	return self;
}

#ifdef EDITOR
std::string UIElementToString(void *obj, int expandMembersLevel, CDebugger* dbg) {
	CUIElement* elementHandle = static_cast<CUIElement*>(obj);
    std::string text = elementHandle->GetText();
	DirectX::SimpleMath::Vector3 position = elementHandle->GetPosition();
	DirectX::SimpleMath::Vector3 scale = elementHandle->GetScale();
	DirectX::SimpleMath::Vector3 color = elementHandle->GetColor();
    float depth = elementHandle->GetDepth();
	return std::format("Text: {} Position: {}, {} Scale: {}, {} Color: {}, {}, {} Depth {}", 
                        text,
						position.x, position.y,
						scale.x, scale.y,
						color.x, color.y, color.z,
                        depth);
}
#endif // EDITOR

void RegisterUI() {
    asIScriptEngine* engine = gScriptSys->GetRawEngine();

    int r = engine->RegisterObjectType("UIElement", 0, asOBJ_REF); assert(r > 0);
    CNativeObject::RecognizeNativeType(engine->GetTypeInfoById(r));
    r = engine->RegisterObjectBehaviour("UIElement", asBEHAVE_FACTORY, "UIElement@ f()", asFUNCTION(UIElementFactory), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterObjectBehaviour("UIElement", asBEHAVE_ADDREF, "void f()", asMETHOD(CUIElement, AddRef), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectBehaviour("UIElement", asBEHAVE_RELEASE, "void f()", asMETHOD(CUIElement, Release), asCALL_THISCALL); assert(r >= 0);

	r = engine->RegisterObjectMethod("UIElement", "UIElement@+ opAssign(const UIElement@+)", asFUNCTION(CUIElementHandleAssign), asCALL_CDECL_OBJLAST); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "string GetText() const", asMETHOD(CUIElement, GetText), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "void SetText(const string &in)", asMETHOD(CUIElement, SetText), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "Math::Vector3 GetPosition() const", asMETHOD(CUIElement, GetPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "void SetPosition(const Math::Vector3 &in)", asMETHOD(CUIElement, SetPosition), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "Math::Vector3 GetScale() const", asMETHOD(CUIElement, GetScale), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "void SetScale(const Math::Vector3 &in)", asMETHOD(CUIElement, SetScale), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "Math::Vector3 GetColor() const", asMETHOD(CUIElement, GetColor), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "void SetColor(const Math::Vector3 &in)", asMETHOD(CUIElement, SetColor), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "float GetDepth() const", asMETHOD(CUIElement, GetDepth), asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("UIElement", "void SetDepth(const float &in)", asMETHOD(CUIElement, SetDepth), asCALL_THISCALL); assert(r >= 0);

#ifdef EDITOR
	CDebugger* debugger = gScriptSys->GetDebugger();

	asITypeInfo* uiElementType = engine->GetTypeInfoByDecl("UIElement");

	debugger->RegisterToStringCallback(uiElementType, UIElementToString);
#endif // EDITOR
}