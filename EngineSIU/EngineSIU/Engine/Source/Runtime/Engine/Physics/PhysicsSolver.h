#pragma once

#include "Container/Set.h"
#include "Math/Vector.h"
#include "Math/Transform.h"
#include "CollisionShape.h"
#include "Components/Shapes/ShapeComponent.h"

class AActor;
struct FHitResult;
struct FPhysicsBody;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnOverlapDelegate, const FPhysicsBody&)

struct FPhysicsBody
{
    UShapeComponent* Component;
    FTransform Transform;
    FVector Velocity;
    FVector Acceleration;
    float Mass;
    bool bIsSimulatingPhysics;
    bool bBlock;
    bool bGrounded;
    bool bStickToGround;
    float Restitution;
    FCollisionShape CollisionShape;
    FPhysicsBody(UShapeComponent* InComponent)
        : Component(InComponent), Mass(1.0f)
    {
        bIsSimulatingPhysics = InComponent->bIsSimulatingPhysics;
        Transform = InComponent->GetRelativeTransform();
        Velocity = FVector::ZeroVector;
        Acceleration = FVector::ZeroVector;
        Restitution = 0.5f;
        bStickToGround = true;
    }

    bool operator==(const FPhysicsBody& Other) const
    {
        return Component == Other.Component;
    }

public:
    FOnOverlapDelegate OnOverlap;
};

// 간단한 PhysicsSolver 예시
class FPhysicsSolver
{
public:
    FPhysicsSolver();
    ~FPhysicsSolver();

    // 복사 생성자와 복사 대입 연산자를 delete로 명시적으로 삭제
    FPhysicsSolver(const FPhysicsSolver&) = delete;
    FPhysicsSolver& operator=(const FPhysicsSolver&) = delete;

    // 시뮬레이션 이전 최신값을 반영
    void UpdateBodyFromComponent();

    // 물리 시뮬레이션을 한 프레임 진행
    void AdvanceAndDispatch(float DeltaTime);

    // 파티클(물리 객체) 추가/삭제
    void AddBody(UShapeComponent* Component);
    void RemoveBody(UShapeComponent* Component);
    FPhysicsBody* GetBody(const UShapeComponent* Component);

    // 시뮬레이션 결과를 가져옴
    bool GetSimulatedTransform(UShapeComponent* Component, FTransform& OutTransform) const;
    bool GetOverlappingBodies(const FPhysicsBody& Body, TArray<FPhysicsBody*>& OverlappingBodies);

private:
    // 시뮬레이션
    void ApplyForces(); // 힘을 계산 (현재는 중력만)
    void Integrate(float DeltaTime); // 힘에 따른 위치 업데이트
    void HandleOverlaps(); // 겹침 처리
    void HandleCollisions(); // 충돌 감지 및 반응 계산(속도, 가속도 등)
    void UpdateTransforms(); // 변화된 위치를 업데이트

    static bool IsOverlapping(const FPhysicsBody& BodyA, const FPhysicsBody& BodyB);

protected:
    // 시뮬레이트 된 결과물
    TArray<FPhysicsBody> SimulatedBodies;
    TArray<TPair<int32, int32>> CachedOverlaps;

private:
    const float RestitutionThreshold = 1e-2f;
    float Friction = 0.7;

    // contanct 계산용
    struct FContactInfo
    {
        FVector Normal = FVector::ZeroVector;
        float Penetration = 0.f;
        float RelativeVelocity = 0.f;
        bool bValid = false;
    };
    static FContactInfo ComputeContactInfo(const FPhysicsBody& BodyA, const FPhysicsBody& BodyB);

};
