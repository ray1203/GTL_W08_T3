#include "RiceMonkey.h"
#include "Engine/FLoaderOBJ.h" 
#include "World/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Lua/LuaScriptComponent.h"

ARiceMonkey::ARiceMonkey()
{
}

void ARiceMonkey::PostSpawn()
{
    Super::PostSpawn();
    MeshComponent = AddComponent<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/12958_Spider_Monkey_v1_l2.obj"));
    MeshComponent->SetRelativeScale3D(FVector(0.05, 0.05, 0.05));
    RootComponent = MeshComponent;

    Collider = AddComponent<UBoxComponent>(TEXT("Box"));
    Collider->SetupAttachment(MeshComponent);
    Collider->bIsSimulatingPhysics = false;

    LuaComp = AddComponent<ULuaScriptComponent>(TEXT("LuaScriptComponent"));
    LuaComp->SetScriptPath(TEXT("Monkey"));
}

UObject* ARiceMonkey::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewActor->MeshComponent = NewActor->GetComponentByClass<UStaticMeshComponent>();
    NewActor->Collider = NewActor->GetComponentByClass<UBoxComponent>();
    NewActor->LuaComp = NewActor->GetComponentByClass<ULuaScriptComponent>();
    NewActor->LuaComp->SetScriptPath(TEXT("Monkey"));
    return NewActor;
}

void ARiceMonkey::BeginPlay()
{
    Super::BeginPlay();

    FPhysicsBody* body = GetWorld()->PhysicsScene.SceneSolver.GetBody(this->Collider);
    if (body)
    {
        body->OnOverlap.AddDynamic(this, &ARiceMonkey::OnOverlap);
    }

    this->LuaComp->CallLuaFunction("OnGenerated", GetActorLocation());
}

void ARiceMonkey::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ARiceMonkey::OnOverlap(const FPhysicsBody& result)
{
    this->LuaComp->CallLuaFunction("OnOverlap", result.Component->GetOwner());
}

