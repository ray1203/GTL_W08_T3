// SCurveEditorPanel.cpp
#include "SCurveEditorPanel.h"

#include "ImCurveWidget.h"
#include "tinyfiledialogs.h"
#include "Curve/CurveManager.h"
#include "Math/MathUtility.h"
#include "Math/StringUtils.h"
#include "Math/Vector.h"
#include "Misc/FileHelper.h"


SCurveEditorPanel::SCurveEditorPanel()
{
    for (int i = 0; i < MaxCurveCount; ++i)
    {
        Curves[i].MaxPoints = MaxPoints;
        Curves[i].SelectedIndex = -1;
        Curves[i].ControlPoints.SetNum(MaxPoints);
        for (int j = 0; j < MaxPoints; ++j)
        {
            Curves[i].ControlPoints[j] = FVector2D(0.f, 0.f); // 직접 0 초기화
        }
        Curves[i].ControlPoints[0] = FVector2D(0.0f, 0.0f);
        Curves[i].ControlPoints[1] = FVector2D(1.0f, 0.0f);
        Curves[i].ControlPoints[2].X = ImGui::CurveTerminator;
    }
}

void SCurveEditorPanel::Render()
{
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin("Curve Editor");

    // === 상단 버튼 영역 ===
    if (ImGui::Button("Open CSV"))
    {
        const char* filter[] = { "*.csv" };
        const char* filePath = tinyfd_openFileDialog("Open Curve CSV", "", 1, filter, "CSV Files", 0);

        if (filePath)
        {
            LoadFromCSV(FString(filePath));
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Save CSV"))
    {
        const char* filter[] = { "*.csv" };
        const char* filePath = tinyfd_saveFileDialog("Save Curve CSV", "", 1, filter, "CSV Files");

        if (filePath)
        {
            SaveToCSV(FString(filePath));
        }
    }
    if (ImGui::Button("Import Current Tab"))
    {
        const char* filter[] = { "*.csv" };
        const char* filePath = tinyfd_openFileDialog("Select Curve CSV", "", 1, filter, "CSV Files", 0);

        if (filePath)
        {
            FString LabelToImport = CurveLabels[CurrentTabIndex];
            if (!ImportSingleCurveFromCSV(FString(filePath), LabelToImport))
            {
                UE_LOG(ELogLevel::Warning, TEXT("탭 %s 을 %s 에서 찾을 수 없습니다."), *LabelToImport, *FString(filePath));
            }
        }
    }

    ImGui::Separator();

    // 탭 선택 영역 추가
    ImGui::BeginChild("CurveTabs", ImVec2(100, 0), true);
    for (int i = 0; i < MaxCurveCount; ++i)
    {
        if (ImGui::Selectable(*CurveLabels[i], CurrentTabIndex == i))
        {
            CurrentTabIndex = i;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // 현재 선택된 커브 하나만 렌더링
    ImGui::BeginChild("CurveEditorContent", ImVec2(0, 0), false);
    ImGui::Text("%s", *CurveLabels[CurrentTabIndex]);
    ImGui::Curve(*CurveLabels[CurrentTabIndex], ImVec2(600, 150),
        MaxPoints, (ImVec2*)Curves[CurrentTabIndex].ControlPoints.GetData(),
        &Curves[CurrentTabIndex].SelectedIndex,
        ImVec2(0.0f, -1.0f), ImVec2(1.0f, 1.0f));
    ImGui::EndChild();


    ImGui::End();
}


void SCurveEditorPanel::OnResize(HWND hWnd)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    Width = clientRect.right - clientRect.left;
    Height = clientRect.bottom - clientRect.top;
}


void SCurveEditorPanel::LoadFromCSV(const FString& FilePath)
{
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
        return;

    TMap<FString, FCurveData> LoadedCurves;
    if (!CurveIO::ParseCurvesFromCSV(FileContent, LoadedCurves))
        return;

    int i = 0;
    for (const auto& Pair : LoadedCurves)
    {
        if (i >= MaxCurveCount)
            break;

        CurveLabels[i] = Pair.Key;
        Curves[i] = Pair.Value;
        ++i;
    }

    // 나머지 Curve는 초기화
    for (; i < MaxCurveCount; ++i)
    {
        CurveLabels[i] = FString::Printf(TEXT("Curve%d"), i);
        Curves[i] = FCurveData(); // 기본 초기화
    }
}

void SCurveEditorPanel::SaveToCSV(const FString& FilePath) const
{
    TMap<FString, FCurveData> CurveMap;
    for (int i = 0; i < MaxCurveCount; ++i)
    {
        CurveMap.Add(CurveLabels[i], Curves[i]);
    }

    FString Output = CurveIO::SerializeCurvesToCSV(CurveMap);
    FFileHelper::SaveStringToFile(Output, *FilePath);
}
bool SCurveEditorPanel::ImportSingleCurveFromCSV(const FString& FilePath, const FString& TargetLabel)
{
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
        return false;

    TMap<FString, FCurveData> ImportedCurves;
    if (!CurveIO::ParseCurvesFromCSV(FileContent, ImportedCurves))
        return false;

    const FCurveData* FoundCurve = ImportedCurves.Find(TargetLabel);
    if (!FoundCurve)
        return false;

    Curves[CurrentTabIndex] = *FoundCurve;
    CurveLabels[CurrentTabIndex] = TargetLabel;
    return true;
}
