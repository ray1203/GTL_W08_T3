#include "CurveManager.h"

#include "Misc/FileHelper.h"
#include "Math/StringUtils.h"
bool CurveIO::ParseCurvesFromCSV(const FString& Text, TMap<FString, FCurveData>& OutCurves)
{
    OutCurves.Empty();
    FString CurrentLabel;
    FString CurveCSV;

    TArray<FString> Lines;
    StringUtils::ParseIntoArrayLines(Text, Lines);

    for (const FString& Line : Lines)
    {
        if (Line.IsEmpty())
        {
            if (!CurrentLabel.IsEmpty() && !CurveCSV.IsEmpty())
            {
                FCurveData Curve;
                if (Curve.FromCSV(CurveCSV))
                {
                    OutCurves.Add(CurrentLabel, Curve);
                }
            }
            CurrentLabel.Empty();
            CurveCSV.Empty();
            continue;
        }

        if (!Line.Contains(TEXT(",")))
        {
            // 새 라벨 시작
            if (!CurrentLabel.IsEmpty() && !CurveCSV.IsEmpty())
            {
                FCurveData Curve;
                if (Curve.FromCSV(CurveCSV))
                {
                    OutCurves.Add(CurrentLabel, Curve);
                }
            }

            CurrentLabel = Line;
            CurveCSV.Empty();
        }
        else
        {
            CurveCSV += Line + TEXT("\n");
        }
    }

    // 마지막 곡선 처리
    if (!CurrentLabel.IsEmpty() && !CurveCSV.IsEmpty())
    {
        FCurveData Curve;
        if (Curve.FromCSV(CurveCSV))
        {
            OutCurves.Add(CurrentLabel, Curve);
        }
    }

    return OutCurves.Num() > 0;
}


FString CurveIO::SerializeCurvesToCSV(const TMap<FString, FCurveData>& Curves)
{
    FString Output;
    for (const auto& Pair : Curves)
    {
        Output += Pair.Key + TEXT("\n");
        Output += Pair.Value.ToCSV();
        Output += TEXT("\n");
    }
    return Output;
}
FCurveManager& FCurveManager::Get()
{
    static FCurveManager Instance;
    return Instance;
}

// FCurveManager::LoadFromFile
bool FCurveManager::LoadFromFile(const FString& FilePath)
{
    FString Content;
    if (!FFileHelper::LoadFileToString(Content, *FilePath))
        return false;

    FString SetName = GetFileNameWithoutExtension(GetData(FilePath));
    return LoadFromCSV(Content, SetName); // 기존 내부 API 활용
}



bool FCurveManager::SaveToFile(const FString& FilePath) const
{
    FString SetName = GetFileNameWithoutExtension(GetData(FilePath));
    FString Content = SaveToCSV(SetName);
    return FFileHelper::SaveStringToFile(Content, *FilePath);
}

bool FCurveManager::LoadFromCSV(const FString& Text, const FString& SetName)
{
    if (SetName.IsEmpty())
        return false;

    TMap<FString, FCurveData>& SetMap = CurveMap.FindOrAdd(SetName);
    return CurveIO::ParseCurvesFromCSV(Text, SetMap);
}


FString FCurveManager::SaveToCSV(const FString& SetName) const
{
    const TMap<FString, FCurveData>* SetMap = CurveMap.Find(SetName);
    return SetMap ? CurveIO::SerializeCurvesToCSV(*SetMap) : FString();
}


void FCurveManager::RegisterCurve(const FString& SetName, const FString& Label, const FCurveData& Curve)
{
    CurveMap.FindOrAdd(SetName).Add(Label, Curve);
}

FCurveData* FCurveManager::GetCurve(const FString& SetName, const FString& Label)
{
    TMap<FString, FCurveData>* SetMap = CurveMap.Find(SetName);
    return SetMap ? SetMap->Find(Label) : nullptr;
}

const FCurveData* FCurveManager::GetCurve(const FString& SetName, const FString& Label) const
{
    const TMap<FString, FCurveData>* SetMap = CurveMap.Find(SetName);
    return SetMap ? SetMap->Find(Label) : nullptr;
}

bool FCurveManager::CopyCurve(const FString& SetName, const FString& FromLabel, const FString& ToLabel)
{
    const FCurveData* From = GetCurve(SetName, FromLabel);
    if (!From)
        return false;

    RegisterCurve(SetName, ToLabel, *From);
    return true;
}
std::string GetFileNameWithoutExtension(const std::string& path)
{
    size_t slash = path.find_last_of("/\\");
    size_t dot = path.find_last_of(".");

    if (dot == std::string::npos || dot < slash)
        dot = path.length();

    return path.substr(slash + 1, dot - slash - 1);
}
