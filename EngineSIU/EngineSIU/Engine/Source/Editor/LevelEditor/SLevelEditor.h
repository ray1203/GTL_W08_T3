#pragma once
#include <sstream>
#include "Define.h"
#include "Container/Map.h"
#include "Delegates/Delegate.h"

class SSplitterH;
class SSplitterV;
class UWorld;
class FEditorViewportClient;


class SLevelEditor
{
public:
    SLevelEditor();

    void Initialize(uint32 InEditorWidth, uint32 InEditorHeight);
    void Tick(float DeltaTime);
    void Release();

    void ResizeEditor(uint32 InEditorWidth, uint32 InEditorHeight);
    void SelectViewport(const FVector2D& Point);

    void ResizeViewports();
    void SetEnableMultiViewport(bool bIsEnable);
    bool IsMultiViewport() const;

private:
    SSplitterH* HSplitter;
    SSplitterV* VSplitter;
    
    std::shared_ptr<FEditorViewportClient> ViewportClients[4];
    std::shared_ptr<FEditorViewportClient> ActiveViewportClient;

private:
    // Input 관련 변수들

    /** 우클릭 시 캡처된 마우스 커서의 초기 위치 (스크린 좌표계) */
    FVector2D MousePinPosition;

    /** 좌클릭시 커서와 선택된 Actor와의 거리 차 */
    FVector TargetDiff;

    /** 현재 복제중인지? */
    bool bIsDuplicating = false;

private:
    bool bMultiViewportMode;
    
    uint32 EditorWidth;
    uint32 EditorHeight;

public:
    std::shared_ptr<FEditorViewportClient>* GetViewports() { return ViewportClients; }
    std::shared_ptr<FEditorViewportClient> GetActiveViewportClient() const
    {
        return ActiveViewportClient;
    }
    void SetActiveViewportClient(const std::shared_ptr<FEditorViewportClient>& InViewportClient)
    {
        ActiveViewportClient = InViewportClient;
    }
    void SetActiveViewportClient(int Index)
    {
        ActiveViewportClient = ViewportClients[Index];
    }

    //Save And Load
private:
    const FString IniFilePath = "editor.ini";

public:
    void LoadConfig();
    void SaveConfig();

private:
    TArray<FDelegateHandle> InputDelegateHandles;

public:
    // 에디터 모드 입장 시
    void BindEditorInput();
    // 에디터 모드 나갈 때
    void UnbindEditorInput();

private:
    TMap<FString, FString> ReadIniFile(const FString& FilePath);
    void WriteIniFile(const FString& FilePath, const TMap<FString, FString>& Config);

    template <typename T>
    T GetValueFromConfig(const TMap<FString, FString>& Config, const FString& Key, T DefaultValue) {
        if (const FString* Value = Config.Find(Key))
        {
            std::istringstream iss(**Value);
            T ConfigValue;
            if (iss >> ConfigValue)
            {
                return ConfigValue;
            }
        }
        return DefaultValue;
    }
};
