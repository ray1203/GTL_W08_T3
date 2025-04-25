#pragma once
#include "ShapeComponent.h"
class UBoxComponent :
    public UShapeComponent
{
    DECLARE_CLASS(UBoxComponent, UShapeComponent)

public:
    UBoxComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;


    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    inline FVector GetBoxExtent() const { return BoxExtent; }
    inline void SetBoxExtent(const FVector& InBoxExtent)
    {
        BoxExtent = FVector::Max(InBoxExtent, FVector(KINDA_SMALL_NUMBER, KINDA_SMALL_NUMBER, KINDA_SMALL_NUMBER));
    }
private:
    // model space 기준
    FVector BoxExtent;
};
