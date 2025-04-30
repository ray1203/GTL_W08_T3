#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"
#include "LightGridGenerator.h"

struct ImFont;
struct ImVec2;

class ControlEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void CreateMenuButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateModifyButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateFlagButton() const;
    void CreatePIEButton(ImVec2 ButtonSize, ImFont* IconFont) const;
    void CreateSRTButton(ImVec2 ButtonSize) const;
    void CreateLightSpawnButton(ImVec2 ButtonSize, ImFont* IconFont);
    void CreateCurveEditorButton(ImVec2 ButtonSize, ImFont* IconFont);

    uint64 ConvertSelectionToFlags(const bool selected[]) const;
    
private:
    float Width = 300, Height = 100;
    bool bOpenMenu = false;

    float* FOV = nullptr;
    float CameraSpeed = 0.0f;
    float GridScale = 1.0f;
    FLightGridGenerator LightGridGenerator;
};

