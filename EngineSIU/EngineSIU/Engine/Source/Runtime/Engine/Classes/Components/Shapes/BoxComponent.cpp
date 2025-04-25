#include "BoxComponent.h"

#include "UObject/Casts.h"

UObject* UBoxComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    return NewComponent;
}

void UBoxComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UBoxComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int UBoxComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return 0;
}

void UBoxComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void UBoxComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}
