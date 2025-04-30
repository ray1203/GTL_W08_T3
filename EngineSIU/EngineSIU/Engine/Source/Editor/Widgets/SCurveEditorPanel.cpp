// SCurveEditorPanel.cpp
#include "SCurveEditorPanel.h"

#include "ImCurveWidget.h"
#include "tinyfiledialogs.h"
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

    // 모든 곡선 초기화
    for (int i = 0; i < MaxCurveCount; ++i)
    {
        Curves[i].ControlPoints.SetNum(MaxPoints);
        for (int j = 0; j < MaxPoints; ++j)
        {
            Curves[i].ControlPoints[j] = FVector2D(0.f, 0.f);
        }
        Curves[i].ControlPoints[0] = FVector2D(0.0f, 0.0f);
        Curves[i].ControlPoints[1] = FVector2D(1.0f, 0.0f);
        Curves[i].ControlPoints[2].X = ImGui::CurveTerminator;
        Curves[i].SelectedIndex = -1;
    }

    TArray<FString> Lines;
    StringUtils::ParseIntoArrayLines(FileContent, Lines);

    int CurveIdx = -1;
    int PointIdx = 0;

    for (const FString& Line : Lines)
    {
        if (Line.IsEmpty())
            continue;

        if (!Line.Contains(TEXT(","))) // 새 곡선의 라벨 (Curve name)
        {
            ++CurveIdx;
            if (CurveIdx >= MaxCurveCount)
                break;

            CurveLabels[CurveIdx] = Line;
            PointIdx = 0;
            continue;
        }

        if (CurveIdx < 0 || CurveIdx >= MaxCurveCount || PointIdx >= MaxPoints)
            continue;

        TArray<FString> Tokens;
        StringUtils::ParseIntoArray(Line, Tokens, TEXT(","), true);
        if (Tokens.Num() != 2)
            continue;

        float X = FCString::Atof(*Tokens[0]);
        float Y = FCString::Atof(*Tokens[1]);

        if (FMath::IsNearlyEqual(X, ImGui::CurveTerminator))
            break;

        Curves[CurveIdx].ControlPoints[PointIdx++] = FVector2D(X, Y);

        if (PointIdx < MaxPoints)
        {
            Curves[CurveIdx].ControlPoints[PointIdx].X = ImGui::CurveTerminator;
        }
    }
}



void SCurveEditorPanel::SaveToCSV(const FString& FilePath) const
{
    FString Output;

    for (int CurveIdx = 0; CurveIdx < MaxCurveCount; ++CurveIdx)
    {
        Output += CurveLabels[CurveIdx] + TEXT("\n");

        for (int i = 0; i < MaxPoints; ++i)
        {
            const FVector2D& Pt = Curves[CurveIdx].ControlPoints[i];
            if (FMath::IsNearlyEqual(Pt.X, ImGui::CurveTerminator))
                break;
            Output += FString::Printf(TEXT("%.6f,%.6f\n"), Pt.X, Pt.Y);
        }

        Output += TEXT("\n"); // Curve 간 공백
    }

    FFileHelper::SaveStringToFile(Output, *FilePath);
}
bool SCurveEditorPanel::ImportSingleCurveFromCSV(const FString& FilePath, const FString& TargetLabel)
{
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
        return false;

    TArray<FString> Lines;
    StringUtils::ParseIntoArrayLines(FileContent, Lines);

    int CurveIdx = -1;
    int PointIdx = 0;
    bool bFound = false;

    for (int LineIdx = 0; LineIdx < Lines.Num(); ++LineIdx)
    {
        FString Line = Lines[LineIdx];
        if (Line.IsEmpty())
            continue;

        if (!Line.Contains(TEXT(","))) // 곡선 이름
        {
            if (Line == TargetLabel)
            {
                bFound = true;
                ++CurveIdx;
                PointIdx = 0;
                CurveLabels[CurrentTabIndex] = TargetLabel;
                continue;
            }
            else
            {
                bFound = false;
            }
        }
        else if (bFound)
        {
            if (PointIdx >= MaxPoints)
                break;

            TArray<FString> Tokens;
            StringUtils::ParseIntoArray(Line, Tokens, TEXT(","), true);
            if (Tokens.Num() != 2)
                continue;

            float X = FCString::Atof(*Tokens[0]);
            float Y = FCString::Atof(*Tokens[1]);

            if (FMath::IsNearlyEqual(X, ImGui::CurveTerminator))
                break;

            Curves[CurrentTabIndex].ControlPoints[PointIdx++] = FVector2D(X, Y);
        }

        // 다음 Curve 시작 전 Terminator 설정
        if (bFound && (LineIdx + 1 >= Lines.Num() || Lines[LineIdx + 1].IsEmpty()))
        {
            if (PointIdx < MaxPoints)
                Curves[CurrentTabIndex].ControlPoints[PointIdx].X = ImGui::CurveTerminator;
            break;
        }
    }

    return bFound;
}
