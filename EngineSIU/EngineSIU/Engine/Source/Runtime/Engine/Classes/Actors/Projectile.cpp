#include "Projectile.h"
#include "World/World.h"
#include <Components/ProjectileMovementComponent.h>
#include "Components/StaticMeshComponent.h"
#include "Components/Shapes/SphereComponent.h"
#include "Engine/FLoaderOBJ.h" 

AProjectile::AProjectile()
{
}

void AProjectile::PostSpawn()
{
    MeshComponent = AddComponent<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetRelativeScale3D(FVector(0.5, 0.5, 0.5));
    MeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Sphere.obj"));

    RootComponent = MeshComponent;

    ProjectileMovementComponent = AddComponent<UProjectileMovementComponent>(TEXT("Proj"));
    ProjectileMovementComponent->SetupAttachment(MeshComponent);

    SphereComponent = AddComponent<USphereComponent>(TEXT("Sphere"));
    SphereComponent->SetupAttachment(MeshComponent);
}

void AProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void AProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    dt += DeltaTime;
    if (dt >= LifeTime)
    {
        GetWorld()->DestroyActor(this);
    }
}

void AProjectile::SetInitialSpeed(const FVector& InitialSpeed)
{
    ProjectileMovementComponent->SetVelocity(InitialSpeed);

}
