#pragma once
#include "GameFramework/Actor.h"
#include "Components/Shapes/BoxComponent.h" 

class ALandBlock :
    public AActor
{
    DECLARE_CLASS(ALandBlock, AActor)
public:
    ALandBlock();

    virtual void PostSpawn() override;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    void OnOverlap(const FPhysicsBody& result);

    void GenerateNextLevel();
private:
    bool bIsStepped = false;

    int Level = 1;

    class UStaticMeshComponent* MeshComponent = nullptr;
    class UBoxComponent* Collider = nullptr;
    class ULuaScriptComponent* LuaComp = nullptr;
};

