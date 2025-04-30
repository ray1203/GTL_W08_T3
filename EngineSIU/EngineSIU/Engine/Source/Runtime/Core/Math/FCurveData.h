#pragma once
#include "Container/Array.h"


struct FVector2D;

// 단일 커브 데이터를 표현하는 클래스
class FCurveData
{
public:
    FCurveData();

    // 최대 포인트 수
    int MaxPoints;

    // 현재 선택된 포인트 인덱스
    int SelectedIndex;

    // 커브 제어 포인트 (0 ~ MaxPoints 사이)
    TArray<FVector2D> ControlPoints;

    // 선형 보간 기반 평가
    float EvaluateLinear(float X) const;

    // 스플라인 기반 평가
    float EvaluateSmooth(float X) const;

    // CSV 직렬화
    FString ToCSV() const;
    bool FromCSV(const FString& CSVText);
};
