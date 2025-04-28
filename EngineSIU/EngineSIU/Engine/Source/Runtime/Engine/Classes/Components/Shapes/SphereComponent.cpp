#include "SphereComponent.h"

#include "UObject/Casts.h"
#include "World/World.h"
#include "Actors/Player.h"

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
    Super::BeginPlay();
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
    OutProperties.Add(TEXT("SphereRadius"), FString::Printf(TEXT("%f"), SphereRadius));
}

void USphereComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("SphereRadius"));
    if (TempStr)
    {
        SphereRadius = FCString::Atof(**TempStr);
    }
}

void USphereComponent::OnOverlap(const FPhysicsBody& result)
{
    UE_LOG(ELogLevel::Warning, TEXT("USphere Comp : OnOverlapped"));
    
}
