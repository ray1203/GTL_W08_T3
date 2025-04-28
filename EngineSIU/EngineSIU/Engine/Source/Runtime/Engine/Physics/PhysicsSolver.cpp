#include "PhysicsSolver.h"

#include "UObject/Casts.h"
#include "Components/Shapes/BoxComponent.h"
#include "Components/Shapes/CapsuleComponent.h"
#include "Components/Shapes/SphereComponent.h"
#include "Engine/World/World.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/ProjectileMovementComponent.h"

FPhysicsSolver::FPhysicsSolver()
{
}

FPhysicsSolver::~FPhysicsSolver()
{
    SimulatedBodies.Empty();
}

void FPhysicsSolver::UpdateBodyFromComponent()
{
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        Body.Transform = Body.Component->GetWorldTransform();
        if (UProjectileMovementComponent* ProjComp = Body.Component->GetOwner()->GetComponentByClass<UProjectileMovementComponent>())
        {
            Body.Velocity = ProjComp->GetVelocity();
            Body.Acceleration = ProjComp->GetAcceleration();
        }
    }
}

void FPhysicsSolver::AdvanceAndDispatch(float DeltaTime)
{
    ApplyForces();
    Integrate(DeltaTime);
    HandleCollisions();
    UpdateTransforms();
}

void FPhysicsSolver::ApplyForces()
{
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        if (Body.Velocity.LengthSquared() != 0 && Body.Transform.Translation.Z < KINDA_SMALL_NUMBER)
        {
            Body.Acceleration.X -= Friction * Body.Velocity.X;
            Body.Acceleration.Y -= Friction * Body.Velocity.Y;
        }
        // UProjectileComponent의 중력가속도를 이용
    }
}

void FPhysicsSolver::Integrate(float DeltaTime)
{
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        if (Body.bIsSimulatingPhysics)
        {
            if (Body.Velocity.Dot(Body.Acceleration) < 0) // 방향이 반대일 때만 clamp
            {
                FVector DeltaV = Body.Acceleration * DeltaTime;
                if (DeltaV.LengthSquared() > Body.Velocity.LengthSquared())
                {
                    Body.Velocity = FVector::ZeroVector;
                }
                else
                {
                    Body.Velocity += DeltaV;
                }
            }
            else
            {
                Body.Velocity += Body.Acceleration * DeltaTime;
            }
            Body.Transform.AddToTranslation(Body.Velocity * DeltaTime);
        }
    }
}

void FPhysicsSolver::HandleCollisions()
{
    // 충돌 감지 및 반응(Velocity) 계산
    // 지금은 오버랩만 하니까 쓰지 않음
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        if (Body.bIsSimulatingPhysics)
        {
            // deltaTime이 너무 커서 실패함
            // step을 줘서 더 작게 만들던가 해야함
            if (Body.Transform.Translation.Z < 0.f)
            {
                Body.Transform.Translation.Z = 0.f;      // 바닥 위로 위치 보정
                Body.Velocity.Z *= -Restitution;         // 반발력 적용 (Restitution 예: 0.9)
                // 임계치 이하의 속도는 0으로 처리
                if (std::abs(Body.Velocity.LengthSquared()) < RestitutionThreshold * RestitutionThreshold)
                {
                    Body.Velocity = FVector::ZeroVector;
                }
            }
        }
    }


    // 2. 두 물체 간의 충돌 처리 (구체-구체 충돌 예시)
    int32 NumBodies = SimulatedBodies.Num();
    for (int32 i = 0; i < NumBodies; ++i)
    {
        FPhysicsBody& BodyA = SimulatedBodies[i];
        if (!BodyA.bIsSimulatingPhysics)
            continue;

        for (int32 j = i + 1; j < NumBodies; ++j)
        {
            FPhysicsBody& BodyB = SimulatedBodies[j];
            if (!BodyB.bIsSimulatingPhysics)
                continue;

            // 구체-구체 충돌 가정
            const FVector PosA = BodyA.Transform.Translation;
            const FVector PosB = BodyB.Transform.Translation;
            float RadiusA = BodyA.CollisionShape.Sphere.Radius;
            float RadiusB = BodyB.CollisionShape.Sphere.Radius;;

            FVector Delta = PosB - PosA;
            float Distance = Delta.Length();
            float Penetration = RadiusA + RadiusB - Distance;

            if (Penetration > 0.f && Distance > KINDA_SMALL_NUMBER)
            {
                FVector Normal = Delta / Distance;
                float RelativeVelocity = FVector::DotProduct(BodyB.Velocity - BodyA.Velocity, Normal);
                if (RelativeVelocity < 0.f)
                {
                    float InvMassA = (BodyA.Mass > 0.f) ? (1.f / BodyA.Mass) : 0.f;
                    float InvMassB = (BodyB.Mass > 0.f) ? (1.f / BodyB.Mass) : 0.f;
                    float Impulse = -(1.f + Restitution) * RelativeVelocity / (InvMassA + InvMassB);

                    FVector ImpulseVec = Impulse * Normal;

                    BodyA.Velocity -= ImpulseVec * InvMassA;
                    BodyB.Velocity += ImpulseVec * InvMassB;
                }

                // 침투 보정
                float Correction = Penetration * 0.5f;
                BodyA.Transform.Translation = (PosA - Normal * Correction);
                BodyB.Transform.Translation = (PosB + Normal * Correction);
            }
        }
    }

}

void FPhysicsSolver::UpdateTransforms()
{
}

void FPhysicsSolver::AddBody(UShapeComponent* Component)
{
    static uint32 count = 0;

    FPhysicsBody Body(Component);
    if (UBoxComponent* BoxComp = Cast<UBoxComponent>(Component))
    {
        FCollisionShape Box;
        Box.ShapeType = ECollisionShape::Box;
        FVector Scale = BoxComp->GetRelativeScale3D();
        Box.Box = { Scale.X, Scale.Y, Scale.Z };
        UE_LOG(ELogLevel::Error, "FPhysicsSolver::AddBody[%d] : Box %f, %f, %f", count++, Scale.X, Scale.Y, Scale.Z);


        Body.CollisionShape = Box;
    }
    else if (USphereComponent* SphereComp = Cast<USphereComponent>(Component))
    {
        // SphereComponent에 대한 처리
        FCollisionShape Sphere;
        Sphere.ShapeType = ECollisionShape::Sphere;
        Sphere.Sphere.Radius = SphereComp->GetSphereRadius();

        Body.CollisionShape = Sphere;
        UE_LOG(ELogLevel::Error, "FPhysicsSolver::AddBody[%d] : Sphere %f", count++, Sphere.Sphere.Radius);


    }
    else if (UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(Component))
    {
        // CapsuleComponent에 대한 처리
        FCollisionShape Capsule;
        Capsule.ShapeType = ECollisionShape::Capsule;
        Capsule.Capsule.Radius = CapsuleComp->GetCapsuleRadius();
        Capsule.Capsule.HalfHeight = CapsuleComp->GetCapsuleHalfHeight();

        Body.CollisionShape = Capsule;
    }
    else
    {
        return;
    }
    Body.Transform = Component->GetWorldTransform();
    SimulatedBodies.Add(Body);
}

void FPhysicsSolver::RemoveBody(UShapeComponent* Component)
{
    if (Component)
    {
        for (auto& Body : SimulatedBodies)
        {
            if (Body.Component == Component)
            {
                SimulatedBodies.Remove(Body);
                break;
            }
        }
    }
}

const FPhysicsBody* FPhysicsSolver::GetBody(UShapeComponent* Component)
{
    for (const FPhysicsBody& Body : SimulatedBodies)
    {
        if (Body.Component == Component)
        {
            return &Body;
        }
    }
    return nullptr;
}

bool FPhysicsSolver::GetSimulatedTransform(UShapeComponent* Component, FTransform& OutTransform) const
{
    if (Component)
    {
        for (auto& Body : SimulatedBodies)
        {
            if (Body.Component == Component)
            {
                OutTransform = Body.Transform;
                return true;
            }
        }
    }
    return false;
}

bool FPhysicsSolver::Raycast(const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
    //// 아주 단순화된 예시 - 실제 구현은 엔진의 SceneQuery를 활용해야 함
    //for (UShapeComponent* Comp : SimulatedParticles)
    //{
    //    if (Comp && Comp->ComponentOverlapComponent(Start, End, FQuat::Identity, Comp->GetCollisionShape()))
    //    {
    //        OutHit.Component = Comp;
    //        OutHit.Location = Start;
    //        return true;
    //    }
    //}
    return false;
}

bool FPhysicsSolver::Overlap(const FPhysicsBody& Body, TArray<FPhysicsBody*> OverlappingBodies)
{
    if (Body.CollisionShape.ShapeType == ECollisionShape::Sphere)
    {
        // 도형과 위치정보를 가진 구조체로 담음
        FSphere Source;
        Source.Center = Body.Transform.Translation;
        Source.Radius = Body.CollisionShape.Sphere.Radius;

        for (FPhysicsBody& Other : SimulatedBodies)
        {
            if (Other == Body)
            {
                continue; // 자기 자신은 제외  
            }
            // 일단 구형끼리 체크
            if (Other.CollisionShape.ShapeType == ECollisionShape::Sphere)
            {
                FSphere Target;
                Target.Center = Other.Transform.Translation;
                Target.Radius = Other.CollisionShape.Sphere.Radius;
                if (JungleCollision::Intersects(Source, Target))
                {
                    OverlappingBodies.Add(&Other);
                }
            }
        }
    }
    if (OverlappingBodies.Num() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
