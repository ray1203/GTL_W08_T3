#pragma once
#include "Define.h"
#include "Container/Set.h"
#include "UObject/ObjectFactory.h"
#include "UObject/ObjectMacros.h"
#include "WorldType.h"
#include "Level.h"
#include "Physics/PhysicsScene.h"
#include "Camera/PlayerCameraManager.h"

class FObjectFactory;
class AActor;
class UObject;
class USceneComponent;
class APlayer;

class UWorld : public UObject
{
    DECLARE_CLASS(UWorld, UObject)

public:
    UWorld() = default;

    static UWorld* CreateWorld(UObject* InOuter, const EWorldType InWorldType, const FString& InWorldName = "DefaultWorld");

    void InitializeNewWorld();

    virtual UObject* Duplicate(UObject* InOuter) override;

    void Tick(float DeltaTime);
    void BeginPlay();

    void Release();

    /**
     * World에 Actor를 Spawn합니다.
     * @param InClass Spawn할 Actor 정보
     * @return Spawn된 Actor
     */
    AActor* SpawnActor(UClass* InClass, FName InActorName = NAME_None);

    /** 
     * World에 Actor를 Spawn합니다.
     * @tparam T AActor를 상속받은 클래스
     * @return Spawn된 Actor의 포인터
     */
    template <typename T>
        requires std::derived_from<T, AActor>
    T* SpawnActor();

    /** World에 존재하는 Actor를 제거합니다. */
    bool DestroyActor(AActor* ThisActor);

    virtual UWorld* GetWorld() const override;
    ULevel* GetActiveLevel() const { return ActiveLevel; }

    template <typename T>
        requires std::derived_from<T, AActor>
    T* DuplicateActor(T* InActor);

    EWorldType WorldType = EWorldType::None;

    void RenderUIComponents();


    // TODO: 생성하는 위치 정하면 private로 넣기
    // 현재는 외부에서 생성하고 있음.
    // 물리 처리를 진행하는 Scene
    FPhysicsScene PhysicsScene;

    // 플레이어 반환
    APlayer* GetPlayer() const;
    APlayerCameraManager* GetPlayerCameraManager() const;

    // linetrace
    bool LineTraceSingle(FHitResult& OutHit, const FVector& Start, const FVector& End);

private:
    FString WorldName = "DefaultWorld";

    ULevel* ActiveLevel = nullptr;

    /** Actor가 Spawn되었고, 아직 BeginPlay가 호출되지 않은 Actor들 */
    TArray<AActor*> PendingBeginPlayActors;
};


template <typename T>
    requires std::derived_from<T, AActor>
T* UWorld::SpawnActor()
{
    return Cast<T>(SpawnActor(T::StaticClass()));
}

template <typename T>
    requires std::derived_from<T, AActor>
T* UWorld::DuplicateActor(T* InActor)
{
    if (ULevel* ActiveLevel = GetActiveLevel())
    {
        T* NewActor = static_cast<T*>(InActor->Duplicate(this));
        ActiveLevel->Actors.Add(NewActor);
        PendingBeginPlayActors.Add(NewActor);
        return NewActor;
    }
    return nullptr;
}

