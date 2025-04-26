#include "PhysicsScene.h"
#include "PhysicsSolver.h"
#include "Components/Shapes/ShapeComponent.h"
#include "Classes/GameFramework/Actor.h"
FPhysicsScene::FPhysicsScene()
{
    SceneSolver = new FPhysicsSolver();
}

FPhysicsScene::~FPhysicsScene()
{
    delete SceneSolver;
    SceneSolver = nullptr;
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
            if (SceneSolver && SceneSolver->GetSimulatedTransform(Comp, SimulatedTransform))
            {
                Comp->GetOwner()->SetActorLocation(SimulatedTransform.Translation);
                Comp->GetOwner()->SetActorRotation(SimulatedTransform.Rotation.ToRotator());
                // 강체인데 scale을 바꿀일이 있나...?
                Comp->GetOwner()->SetActorScale(SimulatedTransform.Scale3D);
            }
        }
    }
}

void FPhysicsScene::TickPhysScene(float DeltaSeconds)
{
    DeltaTime = DeltaSeconds;
    if (SceneSolver)
    {
        SceneSolver->UpdateBodyFromComponent();
        SceneSolver->AdvanceAndDispatch(DeltaSeconds);
    }
}

void FPhysicsScene::AddRigidBody(UShapeComponent* Component)
{
    if (Component && !RegisteredBodies.Contains(Component))
    {
        RegisteredBodies.Add(Component);
        if (SceneSolver)
        {
            SceneSolver->AddBody(Component);
        }
    }
}

void FPhysicsScene::RemoveRigidBody(UShapeComponent* Component)
{
    if (Component && RegisteredBodies.Contains(Component))
    {
        RegisteredBodies.Remove(Component);
        if (SceneSolver)
        {
            SceneSolver->RemoveBody(Component);
        }
    }
}

bool FPhysicsScene::LineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
    //// 간단 예시: Solver의 Raycast 호출
    //if (SceneSolver)
    //{
    //    return SceneSolver->Raycast(Start, End, OutHit);
    //}
    return false;
}

bool FPhysicsScene::Overlap(const UShapeComponent& Shape, const FTransform& Transform, TArray<FOverlapInfo>& OutOverlaps) const
{
    return false;
}

//bool FPhysicsScene::Overlap(const FShape& Shape, const FTransform& Transform, TArray<AActor*>& OutOverlaps) const
//{
//    if (SceneSolver)
//    {
//        return SceneSolver->Overlap(Shape, Transform, OutOverlaps);
//    }
//    return false;
//}
