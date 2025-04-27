#pragma once
#include "Math/Vector.h"
#include "Math/JungleCollision.h"

// Trace시 사용하는 Collision Shape
namespace ECollisionShape
{
    enum Type : uint8
    {
        Line,
        Box,
        Sphere,
        Capsule
    };
};

// 16바이트 공용체로, 사용할 때 타입을 지정해야합니다.
// 타입과 데이터의 관계를 보장하지 않습니다.
struct FCollisionShape
{
    ECollisionShape::Type ShapeType;

    static FORCEINLINE constexpr float MinBoxExtent() { return KINDA_SMALL_NUMBER; }
    static FORCEINLINE constexpr float MinSphereRadius() { return KINDA_SMALL_NUMBER; }
    static FORCEINLINE constexpr float MinCapsuleRadius() { return KINDA_SMALL_NUMBER; }
    static FORCEINLINE constexpr float MinCapsuleAxisHalfHeight() { return KINDA_SMALL_NUMBER; }

    union
    {
        struct
        {
            float HalfExtentX;
            float HalfExtentY;
            float HalfExtentZ;
        } Box;

        struct
        {
            float Radius;
        } Sphere;

        struct
        {
            float Radius;
            float HalfHeight; // Capsule의 중심부터 Spehre의 중심까지
        } Capsule;
    };

    FCollisionShape()
    {
        ShapeType = ECollisionShape::Line;
    }

    bool IsLine() const
    {
        return ShapeType == ECollisionShape::Line;
    }

    bool IsBox() const
    {
        return ShapeType == ECollisionShape::Box;
    }

    bool IsSphere() const
    {
        return ShapeType == ECollisionShape::Sphere;
    }

    bool IsCapsule() const
    {
        return ShapeType == ECollisionShape::Capsule;
    }

    void SetBox(const FVector& HalfExtent)
    {
        ShapeType = ECollisionShape::Box;
        Box.HalfExtentX = HalfExtent.X;
        Box.HalfExtentY = HalfExtent.Y;
        Box.HalfExtentZ = HalfExtent.Z;
    }

    void SetSphere(const float Radius)
    {
        ShapeType = ECollisionShape::Sphere;
        Sphere.Radius = Radius;
    }

    void SetCapsule(const float Radius, const float HalfHeight)
    {
        ShapeType = ECollisionShape::Capsule;
        Capsule.Radius = Radius;
        Capsule.HalfHeight = HalfHeight;
    }

    bool IsNearlyZero() const
    {
        switch (ShapeType)
        {
            case ECollisionShape::Box:
            {
                return (Box.HalfExtentX <= FCollisionShape::MinBoxExtent() && Box.HalfExtentY <= FCollisionShape::MinBoxExtent() && Box.HalfExtentZ <= FCollisionShape::MinBoxExtent());
            }
            case  ECollisionShape::Sphere:
            {
                return (Sphere.Radius <= FCollisionShape::MinSphereRadius());
            }
            case ECollisionShape::Capsule:
            {
                return (Capsule.Radius <= FCollisionShape::MinCapsuleRadius());
            }
        }

        return true;
    }

    // x,y,z방향의 외접하는 Cube의 크기
    FVector GetExtent() const
    {
        switch (ShapeType)
        {
            case ECollisionShape::Box:
            {
                return FVector(Box.HalfExtentX, Box.HalfExtentY, Box.HalfExtentZ);
            }
            case  ECollisionShape::Sphere:
            {
                return FVector(Sphere.Radius, Sphere.Radius, Sphere.Radius);
            }
            case ECollisionShape::Capsule:
            {
                return FVector(Capsule.Radius, Capsule.Radius, Capsule.HalfHeight);
            }
        }

        return FVector::ZeroVector;
    }
};
