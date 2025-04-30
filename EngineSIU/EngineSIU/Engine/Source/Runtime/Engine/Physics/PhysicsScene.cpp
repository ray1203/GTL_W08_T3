#include "PhysicsScene.h"
#include "Components/Shapes/ShapeComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Classes/GameFramework/Actor.h"
FPhysicsScene::FPhysicsScene()
{
}

FPhysicsScene::~FPhysicsScene()
{
    RegisteredBodies.Empty();
}

void FPhysicsScene::SyncBodies()
{
    // Solver 결과를 각 Actor/Component에 반영
    for (UShapeComponent* Comp : RegisteredBodies)
    {
        if (Comp)
        {
            // 예시: Solver에서 Comp에 대한 위치/회전 정보를 가져와 반영
            FTransform SimulatedTransform;
            if (SceneSolver.GetSimulatedTransform(Comp, SimulatedTransform))
            {
                Comp->GetOwner()->SetActorLocation(SimulatedTransform.Translation);
                Comp->GetOwner()->SetActorRotation(SimulatedTransform.Rotation.ToRotator());
                // 강체인데 scale을 바꿀일이 있나...?
                //Comp->GetOwner()->SetActorScale(SimulatedTransform.Scale3D);
                const FPhysicsBody* Body = SceneSolver.GetBody(Comp);
                if (UProjectileMovementComponent* ProjComp = Comp->GetOwner()->GetComponentByClass<UProjectileMovementComponent>())
                {
                    ProjComp->SetVelocity(Body->Velocity);
                }
                Comp->bGrounded = Body->bGrounded;
            }

            if (Comp->bGenerateOverlapEvents)
            {
                // Delegate 추가하기
            }

        }
    }
}

void FPhysicsScene::Clear()
{
    RegisteredBodies.Empty();
    SceneSolver.Clear();
}

void FPhysicsScene::TickPhysScene(float DeltaSeconds)
{
    DeltaTime = DeltaSeconds;
    SceneSolver.UpdateBodyFromComponent();
    SceneSolver.AdvanceAndDispatch(DeltaSeconds);
}

void FPhysicsScene::AddRigidBody(UShapeComponent* Component)
{
    static uint32 num = 0;
    if (Component && !RegisteredBodies.Contains(Component))
    {
        //UE_LOG(ELogLevel::Error, "FPhysicsScene::AddRigidBody[%d] : %s", num++,*Component->StaticClass()->GetName());
        RegisteredBodies.Add(Component);
        SceneSolver.AddBody(Component);
    }
}

void FPhysicsScene::RemoveRigidBody(UShapeComponent* Component)
{
    if (Component && RegisteredBodies.Contains(Component))
    {
        RegisteredBodies.Remove(Component);
        SceneSolver.RemoveBody(Component);
    }
}


bool FPhysicsScene::GetOverlappings(UShapeComponent* Shape, TArray<FOverlapInfo>& OutOverlaps)
{
    const FPhysicsBody* Body = SceneSolver.GetBody(Shape);
    TArray<FPhysicsBody*> OverlappingBodies;
    SceneSolver.GetOverlappingBodies(*Body, OverlappingBodies);

    for (auto& Body : OverlappingBodies)
    {
        FOverlapInfo Info;
        Info.bFromSweep = false;
        Info.OverlapInfo.Component = Body->Component;
        OutOverlaps.Add(Info);
    }

    if (OutOverlaps.Num() > 0)
    {
        return true;
    }
    return false;
}

bool FPhysicsScene::RayTraceSingle(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
    bool bHitSomething = false;
    float ClosestHitDistance = FLT_MAX;
    UShapeComponent* ClosestComponent = nullptr;
    FVector ClosestHitPoint, ClosestHitNormal;

    FVector RayOrigin = Start;
    FVector RayDir = (End - Start);
    float RayLength = RayDir.Length();
    if (RayLength <= KINDA_SMALL_NUMBER)
        return false;
    RayDir /= RayLength; // 정규화

    // PhysicsScene->RegisteredBodies 순회
    for (UShapeComponent* ShapeComp : RegisteredBodies)
    {
        float HitDistance = 0.f;
        FVector LocalHitPoint, LocalNormal;

        // 실제로는 CheckRayIntersection가 히트시 HitDistance, HitPoint, Normal을 채워줄 수 있게 설계 필요
        int HitType = ShapeComp->CheckRayIntersection(RayOrigin, RayDir, HitDistance);
        // HitType: 0=NoHit, 1=Blocking, 2=Overlap 등으로 가정

        if (HitType == 1) // Blocking만 처리
        {
            if (HitDistance >= 0.f && HitDistance <= RayLength && HitDistance < ClosestHitDistance)
            {
                ClosestHitDistance = HitDistance;
                ClosestComponent = ShapeComp;
                ClosestHitPoint = RayOrigin + RayDir * HitDistance;

                // Normal 계산 (캡슐/스피어는 중심방향, 박스는 표면 법선 등)
                bHitSomething = true;
            }
        }
    }

    if (bHitSomething && ClosestComponent)
    {
        OutHit.Time = ClosestHitDistance / RayLength;
        OutHit.Distance = ClosestHitDistance;
        OutHit.Location = ClosestComponent->GetWorldLocation();
        OutHit.ImpactPoint = ClosestHitPoint;
        OutHit.Normal = ClosestHitNormal;
        OutHit.TraceStart = Start;
        OutHit.TraceEnd = End;
        OutHit.bBlockingHit = true;
        OutHit.Component = ClosestComponent;
        // OutHit.Owner = ClosestComponent->GetOwner(); // 필요시

        return true;
    }
    return false;
}
