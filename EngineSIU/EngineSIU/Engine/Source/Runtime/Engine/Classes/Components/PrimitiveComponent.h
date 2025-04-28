#pragma once
#include "Components/SceneComponent.h"
#include "Physics/OverlapInfo.h"

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    bool IntersectRayTriangle(
        const FVector& rayOrigin, const FVector& rayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance
    ) const;

    
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    FBoundingBox AABB;

private:
    FString m_Type;

public:
    FString GetType() { return m_Type; }

    void SetType(const FString& _Type)
    {
        m_Type = _Type;
        //staticMesh = FEngineLoop::resourceMgr.GetMesh(m_Type);
    }
    FBoundingBox GetBoundingBox() const { return AABB; }

    // week 08 : collision
public:
    // overlap : 두개의 물체가 부딪힌 이후 겹침
    // block : 두개의 물체가 부딪힌 이후 겹치지 않음
    bool bGenerateOverlapEvents : 1 = true; // 오버랩 이벤트를 생성할지 여부
    bool bBlockComponent : 1 = true; // 블로킹 이벤트를 생성할지 여부

    const TArray<FOverlapInfo>& GetOverlappingComponents() const { return OverlappingComponents; }
    void SetOverlappingComponents(const TArray<FOverlapInfo>& InOverlappingComponents) { OverlappingComponents = InOverlappingComponents; }
private:
    TArray<FOverlapInfo> OverlappingComponents; // 현재 겹쳐있는 컴포넌트들

//public:
//    bool GetGenerateOverlapEvents() const { return bGenerateOverlapEvents; }
//    void SetGenerateOverlapEvents(bool InGenerateOverlapEvents) { bGenerateOverlapEvents = InGenerateOverlapEvents; }
//    bool GetBlockComponent() const { return bBlockComponent; }
//    void SetBlockComponent(bool InBlockComponent) { bBlockComponent = InBlockComponent; }
};

