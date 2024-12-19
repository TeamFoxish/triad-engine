#include "InputBindings.h"

#include "scripts/ScriptSystem.h"
#include "InputDevice.h"

#include "runtime/RuntimeIface.h"

#define INPUT_KEY(NAME) #NAME, (int)Keys::NAME

static bool IsKeyDown(Keys key)
{
    return globalInputDevice->IsKeyDown(key);
}

static bool IsKeyHold(Keys key)
{
    return globalInputDevice->IsKeyHold(key);
}

static bool IsKeyUp(Keys key)
{
    return globalInputDevice->IsKeyUp(key);
}

static void GetMousePosition(float& x, float& y)
{
    Math::Vector2 pos = gEngineRuntime->GetMousePosInViewport();
    x = pos.x;
    y = pos.y;
}

static void GetMouseOffset(float& x, float& y)
{
    x = globalInputDevice->MouseOffset.x;
    y = globalInputDevice->MouseOffset.y;
}

void RegisterInputBindings()
{
    auto engine = gScriptSys->GetRawEngine();
    int r;

    engine->SetDefaultNamespace("Input");

    r = engine->RegisterEnum("Key"); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(None)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(LeftButton)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(RightButton)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(MiddleButton)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(MouseButtonX1)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(MouseButtonX2)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(MouseButtonX3)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(WheelUp)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(WheelDown)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Back)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Tab)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Enter)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Pause)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(CapsLock)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Kana)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Kanji)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Escape)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(ImeConvert)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(ImeNoConvert)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Space)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(PageUp)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(PageDown)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(End)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Home)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Left)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Up)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Right)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Down)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Select)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Print)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Execute)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(PrintScreen)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Insert)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Delete)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Help)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D0)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D1)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D2)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D3)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D4)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D5)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D6)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D7)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D8)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D9)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(A)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(B)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(C)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(D)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(E)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(G)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(H)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(I)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(J)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(K)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(L)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(M)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(N)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(O)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(P)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Q)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(R)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(S)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(T)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(U)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(V)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(W)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(X)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Y)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Z)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(LeftWindows)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(RightWindows)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Apps)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Sleep)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad0)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad1)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad2)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad3)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad4)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad5)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad6)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad7)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad8)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumPad9)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Multiply)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Add)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Separator)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Subtract)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Decimal)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Divide)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F1)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F2)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F3)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F4)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F5)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F6)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F7)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F8)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F9)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F10)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F11)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F12)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F13)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F14)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F15)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F16)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F17)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F18)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F19)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F20)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F21)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F22)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F23)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(F24)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(NumLock)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Scroll)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(LeftShift)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(RightShift)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(LeftControl)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(RightControl)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(LeftAlt)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(RightAlt)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(BrowserBack)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(BrowserForward)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(BrowserRefresh)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(BrowserStop)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(BrowserSearch)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(BrowserFavorites)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(BrowserHome)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(VolumeMute)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(VolumeDown)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(VolumeUp)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(MediaNextTrack)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(MediaPreviousTrack)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(MediaStop)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(MediaPlayPause)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(LaunchMail)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(SelectMedia)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(LaunchApplication1)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(LaunchApplication2)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemSemicolon)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemPlus)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemComma)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemMinus)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemPeriod)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemQuestion)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemTilde)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(ChatPadGreen)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(ChatPadOrange)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemOpenBrackets)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemPipe)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemCloseBrackets)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemQuotes)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Oem8)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemBackslash)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(ProcessKey)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemCopy)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemAuto)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemEnlW)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Attn)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Crsel)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Exsel)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(EraseEof)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Play)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Zoom)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(Pa1)); assert(r >= 0);
    r = engine->RegisterEnumValue("Key", INPUT_KEY(OemClear)); assert(r >= 0);

    r = engine->RegisterGlobalFunction("bool IsKeyDown(Key key)", asFUNCTION(IsKeyDown), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool IsKeyHold(Key key)", asFUNCTION(IsKeyHold), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("bool IsKeyUp(Key key)", asFUNCTION(IsKeyUp), asCALL_CDECL); assert(r >= 0);

    r = engine->RegisterGlobalFunction("void GetMousePosition(float &out x, float &out y)", asFUNCTION(GetMousePosition), asCALL_CDECL); assert(r >= 0);
    r = engine->RegisterGlobalFunction("void GetMouseOffset(float &out x, float &out y)", asFUNCTION(GetMouseOffset), asCALL_CDECL); assert(r >= 0);

    engine->SetDefaultNamespace("");
}
