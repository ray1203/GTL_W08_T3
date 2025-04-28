#include "ShapeComponent.h"

#include "UObject/Casts.h"
#include "World/World.h"
#include "Physics/PhysicsScene.h"

UObject* UShapeComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));


    NewComponent->ShapeColor = ShapeColor;
    NewComponent->bDrawOnlyIfSelected = bDrawOnlyIfSelected;
    NewComponent->bIsSimulatingPhysics = bIsSimulatingPhysics;

    return NewComponent;
}

void UShapeComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UShapeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (this->GetWorld()->WorldType != EWorldType::Editor)
    {
         this->GetWorld()->PhysicsScene.RemoveRigidBody(this);
    }

    Super::EndPlay(EndPlayReason);
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

    OutProperties.Add(TEXT("bDrawOnlyIfSelected"), bDrawOnlyIfSelected ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bIsSimulatingPhysics"), bIsSimulatingPhysics ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("Mass"), FString::Printf(TEXT("%f"), Mass));

}

void UShapeComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    const FString* TempStr = nullptr;


    TempStr = InProperties.Find(TEXT("bDrawOnlyIfSelected")); // bAutoActive 변수가 있다고 가정
    if (TempStr)
    {
        this->bDrawOnlyIfSelected = TempStr->ToBool();
    }

    TempStr = InProperties.Find(TEXT("bIsSimulatingPhysics"));
    if (TempStr)
    {
        this->bIsSimulatingPhysics = TempStr->ToBool();
    }

    TempStr = InProperties.Find(TEXT("Mass"));
    if (TempStr)
    {
        Mass = FString::ToFloat(*TempStr);
    }

}

void UShapeComponent::OnOverlap(const FPhysicsBody& result)
{
}

const TArray<FOverlapInfo> UShapeComponent::GetOverlappingComponents() 
{ 

    if (this->GetWorld()->WorldType != EWorldType::Editor)
    {
        TArray<FOverlapInfo> OverlappingComponents;
        this->GetWorld()->PhysicsScene.GetOverlappings(this, OverlappingComponents);
        return OverlappingComponents;
    }
}
