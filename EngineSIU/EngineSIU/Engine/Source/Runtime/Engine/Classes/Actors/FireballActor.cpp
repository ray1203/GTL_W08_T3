#include "FireballActor.h"
#include "Engine/FLoaderOBJ.h"

#include "Components/Light/PointLightComponent.h"

#include "Components/ProjectileMovementComponent.h"

#include "Components/SphereComp.h"

AFireballActor::AFireballActor()
{
    FManagerOBJ::CreateStaticMesh("Contents/Sphere.obj");


    SphereComp = AddComponent<USphereComp>(TEXT("SphereComp"));
    
    SphereComp->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Sphere.obj"));
  
    PointLightComponent = AddComponent<UPointLightComponent>(TEXT("PointLightComponent"));
    
    PointLightComponent->SetLightColor(FLinearColor::Red);
    
    ProjectileMovementComponent = AddComponent<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    PointLightComponent->AttachToComponent(RootComponent);

    ProjectileMovementComponent->SetVelocity(FVector(100, 0, 0));
    ProjectileMovementComponent->SetInitialSpeed(100);
    ProjectileMovementComponent->SetMaxSpeed(100);
}

void AFireballActor::BeginPlay()
{
}
