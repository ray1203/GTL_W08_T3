#pragma once
#include "GameFramework/Actor.h"
#include "Classes/Components/ProjectileMovementComponent.h"


class UStaticMeshComponent;
class USpringArmComponent;
class UShapeComponent;
class UProjectileMovementComponent;

class APlayer : public AActor
{
    DECLARE_CLASS(APlayer, AActor)
    APlayer();

    virtual void BeginPlay() override;
    //virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    FVector GetVelocity() { return Movement->GetVelocity(); }
    void SetVelocity(FVector InVelocity) { Movement->SetVelocity(InVelocity); }

protected:
    UStaticMeshComponent* MeshComponent = nullptr;
    USpringArmComponent* CameraBoom = nullptr;
    UShapeComponent* Collider = nullptr;
    UProjectileMovementComponent* Movement = nullptr;
};
