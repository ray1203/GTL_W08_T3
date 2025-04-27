#pragma once

#include "Math/Vector.h"
#include "Container/Set.h"
#include "OverlapInfo.h"
#include "Math/Transform.h"
#include "PhysicsSolver.h"

class UShapeComponent;

class FPhysicsScene
{
public:
    FPhysicsScene();
    ~FPhysicsScene();
    void TickPhysScene(float DeltaSeconds);
    void SyncBodies();

    void AddRigidBody(UShapeComponent* Component);
    void RemoveRigidBody(UShapeComponent* Component);

    // 물리 쿼리 함수 예시
    bool LineTrace(const FVector& Start, const FVector& End, FHitResult& OutHit) const;
    bool Overlap(const UShapeComponent& Shape, const FTransform& Transform, TArray<FOverlapInfo>& OutOverlaps) const;

    float DeltaTime = 0.0f;
    FPhysicsSolver SceneSolver;

private:
    TSet<UShapeComponent*> RegisteredBodies;
};
