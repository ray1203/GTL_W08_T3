#include "Player.h"
#include "Components/StaticMeshComponent.h"
#include <Engine/FLoaderOBJ.h>
#include "Camera/SpringArmComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Components/Shapes/SphereComponent.h"


#include "Components/Lua/LuaScriptComponent.h"

APlayer::APlayer()
{

}

void APlayer::PostSpawn()
{
    Super::PostSpawn();
    MeshComponent = AddComponent<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

    MeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));

    CameraBoom = AddComponent<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(MeshComponent);
    CameraBoom->SetTargetArmLength(10.0f);
    CameraBoom->SetSocketOffset(FVector(0.f, 0.0f, 0.0f));

    Collider = AddComponent<USphereComponent>(TEXT("Collider"));
    Collider->SetupAttachment(MeshComponent);
	Collider->Restitution = 0.3f;
    Movement = AddComponent<UProjectileMovementComponent>(TEXT("Movement"));
    Movement->SetVelocity(FVector(0, 0, 10.f));
    LuaComp->SetScriptPath(TEXT("TestLuaActor"));


#if !GAME_BUILD
    SetActorTickInEditor(false);
#endif
}

UObject* APlayer::Duplicate(UObject* InOuter)
{
    ThisClass* NewPlayer = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewPlayer->MeshComponent = NewPlayer->GetComponentByClass<UStaticMeshComponent>();
    NewPlayer->CameraBoom = NewPlayer->GetComponentByClass<USpringArmComponent>();
    NewPlayer->Collider = NewPlayer->GetComponentByClass<USphereComponent>();
    NewPlayer->Movement = NewPlayer->GetComponentByClass<UProjectileMovementComponent>();

    return NewPlayer;
}

void APlayer::BeginPlay()
{
    Super::BeginPlay();
}

void APlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

