#include "GameManager.h"

AGameManager::AGameManager()
{
}

void AGameManager::BeginPlay()
{
}

void AGameManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void AGameManager::InitGameWorld(AActor* Self)
{
    // 발판 100개 랜덤 생성
    UWorld* World = GetWorld();
    FVector InitialPos = FVector(10, 10, 0);
    //SpawnActor
}

void AGameManager::InitiateActor(AActor* Self, const std::string* ActorType, const FVector& SpawnPos, const FRotator& SpawnRotator)
{

}
