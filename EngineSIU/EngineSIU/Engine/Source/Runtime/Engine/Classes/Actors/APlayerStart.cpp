#include "APlayerStart.h"
#include "Actors/Player.h"
#include "World/World.h"
#include <Engine/FLoaderOBJ.h>
#include "Components/SphereComp.h"

APlayerStart::APlayerStart()
{
    USphereComp* SphereComp = AddComponent<USphereComp>(TEXT("SphereComponent"));
    RootComponent = SphereComp;
    SphereComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Sphere.obj"));
}

void APlayerStart::BeginPlay()
{
    AActor::BeginPlay();
    // Set the initial location and rotation of the player start actor


}

void APlayerStart::Tick(float DeltaTime)
{
    // BeginPlay시점에 액터를 스폰할 수 없음. Tick에서 플래그체크해서 스폰
    AActor::Tick(DeltaTime);
    if (bIsSpawned)
    {
        return;
    }
    SpawnPlayer();
}

void APlayerStart::SpawnPlayer()
{
    UWorld* World = GetWorld();

    if (!World || World->WorldType == EWorldType::Editor)
        return;

    if (ULevel* CurrentLevel = World->GetActiveLevel())
    {
        for (AActor* Actor : CurrentLevel->Actors)
        {
            if (Actor && Actor->IsA<APlayer>())
            {
                // 2개 이상의 PlayerStart가 존재할 경우, 첫 번째 PlayerStart만 사용
                bIsSpawned = true;
                return;
            }
        }
    }

    // PlayerActor를 스폰
    APlayer* PlayerActor = World->SpawnActor<APlayer>();
    PlayerActor->SetActorLocation(GetActorLocation());
    PlayerActor->SetActorRotation(GetActorRotation());

    // 자기 자신 제거
    World->DestroyActor(this);
    bIsSpawned = true;
}
