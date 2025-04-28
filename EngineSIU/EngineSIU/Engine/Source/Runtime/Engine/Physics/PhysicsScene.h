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

    void Clear();
    void TickPhysScene(float DeltaSeconds);
    void SyncBodies();

    void AddRigidBody(UShapeComponent* Component);
    void RemoveRigidBody(UShapeComponent* Component);

    // 물리 쿼리 함수 예시
    bool GetOverlappings(UShapeComponent* Shape, TArray<FOverlapInfo>& OutOverlaps);

    float DeltaTime = 0.0f;
    FPhysicsSolver SceneSolver;

private:
    TSet<UShapeComponent*> RegisteredBodies;
};
