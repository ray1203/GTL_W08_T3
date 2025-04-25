#pragma once
#include "HitResult.h"

class UPrimitiveComponent;

/** Overlap info consisting of the primitive and the body that is overlapping */
struct FOverlapInfo
{
    FOverlapInfo() = default;

    explicit FOverlapInfo(UPrimitiveComponent* InComponent)
    {
        ZeroMemory(&OverlapInfo, sizeof(FHitResult));
        OverlapInfo.Component = InComponent;
    }

    friend bool operator == (const FOverlapInfo& LHS, const FOverlapInfo& RHS) { return LHS.OverlapInfo.Component == RHS.OverlapInfo.Component; }
    bool bFromSweep = false;

    // sweep과 overlap의 검사(쿼리) 결과를 저장합니다.
    // 만약 bFromSweep이 false라면 움직임의 결과로 발생한 충돌의 정보를 가진 것이 아니라 아니라,
    // 정지된 상태에서의 정보, 즉 overlap되어있는 상태에서의 정보를 담고 있습니다.
    // 따라서 FHitResult::Component만 담겨 있습니다.
    FHitResult OverlapInfo;
};
