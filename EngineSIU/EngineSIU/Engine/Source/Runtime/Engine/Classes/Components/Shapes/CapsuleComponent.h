#pragma once
#include "ShapeComponent.h"
class UCapsuleComponent :
    public UShapeComponent
{
    DECLARE_CLASS(UCapsuleComponent, UShapeComponent)

public:
    UCapsuleComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;

    /** 모든 초기화가 끝나고, 준비가 되었을 때 호출됩니다. */
    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;


    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    inline float GetCapsuleHalfHeight() const { return CapsuleHalfHeight; }
    inline void SetCapsuleHalfHeight(float InCapsuleHalfHeight)
    {
        CapsuleHalfHeight = FMath::Max(InCapsuleHalfHeight, KINDA_SMALL_NUMBER);
    }
    inline float GetCapsuleRadius() const { return CapsuleRadius; }
    inline void SetCapsuleRadius(float InCapsuleRadius)
    {
        CapsuleRadius = FMath::Max(InCapsuleRadius, KINDA_SMALL_NUMBER);
    }
private:
    // model space 기준

    float CapsuleHalfHeight = 1;
    float CapsuleRadius = 1;

};
