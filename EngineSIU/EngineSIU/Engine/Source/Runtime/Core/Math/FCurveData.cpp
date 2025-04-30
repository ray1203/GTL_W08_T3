#include "FCurveData.h"

#include "StringUtils.h"
#include "Vector.h"
#include "ImGUI/imgui.h"
#include "Widgets/ImCurveWidget.h"

FCurveData::FCurveData()
{
    MaxPoints = 10;
    SelectedIndex = -1;

    ControlPoints.SetNum(MaxPoints);
    ControlPoints[0] = FVector2D(0.0f, 0.0f);
    ControlPoints[1] = FVector2D(1.0f, 0.0f);
    ControlPoints[2] = FVector2D(ImGui::CurveTerminator, 0.0f); // 종료 지점 표시
}

float FCurveData::EvaluateLinear(float X) const
{
    return ImGui::CurveValue(X, MaxPoints, reinterpret_cast<const ImVec2*>(ControlPoints.GetData()));
}

float FCurveData::EvaluateSmooth(float X) const
{
    return ImGui::CurveValueSmooth(X, MaxPoints, reinterpret_cast<const ImVec2*>(ControlPoints.GetData()));
}

FString FCurveData::ToCSV() const
{
    FString Result;
    for (int i = 0; i < MaxPoints; ++i)
    {
        const FVector2D& P = ControlPoints[i];
        if (P.X <= ImGui::CurveTerminator)
            break;

        Result += FString::Printf(TEXT("%.6f,%.6f\n"), P.X, P.Y);
    }
    return Result;
}

bool FCurveData::FromCSV(const FString& CSVText)
{
    TArray<FString> Lines;
    StringUtils::ParseIntoArrayLines(CSVText,Lines);
    ControlPoints.Empty();
    int Count = 0;

    for (const FString& Line : Lines)
    {
        TArray<FString> Tokens;
        StringUtils::ParseIntoArray(Line, Tokens, TEXT(","), true);
        if (Tokens.Num() != 2)
            continue;

        float X = FCString::Atof(*Tokens[0]);
        float Y = FCString::Atof(*Tokens[1]);

        ControlPoints.Add(FVector2D(X, Y));
        ++Count;

        if (Count >= MaxPoints - 1)
            break;
    }

    if (Count >= 2)
    {
        ControlPoints.Add(FVector2D(ImGui::CurveTerminator, 0.0f));
        ControlPoints.SetNum(MaxPoints);
        return true;
    }

    return false;
}
