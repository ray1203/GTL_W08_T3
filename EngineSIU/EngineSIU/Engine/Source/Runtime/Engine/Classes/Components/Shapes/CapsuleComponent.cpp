#include "CapsuleComponent.h"

#include "UObject/Casts.h"

UObject* UCapsuleComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewComponent;
}

void UCapsuleComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UCapsuleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int UCapsuleComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return 0;
}

void UCapsuleComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void UCapsuleComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}
