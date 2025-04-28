#include "Platform.h"
#include "Engine/FLoaderOBJ.h" 
#include "World/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Lua/LuaScriptComponent.h"
#include "Actors/Player.h"
#include "RiceMonkey.h"

APlatform::APlatform()
{
}

void APlatform::PostSpawn()
{
    Super::PostSpawn();
    MeshComponent = AddComponent<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/ssal.obj"));
    RootComponent = MeshComponent;

    Collider = AddComponent<UBoxComponent>(TEXT("Box"));
    Collider->SetupAttachment(MeshComponent);
    Collider->bIsSimulatingPhysics = false;

    LuaComp = AddComponent<ULuaScriptComponent>(TEXT("LuaScriptComponent"));
    LuaComp->SetScriptPath(TEXT("LandGenerate"));
}

UObject* APlatform::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewActor->MeshComponent = NewActor->GetComponentByClass<UStaticMeshComponent>();
    NewActor->Collider = NewActor->GetComponentByClass<UBoxComponent>();
    NewActor->LuaComp = NewActor->GetComponentByClass<ULuaScriptComponent>();
    NewActor->bIsStepped = bIsStepped;

    return NewActor;
}

void APlatform::BeginPlay()
{
    Super::BeginPlay();

    FPhysicsBody* body = GetWorld()->PhysicsScene.SceneSolver.GetBody(this->Collider);
    if (body)
    {
        body->OnOverlap.AddDynamic(this, &APlatform::OnOverlap);
    }

    //this->LuaComp->CallLuaFunction("OnGenerated", GetActorLocation());

}

void APlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void APlatform::OnOverlap(const FPhysicsBody& result)
{
    if (result.Component->GetOwner()->IsA<class APlayer>())
    {
        if (bIsStepped == false)
        {
            bIsStepped = true;
            GenerateNextLevel();
        }
    }
}

void APlatform::GenerateNextLevel()
{
    APlatform* NewBlock = GetWorld()->SpawnActor<APlatform>();
    NewBlock->Level = Level + 1;
    NewBlock->SetActorLocation(GetActorLocation());
    NewBlock->LuaComp->CallLuaFunction("OnGenerated", GetActorLocation());

    // 10단계마다 원숭이 생성
    if (Level % 6 == 0)
    {
        NewBlock = GetWorld()->SpawnActor<APlatform>();
        NewBlock->Level = Level + 1;
        NewBlock->SetActorLocation(GetActorLocation());

        NewBlock->LuaComp->CallLuaFunction("OnGenerated", GetActorLocation());
        NewBlock->bIsStepped = true;
        ARiceMonkey* SsalSoong = GetWorld()->SpawnActor<ARiceMonkey>();
        SsalSoong->SetActorLocation(NewBlock->GetActorLocation() + FVector(0,0,2));
        //ARiceMonkey
    }
}
