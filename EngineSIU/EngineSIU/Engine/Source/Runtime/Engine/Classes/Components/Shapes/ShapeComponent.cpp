#include "ShapeComponent.h"

#include "UObject/Casts.h"

UObject* UShapeComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewComponent;
}

void UShapeComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UShapeComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int UShapeComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return 0;
}

void UShapeComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void UShapeComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}
