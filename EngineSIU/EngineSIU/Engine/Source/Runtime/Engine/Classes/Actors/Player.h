#pragma once
#include "GameFramework/Actor.h"

class UStaticMeshComponent;
class USpringArmComponent;

class APlayer : public AActor
{
    DECLARE_CLASS(APlayer, AActor)
    APlayer();

    virtual void BeginPlay() override;
    //virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

protected:
    UStaticMeshComponent* MeshComponent = nullptr;
    USpringArmComponent* CameraBoom = nullptr;
};
