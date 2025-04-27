#pragma once
#include "GameFramework/Actor.h"

class USphereComp;

class APlayerStart : public AActor
{
public:
    DECLARE_CLASS(APlayerStart, AActor)
    APlayerStart();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void SpawnPlayer();
    bool bIsSpawned = false;
};
