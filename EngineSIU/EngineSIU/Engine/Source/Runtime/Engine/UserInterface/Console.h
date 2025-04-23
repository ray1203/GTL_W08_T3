#pragma once
#include "Container/Array.h"
#include "D3D11RHI/GraphicDevice.h"
#include "HAL/PlatformType.h"
#include "ImGUI/imgui.h"
#include "PropertyEditor/IWindowToggleable.h"

#define UE_LOG FConsole::GetInstance().AddLog


enum class ELogLevel : uint8
{
    Display,
    Warning,
    Error
};


class FStatOverlay
{
public:
    bool bShowFps = false;
    bool bShowMemory = false;
    bool bShowRender = false;

    void ToggleStat(const std::string& Command);
    void Render(ID3D11DeviceContext* Context, UINT InWidth, UINT InHeight) const;

private:
};

class FConsole : public IWindowToggleable
{
private:
    FConsole() = default;

public:
    static FConsole& GetInstance(); // 참조 반환으로 변경

    void Clear();
    void AddLog(ELogLevel Level, const char* Fmt, ...);
    // void AddLog(ELogLevel Level, const char* fmt, va_list args);
    void Draw();
    void ExecuteCommand(const std::string& Command);
    void OnResize(HWND hWnd);

    virtual void Toggle() override
    {
        if (bWasOpen)
        {
            bWasOpen = false;
        }
        else
        {
            bWasOpen = true;
        }
    }

public:
    struct FLogEntry
    {
        ELogLevel Level;
        FString Message;
    };

    TArray<FLogEntry> Items;
    char InputBuf[256] = "";
    bool bScrollToBottom = false;

    ImGuiTextFilter Filter; // 필터링을 위한 ImGuiTextFilter

    // 추가된 멤버 변수들
    bool bShowLogTemp = true; // LogTemp 체크박스
    bool bShowWarning = true; // Warning 체크박스
    bool bShowError = true;   // Error 체크박스

    bool bWasOpen = true;
    bool bShowFps = false;
    bool bShowMemory = false;

    FStatOverlay Overlay;

private:
    bool bExpand = true;
    UINT Width;
    UINT Height;
};
