#include "SphereComponent.h"

#include "UObject/Casts.h"
#include "World/World.h"


UObject* USphereComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    
    NewComponent->SphereRadius = SphereRadius;

    return NewComponent;
}

void USphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USphereComponent::BeginPlay()
{
    if (this->GetWorld()->WorldType != EWorldType::Editor)
    {
        this->GetWorld()->PhysicsScene.AddRigidBody(this);

    }
}

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

int USphereComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    FRay Ray = FRay(rayOrigin, rayDirection);
    FSphere Sphere = FSphere(GetWorldLocation(), SphereRadius);

    return JungleCollision::RayIntersectsSphere(Ray, Sphere, &pfNearHitDistance);
}

void USphereComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void USphereComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}
