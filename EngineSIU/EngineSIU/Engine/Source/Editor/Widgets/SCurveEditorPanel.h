// SCurveEditorPanel.h
#pragma once

#include "UnrealEd/EditorPanel.h"
#include "Math/FCurveData.h"

class SCurveEditorPanel : public UEditorPanel
{
public:
    SCurveEditorPanel();

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    void LoadFromCSV(const FString& FilePath);
    void SaveToCSV(const FString& FilePath) const;
    bool ImportSingleCurveFromCSV(const FString& FilePath, const FString& TargetLabel);

private:
    float Width = 0.0f;
    float Height = 0.0f;

    static constexpr int MaxCurveCount = 6; // 예: X, Y, Z, Pitch, Yaw, Roll
    static constexpr int MaxPoints = 16;

    FString CurveLabels[MaxCurveCount] = { TEXT("X"), TEXT("Y"), TEXT("Z"), TEXT("Pitch"), TEXT("Yaw"), TEXT("Roll") };

    FCurveData Curves[MaxCurveCount];

    int32 CurrentTabIndex = 0;
};
