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
