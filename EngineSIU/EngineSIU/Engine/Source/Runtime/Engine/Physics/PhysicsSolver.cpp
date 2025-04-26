#include "PhysicsSolver.h"

#include "UObject/Casts.h"
#include "Components/Shapes/BoxComponent.h"
#include "Components/Shapes/CapsuleComponent.h"
#include "Components/Shapes/SphereComponent.h"
#include "Engine/World/World.h"

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
        if (Body.bIsSimulatingPhysics)
        {
            Body.Acceleration = FVector(0, 0, -9.8f * 2); // 중력 가속도
        }
    }
}

void FPhysicsSolver::Integrate(float DeltaTime)
{
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        if (Body.bIsSimulatingPhysics)
        {
            Body.Velocity += Body.Acceleration * DeltaTime;
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
}

void FPhysicsSolver::UpdateTransforms()
{
}

void FPhysicsSolver::AddBody(UShapeComponent* Component)
{
    FPhysicsBody Body(Component);
    if (UBoxComponent* BoxComp = Cast<UBoxComponent>(Component))
    {
        FCollisionShape Box;
        Box.ShapeType = ECollisionShape::Box;
        FVector Scale = BoxComp->GetRelativeScale3D();
        Box.Box = { Scale.X, Scale.Y, Scale.Z };

        Body.CollisionShape = Box;
    }
    else if (USphereComponent* SphereComp = Cast<USphereComponent>(Component))
    {
        // SphereComponent에 대한 처리
        FCollisionShape Sphere;
        Sphere.ShapeType = ECollisionShape::Sphere;
        Sphere.Sphere.Radius = SphereComp->GetSphereRadius();

        Body.CollisionShape = Sphere;
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

bool FPhysicsSolver::Overlap(const FCollisionShape& Shape, const FTransform& Transform, TArray<AActor*>& OutOverlaps) const
{
    if (Shape.ShapeType == ECollisionShape::Box)
    {
        //// BoxOverlap 처리
        //for (const FPhysicsBody &Body : SimulatedBodies)
        //{
        //    if (Body.CollisionShape.ShapeType == ECollisionShape::Box)
        //    {
        //        FOrientedBox Target;
        //        Target.AxisX = Transform.Rotation.GetForwardVector();
        //        Target.AxisY = Transform.Rotation.GetRightVector();
        //        Target.AxisZ = Transform.Rotation.GetUpVector();
        //        Target.Center = Transform.Translation;
        //        Target.ExtentX = Shape.Box.HalfExtentX;
        //        Target.ExtentY = Shape.Box.HalfExtentY;
        //        Target.ExtentZ = Shape.Box.HalfExtentZ;

        //        JungleCollision::Intersects(Target, Body.CollisionShape.Box);
        //    }
        //}
    }
    else if (Shape.ShapeType == ECollisionShape::Sphere)
    {
        FSphere Source;
        Source.Center = Transform.Translation;
        Source.Radius = Shape.Sphere.Radius;

        for (const FPhysicsBody& Body : SimulatedBodies)
        {
            if (Body.CollisionShape.ShapeType == ECollisionShape::Sphere)
            {
                FSphere Target;
                Target.Center = Body.Transform.Translation;
                Target.Radius = Body.CollisionShape.Sphere.Radius;
                if (JungleCollision::Intersects(Source, Target))
                {
                    OutOverlaps.Add(Body.Component->GetOwner());
                }
            }
        }
    }
    else if (Shape.ShapeType == ECollisionShape::Capsule)
    {
        // CapsuleOverlap 처리
    }
    if (OutOverlaps.Num() > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
