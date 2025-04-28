#pragma once

#include "Components/PrimitiveComponent.h"
#include "Math/JungleCollision.h"
#include "Math/JungleMath.h"
#include "Math/Color.h"

class UShapeComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UShapeComponent, UPrimitiveComponent)

public:
    UShapeComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;


    virtual void InitializeComponent() override;
    /** 모든 초기화가 끝나고, 준비가 되었을 때 호출됩니다. */
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
 
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    FColor GetShapeColor() const { return ShapeColor; }
    void SetShapeColor(const FColor& InShapeColor) { ShapeColor = InShapeColor; }

    bool GetDrawOnlyIfSelected() const { return bDrawOnlyIfSelected; }
    void SetDrawOnlyIfSelected(bool InDrawOnlyIfSelected) { bDrawOnlyIfSelected = InDrawOnlyIfSelected; }

    FColor ShapeColor;
    bool bDrawOnlyIfSelected : 1 = false; // true : 선택된 경우에만 그려짐. false : 항상 그려짐
    bool bIsSimulatingPhysics : 1 = true;
    bool bGrounded : 1 = false;
    float Restitution = 1.0f;
    float Mass = 1.f;
};
