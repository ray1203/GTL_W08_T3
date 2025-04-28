#pragma once
#include "GameFramework/Actor.h"

class AGameManager : public AActor
{
    DECLARE_CLASS(AGameManager, AActor);

public:
    AGameManager();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    void InitGameWorld(AActor* Self);
    void InitiateActor(AActor* Self, const std::string* ActorType, const FVector& SpawnPos, const FRotator& SpawnRotator);
private:

};
