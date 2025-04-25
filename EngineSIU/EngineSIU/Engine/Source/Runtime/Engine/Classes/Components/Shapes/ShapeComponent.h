#include "Components/PrimitiveComponent.h"

#include "Math/Color.h"

class UShapeComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UShapeComponent, UPrimitiveComponent)

public:
    UShapeComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    FColor GetShapeColor() const { return ShapeColor; }
    void SetShapeColor(const FColor& InShapeColor) { ShapeColor = InShapeColor; }

    bool GetDrawOnlyIfSelected() const { return bDrawOnlyIfSelected; }
    void SetDrawOnlyIfSelected(bool InDrawOnlyIfSelected) { bDrawOnlyIfSelected = InDrawOnlyIfSelected; }

private:
    FColor ShapeColor;
    bool bDrawOnlyIfSelected; // true : 선택된 경우에만 그려짐. false : 항상 그려짐
};
