#include "Player.h"
#include "Components/StaticMeshComponent.h"
#include <Engine/FLoaderOBJ.h>
#include "Camera/SpringArmComponent.h"

APlayer::APlayer()
{
    MeshComponent = AddComponent<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;
    
    MeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));

    CameraBoom = AddComponent<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(MeshComponent);
    CameraBoom->SetTargetArmLength(300.0f);
    CameraBoom->SetSocketOffset(FVector(0, 50.0f, 50.0f));

    

}

void APlayer::BeginPlay()
{
}

void APlayer::Tick(float DeltaTime)
{
}
