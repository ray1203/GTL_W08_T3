#pragma once

#include "Container/Map.h"
#include "Math/FCurveData.h"
namespace CurveIO
{
    bool ParseCurvesFromCSV(const FString& Text, TMap<FString, FCurveData>& OutCurves);
    FString SerializeCurvesToCSV(const TMap<FString, FCurveData>& Curves);
}
class FCurveManager
{
public:
    static FCurveManager& Get();

    // 저장 및 로딩
    bool LoadFromFile(const FString& FilePath);
    bool SaveToFile(const FString& FilePath) const;
private:
    bool LoadFromCSV(const FString& Text, const FString& SetName);
    FString SaveToCSV(const FString& SetName) const;
public:
    // 등록 및 조회
    void RegisterCurve(const FString& SetName, const FString& Label, const FCurveData& Curve);
    FCurveData* GetCurve(const FString& SetName, const FString& Label);
    const FCurveData* GetCurve(const FString& SetName, const FString& Label) const;

    bool CopyCurve(const FString& SetName, const FString& FromLabel, const FString& ToLabel);

private:
    TMap<FString /*SetName*/, TMap<FString /*Label*/, FCurveData>> CurveMap;
};
std::string GetFileNameWithoutExtension(const std::string& path);
