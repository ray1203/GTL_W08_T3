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
    AddQueue.Empty();
    RemoveQueue.Empty();
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
void FPhysicsScene::TickPhysScene(float DeltaSeconds)
{
    DeltaTime = DeltaSeconds;

    // tick에서 add/remove 큐 먼저 처리
    ProcessBodyQueues();

    SceneSolver.UpdateBodyFromComponent();
    SceneSolver.AdvanceAndDispatch(DeltaSeconds);
}

void FPhysicsScene::AddRigidBody(UShapeComponent* Component)
{
    // 바로 추가하지 않고 큐에 등록
    if (Component && !AddQueue.Contains(Component) && !RegisteredBodies.Contains(Component))
    {
        AddQueue.Add(Component);
    }
}

void FPhysicsScene::RemoveRigidBody(UShapeComponent* Component)
{
    // 바로 제거하지 않고 큐에 등록
    if (Component && !RemoveQueue.Contains(Component) && RegisteredBodies.Contains(Component))
    {
        RemoveQueue.Add(Component);
    }
}

bool FPhysicsScene::GetOverlappings(UShapeComponent* Shape, TArray<FOverlapInfo>& OutOverlaps)
{
    const FPhysicsBody* Body = SceneSolver.GetBody(Shape);
    TArray<FPhysicsBody*> OverlappingBodies;
    if (Body)
    {
        if (SceneSolver.GetOverlappingBodies(*Body, OverlappingBodies))
        {
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
        }
    }
    return false;
}

void FPhysicsScene::ProcessBodyQueues()
{
    // Remove 먼저 처리 (등록되어 있을 때만)
    for (UShapeComponent* Comp : RemoveQueue)
    {
        if (Comp && RegisteredBodies.Contains(Comp))
        {
            RegisteredBodies.Remove(Comp);
            SceneSolver.RemoveBody(Comp);
        }
    }
    RemoveQueue.Empty();

    // Add 처리 (등록되어 있지 않을 때만)
    for (UShapeComponent* Comp : AddQueue)
    {
        if (Comp && !RegisteredBodies.Contains(Comp))
        {
            RegisteredBodies.Add(Comp);
            SceneSolver.AddBody(Comp);
        }
    }
    AddQueue.Empty();
}
