#include "CapsuleComponent.h"

#include "UObject/Casts.h"

#include "World/World.h"

UObject* UCapsuleComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->CapsuleHalfHeight = CapsuleHalfHeight;
    NewComponent->CapsuleRadius = CapsuleRadius;

    return NewComponent;
}

void UCapsuleComponent::InitializeComponent()
{
    Super::InitializeComponent();
}


void UCapsuleComponent::BeginPlay()
{
    if (this->GetWorld()->WorldType != EWorldType::Editor)
    {
        this->GetWorld()->PhysicsScene.AddRigidBody(this);

    }
}

void UCapsuleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int UCapsuleComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    FRay Ray(rayOrigin, rayDirection);

    FCapsule Capsule;
    Capsule.A = GetWorldLocation() + GetForwardVector() * CapsuleHalfHeight;
    Capsule.B = GetWorldLocation() - GetForwardVector() * CapsuleHalfHeight;
    Capsule.Radius = CapsuleRadius;

    return JungleCollision::RayIntersectsCapsule(Ray, Capsule, &pfNearHitDistance);
}

void UCapsuleComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("CapsuleHalfHeight"), FString::SanitizeFloat(CapsuleHalfHeight));
    OutProperties.Add(TEXT("CapsuleRadius"), FString::SanitizeFloat(CapsuleRadius));

}

void UCapsuleComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("CapsuleHalfHeight"));
    if (TempStr)
    {
        CapsuleHalfHeight = FCString::Atof(**TempStr);
    }
    TempStr = InProperties.Find(TEXT("CapsuleRadius"));
    if (TempStr)
    {
        CapsuleRadius = FCString::Atof(**TempStr);
    }

}
