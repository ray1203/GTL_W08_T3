#include "PhysicsSolver.h"

#include "UObject/Casts.h"
#include "Components/Shapes/BoxComponent.h"
#include "Components/Shapes/CapsuleComponent.h"
#include "Components/Shapes/SphereComponent.h"
#include "Engine/World/World.h"
#include "Classes/GameFramework/Actor.h"
#include "Classes/Components/ProjectileMovementComponent.h"
#include "Math/JungleCollision.h"
#include "Delegates/DelegateCombination.h"

// [참고: Unreal Engine Transform/Collision 처리 구조](https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Engine/Private/PhysicsEngine/BodyInstance.cpp)

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
        else
        {
            Body.Velocity = FVector::ZeroVector;
            Body.Acceleration = FVector::ZeroVector;
        }
    }
}

void FPhysicsSolver::AdvanceAndDispatch(float DeltaTime)
{
    ApplyForces();
    Integrate(DeltaTime);
    HandleOverlaps();
    HandleCollisions();
    UpdateTransforms();
}

void FPhysicsSolver::ApplyForces()
{
    static int count = 0;
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        if (Body.bGrounded)
        {
            // 아래로 가는 중일 때만 z속도를 0으로 클램프 (점프는 유지)
            if (Body.Velocity.Z < 0.f)
                Body.Velocity.Z = 0.f;

            // 바닥에 있을 때만 z방향 가속도를 0으로 (중력 등)
            if (Body.Acceleration.Z < 0.f)
                Body.Acceleration.Z = 0.f;

            Body.Acceleration.X -= Friction * Body.Velocity.X;
            Body.Acceleration.Y -= Friction * Body.Velocity.Y;
            //UE_LOG(ELogLevel::Error, "FPhysicsScene::bGrounded[%d] : %s", count++, *Body.Component->StaticClass()->GetName());
        }
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

void FPhysicsSolver::HandleOverlaps()
{
    CachedOverlaps.Empty();
    int32 NumBodies = SimulatedBodies.Num();

    // 1. bGrounded를 먼저 false로 초기화
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        Body.bGrounded = false;
    }

    // 2. 오버랩 판정 및 grounded 처리
    for (int32 i = 0; i < NumBodies; ++i)
    {
        FPhysicsBody& BodyA = SimulatedBodies[i];
        for (int32 j = i + 1; j < NumBodies; ++j)
        {
            FPhysicsBody& BodyB = SimulatedBodies[j];
            if (IsOverlapping(BodyA, BodyB))
            {
                CachedOverlaps.Add(TPair<int32, int32>(i, j));
                // 두 바디가 겹칠 때 "윗면"에 있는 쪽만 grounded
                if (BodyA.Transform.Translation.Z > BodyB.Transform.Translation.Z + KINDA_SMALL_NUMBER)
                    BodyA.bGrounded = true;
                if (BodyB.Transform.Translation.Z > BodyA.Transform.Translation.Z + KINDA_SMALL_NUMBER)
                    BodyB.bGrounded = true;

                BodyA.OnOverlap.Broadcast(BodyB);
                BodyB.OnOverlap.Broadcast(BodyA);
            }
        }
    }
    // 바닥(z<=0) 체크도 같이 처리
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        if (Body.Transform.Translation.Z <= 0.f)
            Body.bGrounded = true;
    }
}

// --- Transform에서 실제 월드 shape 추출 람다들
static FSphere GetWorldSphere(const FCollisionShape& Shape, const FTransform& T)
{
    float Scale = T.Scale3D.GetMin();
    return FSphere{ T.Translation, Shape.Sphere.Radius * Scale };
}
static FOrientedBox GetWorldOrientedBox(const FCollisionShape& Shape, const FTransform& T)
{
    FVector Extent = Shape.GetExtent();
    FVector Scale = T.Scale3D;
    FVector WorldExtent = Extent * Scale;
    FQuat Q = T.Rotation;
    FOrientedBox OBB;
    OBB.Center = T.Translation;
    OBB.AxisX = Q.RotateVector(FVector(1, 0, 0));
    OBB.AxisY = Q.RotateVector(FVector(0, 1, 0));
    OBB.AxisZ = Q.RotateVector(FVector(0, 0, 1));
    OBB.ExtentX = WorldExtent.X;
    OBB.ExtentY = WorldExtent.Y;
    OBB.ExtentZ = WorldExtent.Z;
    return OBB;
}
static FCapsule GetWorldCapsule(const FCollisionShape& Shape, const FTransform& T)
{
    FVector Scale = T.Scale3D;
    float HalfHeight = Shape.Capsule.HalfHeight * Scale.Z;
    float Radius = Shape.Capsule.Radius * FMath::Max(Scale.X, Scale.Y);
    FVector Center = T.Translation;
    FQuat Q = T.Rotation;
    FVector Axis = Q.RotateVector(FVector(0, 0, 1));
    FCapsule Capsule;
    Capsule.A = Center - Axis * HalfHeight;
    Capsule.B = Center + Axis * HalfHeight;
    Capsule.Radius = Radius;
    return Capsule;
}

// --- Contact Info 계산 (shape/type별 침투량, 노멀, 상대속도 계산)
FPhysicsSolver::FContactInfo FPhysicsSolver::ComputeContactInfo(const FPhysicsBody& BodyA, const FPhysicsBody& BodyB)
{
    const FCollisionShape& ShapeA = BodyA.CollisionShape;
    const FCollisionShape& ShapeB = BodyB.CollisionShape;
    const FTransform& TransformA = BodyA.Transform;
    const FTransform& TransformB = BodyB.Transform;

    // Sphere - Sphere
    if (ShapeA.IsSphere() && ShapeB.IsSphere())
    {
        FSphere SphereA = GetWorldSphere(ShapeA, TransformA);
        FSphere SphereB = GetWorldSphere(ShapeB, TransformB);
        FVector Delta = SphereB.Center - SphereA.Center;
        float Dist = Delta.Length();
        if (Dist < KINDA_SMALL_NUMBER) return {};
        FVector Normal = Delta / Dist;
        float Penetration = SphereA.Radius + SphereB.Radius - Dist;
        float RelativeVelocity = FVector::DotProduct(BodyB.Velocity - BodyA.Velocity, Normal);
        if (Penetration > 0.f)
            return { Normal, Penetration, RelativeVelocity, true };
        return {};
    }

    // Box - Box (OrientedBox)
    if (ShapeA.IsBox() && ShapeB.IsBox())
    {
        FOrientedBox OBB_A = GetWorldOrientedBox(ShapeA, TransformA);
        FOrientedBox OBB_B = GetWorldOrientedBox(ShapeB, TransformB);
        JungleCollision::FBoxContactResult Result;
        if (JungleCollision::Intersects(OBB_A, OBB_B, &Result) && Result.bValid)
        {
            float RelVel = FVector::DotProduct(BodyB.Velocity - BodyA.Velocity, Result.Normal);
            return { Result.Normal, Result.Penetration, RelVel, true };
        }
        return {};
    }

    // Box - Sphere
    if (ShapeA.IsBox() && ShapeB.IsSphere())
    {
        FOrientedBox OBB_A = GetWorldOrientedBox(ShapeA, TransformA);
        FSphere SphereB = GetWorldSphere(ShapeB, TransformB);
        JungleCollision::FBoxSphereContactResult Result;
        if (JungleCollision::Intersects(OBB_A, SphereB, &Result) && Result.bValid)
        {
            float RelVel = FVector::DotProduct(BodyB.Velocity - BodyA.Velocity, Result.Normal);
            return { Result.Normal, Result.Penetration, RelVel, true };
        }
        return {};
    }
    if (ShapeA.IsSphere() && ShapeB.IsBox())
    {
        FContactInfo Info = ComputeContactInfo(BodyB, BodyA);
        if (Info.bValid)
        {
            Info.Normal *= -1.f;
            Info.RelativeVelocity *= -1.f;
        }
        return Info;
    }

    // Capsule - Capsule
    if (ShapeA.IsCapsule() && ShapeB.IsCapsule())
    {
        FCapsule CapsuleA = GetWorldCapsule(ShapeA, TransformA);
        FCapsule CapsuleB = GetWorldCapsule(ShapeB, TransformB);
        JungleCollision::FCapsuleContactResult Result;
        if (JungleCollision::Intersects(CapsuleA, CapsuleB, &Result) && Result.bValid)
        {
            float RelVel = FVector::DotProduct(BodyB.Velocity - BodyA.Velocity, Result.Normal);
            return { Result.Normal, Result.Penetration, RelVel, true };
        }
        return {};
    }

    // Capsule - Sphere
    if (ShapeA.IsCapsule() && ShapeB.IsSphere())
    {
        FCapsule CapsuleA = GetWorldCapsule(ShapeA, TransformA);
        FSphere SphereB = GetWorldSphere(ShapeB, TransformB);
        JungleCollision::FCapsuleSphereContactResult Result;
        if (JungleCollision::Intersects(CapsuleA, SphereB, &Result) && Result.bValid)
        {
            float RelVel = FVector::DotProduct(BodyB.Velocity - BodyA.Velocity, Result.Normal);
            return { Result.Normal, Result.Penetration, RelVel, true };
        }
        return {};
    }
    if (ShapeA.IsSphere() && ShapeB.IsCapsule())
    {
        FContactInfo Info = ComputeContactInfo(BodyB, BodyA);
        if (Info.bValid)
        {
            Info.Normal *= -1.f;
            Info.RelativeVelocity *= -1.f;
        }
        return Info;
    }

    // Capsule - Box
    if (ShapeA.IsCapsule() && ShapeB.IsBox())
    {
        FCapsule CapsuleA = GetWorldCapsule(ShapeA, TransformA);
        FOrientedBox OBB_B = GetWorldOrientedBox(ShapeB, TransformB);
        JungleCollision::FCapsuleBoxContactResult Result;
        if (JungleCollision::Intersects(CapsuleA, OBB_B, &Result) && Result.bValid)
        {
            float RelVel = FVector::DotProduct(BodyB.Velocity - BodyA.Velocity, Result.Normal);
            return { Result.Normal, Result.Penetration, RelVel, true };
        }
        return {};
    }
    if (ShapeA.IsBox() && ShapeB.IsCapsule())
    {
        FContactInfo Info = ComputeContactInfo(BodyB, BodyA);
        if (Info.bValid)
        {
            Info.Normal *= -1.f;
            Info.RelativeVelocity *= -1.f;
        }
        return Info;
    }

    // 기타 조합 필요시 추가
    return {};
}

void FPhysicsSolver::HandleCollisions()
{
    for (FPhysicsBody& Body : SimulatedBodies)
    {
        if (!Body.bIsSimulatingPhysics) continue;

        if (Body.Transform.Translation.Z < 0.f)
        {
            Body.Transform.Translation.Z = 0.f;
            if (Body.bStickToGround && Body.bGrounded)
            {
                if (Body.Velocity.Z < 0.f)
                {
                    Body.Velocity.Z = 0.f;
                }
            }
            else
            {
                Body.Velocity.Z *= -Body.Restitution;
            }
            if (std::abs(Body.Velocity.LengthSquared()) < RestitutionThreshold * RestitutionThreshold)
            {
                Body.Velocity = FVector::ZeroVector;
            }
            // bGrounded는 여기서 처리하지 않음!
        }
    }
    // shape별 penetration/impulse 처리 (bGrounded 변경 X, 기존과 동일)
    for (const auto& OverlapPair : CachedOverlaps)
    {
        int32 i = OverlapPair.Key;
        int32 j = OverlapPair.Value;
        FPhysicsBody& BodyA = SimulatedBodies[i];
        FPhysicsBody& BodyB = SimulatedBodies[j];

        if (!BodyA.bIsSimulatingPhysics && !BodyB.bIsSimulatingPhysics)
            continue;
        if (!BodyA.bBlock || !BodyB.bBlock)
            continue;

        FContactInfo Contact = ComputeContactInfo(BodyA, BodyB);
        if (!Contact.bValid) continue;

        const FVector& Normal = Contact.Normal;
        float Penetration = Contact.Penetration;
        float RelativeVelocity = Contact.RelativeVelocity;
        float Restitution = FMath::Max(BodyA.Restitution, BodyB.Restitution);

        bool bAWasGrounded = BodyA.bGrounded;
        bool bBWasGrounded = BodyB.bGrounded;

        if (BodyA.bIsSimulatingPhysics && BodyB.bIsSimulatingPhysics)
        {
            if (RelativeVelocity < 0.f)
            {
                float InvMassA = (BodyA.Mass > 0.f) ? (1.f / BodyA.Mass) : 0.f;
                float InvMassB = (BodyB.Mass > 0.f) ? (1.f / BodyB.Mass) : 0.f;
                float Impulse = -(1.f + Restitution) * RelativeVelocity / (InvMassA + InvMassB);

                FVector ImpulseVec = Impulse * Normal;

                BodyA.Velocity -= ImpulseVec * InvMassA;
                BodyB.Velocity += ImpulseVec * InvMassB;
            }
            float Correction = Penetration * 0.5f;
            BodyA.Transform.Translation -= Normal * Correction;
            BodyB.Transform.Translation += Normal * Correction;
        }
        else if (BodyA.bIsSimulatingPhysics)
        {
            if (RelativeVelocity < 0.f)
            {
                float InvMassA = (BodyA.Mass > 0.f) ? (1.f / BodyA.Mass) : 0.f;
                float Impulse = -(1.f + Restitution) * RelativeVelocity / InvMassA;
                FVector ImpulseVec = Impulse * Normal;
                BodyA.Velocity -= ImpulseVec * InvMassA;
            }
            BodyA.Transform.Translation -= Normal * Penetration;
        }
        else if (BodyB.bIsSimulatingPhysics)
        {
            if (RelativeVelocity < 0.f)
            {
                float InvMassB = (BodyB.Mass > 0.f) ? (1.f / BodyB.Mass) : 0.f;
                float Impulse = -(1.f + Restitution) * RelativeVelocity / InvMassB;
                FVector ImpulseVec = Impulse * Normal;
                BodyB.Velocity += ImpulseVec * InvMassB;
            }
            BodyB.Transform.Translation += Normal * Penetration;
        }

        // StickToGround가 true && grounded였던 바디만 Normal 방향 velocity를 0으로!
        if (bAWasGrounded && BodyA.bStickToGround)
        {
            //float vDotN = FVector::DotProduct(BodyA.Velocity, Normal);
            float vDotN = FVector::DotProduct(BodyA.Velocity, FVector::UpVector);
            BodyA.Velocity -= Normal * vDotN;
        }
        if (bBWasGrounded && BodyB.bStickToGround)
        {
            //float vDotN = FVector::DotProduct(BodyB.Velocity, Normal);
            float vDotN = FVector::DotProduct(BodyB.Velocity, FVector::UpVector);
            BodyB.Velocity -= Normal * vDotN;
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
        Body.CollisionShape.SetExtent(Box.GetExtent());
    }
    else if (USphereComponent* SphereComp = Cast<USphereComponent>(Component))
    {
        FCollisionShape Sphere;
        Sphere.ShapeType = ECollisionShape::Sphere;
        Sphere.Sphere.Radius = SphereComp->GetSphereRadius();

        Body.CollisionShape = Sphere;
        UE_LOG(ELogLevel::Error, "FPhysicsSolver::AddBody[%d] : Sphere %f", count++, Sphere.Sphere.Radius);
        Body.CollisionShape.SetExtent(Sphere.GetExtent());

    }
    else if (UCapsuleComponent* CapsuleComp = Cast<UCapsuleComponent>(Component))
    {
        FCollisionShape Capsule;
        Capsule.ShapeType = ECollisionShape::Capsule;
        Capsule.Capsule.Radius = CapsuleComp->GetCapsuleRadius();
        Capsule.Capsule.HalfHeight = CapsuleComp->GetCapsuleHalfHeight();

        Body.CollisionShape = Capsule;
        UE_LOG(ELogLevel::Error, "FPhysicsSolver::AddBody[%d] : Capsule %f, %f", count++, Capsule.Capsule.Radius, Capsule.Capsule.HalfHeight);
        Body.CollisionShape.SetExtent(Capsule.GetExtent());
    }
    else
    {
        return;
    }
    Body.Transform = Component->GetWorldTransform();
    Body.bBlock = Component->bBlockComponent;
    Body.bIsSimulatingPhysics = Component->bIsSimulatingPhysics;
    Body.Mass = Component->Mass;
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

FPhysicsBody* FPhysicsSolver::GetBody(const UShapeComponent* Component)
{
    for (FPhysicsBody& Body : SimulatedBodies)
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

bool FPhysicsSolver::GetOverlappingBodies(const FPhysicsBody& Body, TArray<FPhysicsBody*>& OverlappingBodies)
{
    for (const auto& OverlapPair : CachedOverlaps)
    {
        int32 i = OverlapPair.Key;
        int32 j = OverlapPair.Value;
        FPhysicsBody& BodyA = SimulatedBodies[i];
        FPhysicsBody& BodyB = SimulatedBodies[j];
        if (BodyA == Body)
        {
            OverlappingBodies.Add(&BodyB);
        }
        else if (BodyB == Body)
        {
            OverlappingBodies.Add(&BodyA);
        }
    }
    if (OverlappingBodies.Num() > 0)
    {
        return true;
    }
    return false;
}

bool FPhysicsSolver::IsOverlapping(const FPhysicsBody& BodyA, const FPhysicsBody& BodyB)
{
    const FCollisionShape& ShapeA = BodyA.CollisionShape;
    const FCollisionShape& ShapeB = BodyB.CollisionShape;
    const FTransform& TransformA = BodyA.Transform;
    const FTransform& TransformB = BodyB.Transform;

    if (ShapeA.IsSphere() && ShapeB.IsSphere())
    {
        FSphere SphereA = GetWorldSphere(ShapeA, TransformA);
        FSphere SphereB = GetWorldSphere(ShapeB, TransformB);
        return JungleCollision::Intersects(SphereA, SphereB);
    }
    if (ShapeA.IsBox() && ShapeB.IsBox())
    {
        FOrientedBox OBB_A = GetWorldOrientedBox(ShapeA, TransformA);
        FOrientedBox OBB_B = GetWorldOrientedBox(ShapeB, TransformB);
        return JungleCollision::Intersects(OBB_A, OBB_B);
    }
    if (ShapeA.IsBox() && ShapeB.IsSphere())
    {
        FOrientedBox OBB_A = GetWorldOrientedBox(ShapeA, TransformA);
        FSphere SphereB = GetWorldSphere(ShapeB, TransformB);
        return JungleCollision::Intersects(OBB_A, SphereB);
    }
    if (ShapeA.IsSphere() && ShapeB.IsBox())
    {
        FSphere SphereA = GetWorldSphere(ShapeA, TransformA);
        FOrientedBox OBB_B = GetWorldOrientedBox(ShapeB, TransformB);
        return JungleCollision::Intersects(SphereA, OBB_B);
    }
    if (ShapeA.IsCapsule() && ShapeB.IsCapsule())
    {
        FCapsule CapsuleA = GetWorldCapsule(ShapeA, TransformA);
        FCapsule CapsuleB = GetWorldCapsule(ShapeB, TransformB);
        return JungleCollision::Intersects(CapsuleA, CapsuleB);
    }
    if (ShapeA.IsCapsule() && ShapeB.IsSphere())
    {
        FCapsule CapsuleA = GetWorldCapsule(ShapeA, TransformA);
        FSphere SphereB = GetWorldSphere(ShapeB, TransformB);
        return JungleCollision::Intersects(CapsuleA, SphereB);
    }
    if (ShapeA.IsSphere() && ShapeB.IsCapsule())
    {
        FSphere SphereA = GetWorldSphere(ShapeA, TransformA);
        FCapsule CapsuleB = GetWorldCapsule(ShapeB, TransformB);
        return JungleCollision::Intersects(SphereA, CapsuleB);
    }
    if (ShapeA.IsCapsule() && ShapeB.IsBox())
    {
        FCapsule CapsuleA = GetWorldCapsule(ShapeA, TransformA);
        FOrientedBox OBB_B = GetWorldOrientedBox(ShapeB, TransformB);
        return JungleCollision::Intersects(CapsuleA, OBB_B);
    }
    if (ShapeA.IsBox() && ShapeB.IsCapsule())
    {
        FOrientedBox OBB_A = GetWorldOrientedBox(ShapeA, TransformA);
        FCapsule CapsuleB = GetWorldCapsule(ShapeB, TransformB);
        return JungleCollision::Intersects(OBB_A, CapsuleB);
    }
    // 기타 shape 조합 필요시 추가
    return false;
}
