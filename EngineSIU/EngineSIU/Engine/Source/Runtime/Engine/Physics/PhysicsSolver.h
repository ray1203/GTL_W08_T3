#pragma once

#include "Container/Set.h"
#include "Math/Vector.h"
#include "Math/Transform.h"
#include "CollisionShape.h"
#include "Components/Shapes/ShapeComponent.h"

class AActor;
struct FHitResult;

struct FPhysicsBody
{
    UShapeComponent* Component;
    FTransform Transform;
    FVector Velocity;
    FVector Acceleration;
    float Mass;
    bool bIsSimulatingPhysics;
    FCollisionShape CollisionShape;
    FPhysicsBody(UShapeComponent* InComponent)
        : Component(InComponent), Mass(1.0f), bIsSimulatingPhysics(true)
    {
        Transform = InComponent->GetRelativeTransform();
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
    }

    bool operator==(const FPhysicsBody& Other) const
    {
        return Component == Other.Component;
    }
};

// 간단한 PhysicsSolver 예시
class FPhysicsSolver
{
public:
    FPhysicsSolver();
    ~FPhysicsSolver();

    // 물리 시뮬레이션을 한 프레임 진행
    void AdvanceAndDispatch(float DeltaTime);

    // 시뮬레이션
    void ApplyForces(); // 힘을 계산 (현재는 중력만)
    void Integrate(float DeltaTime); // 힘에 따른 위치 업데이트
    void HandleCollisions(); // 충돌 감지 및 반응 계산(속도, 가속도 등)
    void UpdateTransforms(); // 변화된 위치를 업데이트


    // 파티클(물리 객체) 추가/삭제
    void AddBody(UShapeComponent* Component);
    void RemoveBody(UShapeComponent* Component);

    // 시뮬레이션 결과 transform을 가져옴 (예시)
    bool GetSimulatedTransform(UShapeComponent* Component, FTransform& OutTransform) const;

private:
    bool Raycast(const FVector& Start, const FVector& End, FHitResult& OutHit) const;
    bool Overlap(const FCollisionShape& Shape, const FTransform& Transform, TArray<AActor*>& OutOverlaps) const;

protected:
    // 시뮬레이트 된 결과물
    TArray<FPhysicsBody> SimulatedBodies;
};
