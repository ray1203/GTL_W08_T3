#pragma once
#include "GameFramework/Actor.h"
#include "Classes/Components/ProjectileMovementComponent.h"
#include "Components/Shapes/ShapeComponent.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UShapeComponent;
class UProjectileMovementComponent;
class UCameraComponent;

class APlayer : public AActor
{
    DECLARE_CLASS(APlayer, AActor)
    APlayer();

    virtual void PostSpawn() override;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void BeginPlay() override;
    //virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    FVector GetVelocity() { return Movement->GetVelocity(); }
    FVector GetAcceleration() { return Movement->GetAcceleration(); }
    void SetVelocity(FVector InVelocity) { Movement->SetVelocity(InVelocity); }
    void SetAcceleration(FVector InAcceleration) { Movement->SetAcceleration(InAcceleration); }

    bool IsGrounded() {
        if (Collider)
        {
            return Collider->bGrounded;
        }
    }

    void OnOverlap(const FPhysicsBody& result);


protected:
    UStaticMeshComponent* MeshComponent = nullptr;
    USpringArmComponent* CameraBoom = nullptr;
    UCameraComponent* CameraComp = nullptr;
    UShapeComponent* Collider = nullptr;
    UProjectileMovementComponent* Movement = nullptr;

public:
    bool bInputBlock = false;
};
