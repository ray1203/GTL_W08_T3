#pragma once
#include "GameFramework/Actor.h"
#include "Components/Shapes/BoxComponent.h" 

class ARiceMonkey : public AActor
{
    DECLARE_CLASS(ARiceMonkey, AActor)

    ARiceMonkey();

    virtual void PostSpawn() override;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    void OnOverlap(const FPhysicsBody& result);

    bool bIsAngry = false;
    float SsalMass = 1.0f;
    float AttackSpeed = 1.0f;
    int HP = 5;
    FVector Color;

private:
    class UStaticMeshComponent* MeshComponent = nullptr;
    class UBoxComponent* Collider = nullptr;
    class ULuaScriptComponent* LuaComp = nullptr;
};

