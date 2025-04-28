#include "Cube.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Lua/LuaScriptComponent.h"
#include "Components/Shapes/BoxComponent.h"
#include "Engine/FLoaderOBJ.h"

#include "GameFramework/Actor.h"

ACube::ACube()
{
    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/helloBlender.obj"));
    AddComponent<UBoxComponent>(TEXT("BoxComponent"))->SetupAttachment(StaticMeshComponent);
}

void ACube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
