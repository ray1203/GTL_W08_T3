#pragma once
#include"Components/SceneComponent.h"

class UProjectileMovementComponent : public USceneComponent
{
    DECLARE_CLASS(UProjectileMovementComponent, USceneComponent)
public:
    UProjectileMovementComponent();
    virtual ~UProjectileMovementComponent();

    virtual UObject* Duplicate(UObject* InOuter) override;

    void SetVelocity(const FVector& NewVelocity) { Velocity = NewVelocity; }

    FVector GetVelocity() const { return Velocity; }

    void SetInitialSpeed(float NewInitialSpeed) { InitialSpeed = NewInitialSpeed; }

    float GetInitialSpeed() const { return InitialSpeed; }

    void SetMaxSpeed(float NewMaxSpeed) { MaxSpeed = NewMaxSpeed; }

    float GetMaxSpeed() const { return MaxSpeed; }

    void SetAcceleration(FVector NewAcc) { Acceleration = NewAcc; }

    FVector GetAcceleration() const { return Acceleration; }

    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime) override;
    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;

private:
    float AccumulatedTime;

    float InitialSpeed;
    float MaxSpeed;

    FVector Acceleration;
    FVector Velocity;
};

