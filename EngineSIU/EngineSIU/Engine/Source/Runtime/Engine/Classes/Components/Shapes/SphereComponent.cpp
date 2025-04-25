#include "SphereComponent.h"

#include "UObject/Casts.h"

UObject* USphereComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewComponent;
}

void USphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int USphereComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return 0;
}

void USphereComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void USphereComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}
