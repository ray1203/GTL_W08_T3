#pragma once
// Copyright Epic Games, Inc. All Rights Reserved.
#include "Math/Vector.h"

class AActor;
class UPackageMap;
class UPhysicalMaterial;
class UPrimitiveComponent;

// Hit의 Trace와 관련된 정보
struct FHitResult
{
    // TraceStart에서 충돌시까지의 시간 (0.0~1.0)
    float Time = 0.f;

    // TraceStart에서 충돌시까지의 거리
    float Distance = 0.f;

    // 충돌이 발생했을때 PrimitiveComponent의 위치
    FVector Location;

    // 충돌 접점의 위치
    FVector ImpactPoint;

    // 충돌 접점에서 충돌 대상(상대)의 법선 벡터
    // Capsule과 Sphere는 원의 중심을 향함
    FVector Normal;

    // Sweep할때 시작점
    FVector TraceStart;

    // Sweep할때 종료점
    FVector TraceEnd;

    // 충돌이 Blocking에 의해서 발생한것인지 여부
    // 만약 단순 overlapping이라면 false
    uint8 bBlockingHit : 1;

    // UShapeComponent를 사용하는 것을 추천
    UPrimitiveComponent* Component;

    //// Component를 가진 Owner
    ////만약 StaticMeshComponent의 collider로 쓴다면 UStaticMeshComponent가 되고, Actor의 collider로 쓴다면 AActor가 됩니다.
    //UObject* Owner;
};
