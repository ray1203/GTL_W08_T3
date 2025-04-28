#pragma once
#include "ShapeComponent.h"
class USphereComponent :
    public UShapeComponent
{
    DECLARE_CLASS(USphereComponent, UShapeComponent)

public:
    USphereComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    /** 모든 초기화가 끝나고, 준비가 되었을 때 호출됩니다. */
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;


    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    float GetSphereRadius() const { return SphereRadius; }
    void SetSphereRadius(float InSphereRadius)
    {
        SphereRadius = FMath::Max(InSphereRadius, KINDA_SMALL_NUMBER);
    }
private:
    // model space 기준
    float SphereRadius = 1;
};
