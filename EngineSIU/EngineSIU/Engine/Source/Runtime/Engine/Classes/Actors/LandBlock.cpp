#include "LandBlock.h"
#include "Engine/FLoaderOBJ.h" 
#include "World/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Lua/LuaScriptComponent.h"
#include "Actors/Player.h"

ALandBlock::ALandBlock()
{
}

void ALandBlock::PostSpawn()
{
    Super::PostSpawn();
    MeshComponent = AddComponent<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/helloBlender.obj"));
    RootComponent = MeshComponent;

    Collider = AddComponent<UBoxComponent>(TEXT("Box"));
    Collider->SetupAttachment(MeshComponent);
    Collider->bIsSimulatingPhysics = false;

    LuaComp = AddComponent<ULuaScriptComponent>(TEXT("LuaScriptComponent"));
    LuaComp->SetScriptPath(TEXT("LandGenerate"));
}

UObject* ALandBlock::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewActor->MeshComponent = NewActor->GetComponentByClass<UStaticMeshComponent>();
    NewActor->Collider = NewActor->GetComponentByClass<UBoxComponent>();
    NewActor->LuaComp = NewActor->GetComponentByClass<ULuaScriptComponent>();
    NewActor->bIsStepped = false;
    NewActor->Level = Level;

    return NewActor;
}

void ALandBlock::BeginPlay()
{
    Super::BeginPlay();

    FPhysicsBody* body = GetWorld()->PhysicsScene.SceneSolver.GetBody(this->Collider);
    if (body)
    {
        body->OnOverlap.AddDynamic(this, &ALandBlock::OnOverlap);
    }

    this->LuaComp->CallLuaFunction("OnGenerated", GetActorLocation());

}

void ALandBlock::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ALandBlock::OnOverlap(const FPhysicsBody& result)
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

void ALandBlock::GenerateNextLevel()
{
    ALandBlock* NewBlock = GetWorld()->DuplicateActor<ALandBlock>(this);
    NewBlock->Level++;
    NewBlock->SetActorLocation(GetActorLocation());
}
