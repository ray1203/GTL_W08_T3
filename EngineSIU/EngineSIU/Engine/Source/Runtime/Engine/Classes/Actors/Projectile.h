#pragma once
#include "GameFramework/Actor.h"

class UStaticMeshComponent;
class USphereComponent;
class UProjectileMovementComponent;

class AProjectile : public AActor
{
    DECLARE_CLASS(AProjectile, AActor)
public:
    AProjectile();
    virtual void PostSpawn() override;
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void SetInitialSpeed(const FVector& InitialSpeed);

private:
    float LifeTime = 5.0f;
    float dt = 0.0f;

    UStaticMeshComponent* MeshComponent = nullptr;
    USphereComponent* SphereComponent = nullptr;
    UProjectileMovementComponent* ProjectileMovementComponent = nullptr;
};
