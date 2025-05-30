#include "World.h"

#include "Actors/Cube.h"
#include "Actors/EditorPlayer.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Camera/CameraComponent.h"
#include "Classes/Components/StaticMeshComponent.h"
#include "Components/SkySphereComponent.h"
#include "Components/Shapes/ShapeComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "Actors/HeightFogActor.h"
#include "Components/UI/UUIComponent.h"
#include "Engine/EditorEngine.h"
#include "Engine/Engine.h"
#include "Engine/GameEngine.h"
#include "UnrealEd/SceneManager.h"
#include "Engine/Physics/PhysicsScene.h"
#include "Actors/Player.h"

class UEditorEngine;

UWorld* UWorld::CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName)
{
    UWorld* NewWorld = FObjectFactory::ConstructObject<UWorld>(InOuter);
    NewWorld->WorldName = InWorldName;
    NewWorld->WorldType = InWorldType;
    NewWorld->InitializeNewWorld();

    return NewWorld;
}

void UWorld::InitializeNewWorld()
{
    ActiveLevel = FObjectFactory::ConstructObject<ULevel>(this);
    ActiveLevel->InitLevel(this);

}

UObject* UWorld::Duplicate(UObject* InOuter)
{
    // TODO: UWorld의 Duplicate는 역할 분리후 만드는것이 좋을듯
    UWorld* NewWorld = Cast<UWorld>(Super::Duplicate(InOuter));
    NewWorld->ActiveLevel = Cast<ULevel>(ActiveLevel->Duplicate(NewWorld));
    NewWorld->ActiveLevel->InitLevel(NewWorld);
    
    // PhysicsScene은 복사하지 않음.(PIE에서만 작동하므로)


    return NewWorld;
}

void UWorld::Tick(float DeltaTime)
{
    // 물리 연산 먼저 처리
    if (this->WorldType != EWorldType::Editor)
    {
        if (this->WorldType == EWorldType::Game)
        {
            if (GEngine->bLoadScene)return;
        }
        PhysicsScene.TickPhysScene(DeltaTime);
        PhysicsScene.SyncBodies();

        // Editor에서는 BeginPlay를 호출하지 않음
        // SpawnActor()에 의해 Actor가 생성된 경우, 여기서 BeginPlay 호출
        for (AActor* Actor : PendingBeginPlayActors)
        {
            Actor->BeginPlay();
        }
        PendingBeginPlayActors.Empty();
    }
}

void UWorld::BeginPlay()
{
    // 월드가 시작할 때 등록된 액터들을 물리 scene에 등록
    // 월드 도중에 생성된 액터들은 
    for (AActor* Actor : ActiveLevel->Actors)
    {
        if (Actor->GetWorld() == this)
        {
            Actor->BeginPlay();
        }
    }
}

void UWorld::Release()
{
    if (ActiveLevel)
    {
        ActiveLevel->Release();
        GUObjectArray.MarkRemoveObject(ActiveLevel);
        ActiveLevel = nullptr;
    }
    
    GUObjectArray.ProcessPendingDestroyObjects();
}

AActor* UWorld::SpawnActor(UClass* InClass, FName InActorName)
{
    if (!InClass)
    {
        UE_LOG(ELogLevel::Error, TEXT("SpawnActor failed: ActorClass is null."));
        return nullptr;
    }

    
    // TODO: SpawnParams에서 이름 가져오거나, 필요시 여기서 자동 생성
    // if (SpawnParams.Name != NAME_None) ActorName = SpawnParams.Name;
    
    if (InClass->IsChildOf<AActor>())
    {
        AActor* NewActor = Cast<AActor>(FObjectFactory::ConstructObject(InClass, this, InActorName));
        // TODO: 일단 AddComponent에서 Component마다 초기화
        // 추후에 RegisterComponent() 만들어지면 주석 해제
        // Actor->InitializeComponents();
        ActiveLevel->Actors.Add(NewActor);
        PendingBeginPlayActors.Add(NewActor);
        NewActor->PostSpawn();
        return NewActor;
    }
    
    UE_LOG(ELogLevel::Error, TEXT("SpawnActor failed: Class '%s' is not derived from AActor."), *InClass->GetName());
    return nullptr;
}

bool UWorld::DestroyActor(AActor* ThisActor)
{
    if (ThisActor->GetWorld() == nullptr)
    {
        return false;
    }

    if (ThisActor->IsActorBeingDestroyed())
    {
        return true;
    }
    
    // UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    //
    // Engine->DeselectActor(ThisActor);

    // 액터의 Destroyed 호출
    ThisActor->Destroyed();

    if (ThisActor->GetOwner())
    {
        ThisActor->SetOwner(nullptr);
    }

    TSet<UActorComponent*> Components = ThisActor->GetComponents();
    for (UActorComponent* Component : Components)
    {
        Component->DestroyComponent();
    }

    // World에서 제거
    ActiveLevel->Actors.Remove(ThisActor);

    // 제거 대기열에 추가
    GUObjectArray.MarkRemoveObject(ThisActor);
    return true;
}

UWorld* UWorld::GetWorld() const
{
    return const_cast<UWorld*>(this);
}

void UWorld::RenderUIComponents()
{
    for (AActor* Actor : ActiveLevel->Actors)
    {
        for (UActorComponent* Comp : Actor->GetComponents())
        {
            if (auto UIComp = dynamic_cast<UUIComponent*>(Comp))
            {
                if (UIComp->IsActive())
                {
                    UIComp->RenderUI();
                }
            }
        }
    }
}

APlayer* UWorld::GetPlayer() const
{
    ULevel* Level = GetActiveLevel();

    if (Level)
    {
        for (AActor* Actor : Level->Actors)
        {
            if (APlayer* Player = Cast<APlayer>(Actor))
            {
                return Player;
            }
        }
    }

    return nullptr;
}

APlayerCameraManager* UWorld::GetPlayerCameraManager() const
{
    ULevel* Level = GetActiveLevel();
    if (Level)
    {
        for (AActor* Actor : Level->Actors)
        {
            if (APlayerCameraManager* CameraManager = Cast<APlayerCameraManager>(Actor))
            {
                return CameraManager;
            }
        }
    }
    return nullptr;
}

bool UWorld::LineTraceSingle(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
    return PhysicsScene.RayTraceSingle(OutHit, Start, End);
}
