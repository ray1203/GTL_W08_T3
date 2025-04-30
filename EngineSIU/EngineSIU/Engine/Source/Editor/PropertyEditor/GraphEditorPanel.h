#pragma once
#include "GameFramework/Actor.h"
#include "ImGUI/imgui.h"
#include "UnrealEd/EditorPanel.h"

// 카메라 쉐이크용 그래프 에디터 패널
class GraphEditorPanel : public UEditorPanel
{
public:
    GraphEditorPanel();

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    float Width = 0, Height = 0;

    static constexpr int MaxPoints = 10;
    ImVec2 CurvePoints[MaxPoints] = {};
    int SelectedPoint = -1;
};
