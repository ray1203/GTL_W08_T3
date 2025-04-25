#pragma once
#include <xmmintrin.h>
#include "Math/Vector.h"

// https://docs.nvidia.com/gameworks/content/gameworkslibrary/physx/guide/Manual/Geometry.html 참고
// 이후 PhysX로 대체 가능
struct FRay
{
    FVector Origin;
    FVector Direction;   
};

struct FSphere
{
    FVector Center;
    float Radius;
};

struct FBox
{
    FVector Min;
    FVector Max;
};

struct FCapsule
{
    FVector A; // 캡슐의 한쪽 끝
    FVector B; // 캡슐의 다른쪽 끝
    float Radius;
};

class JungleCollision
{
public:
    static bool RayIntersectsAABB(const FRay& Ray, const FBox& AABB, float* outT);

    static bool RayIntersectsSphere(const FRay& Ray, const FSphere& Sphere, float* outT);

    static bool RayIntersectsCapsule(const FRay& Ray, const FCapsule& Capsule, float* outT);

    static bool Intersects(const FBox& A, const FBox& B);

    static bool Intersects(const FSphere& A, const FSphere& B);

    static bool Intersects(const FCapsule& A, const FCapsule& B);

    static bool Intersects(const FSphere& Sphere, const FBox& AABB);

    static bool Intersects(const FBox& AABB, const FSphere& Sphere);

    static bool Intersects(const FCapsule& Capsule, const FBox& AABB);

    static bool Intersects(const FBox& AABB, const FCapsule& Capsule);

    static bool Intersects(const FSphere& Sphere, const FCapsule& Capsule);

    static bool Intersects(const FCapsule& Capsule, const FSphere& Sphere);

    // 내부 계산은 SIMD로 작동
private:
    struct RaySIMD
    {
        __m128 origin; // (x, y, z, _)
        __m128 dir;    // (x, y, z, _)
    };

    struct SphereSIMD
    {
        __m128 center; // (x, y, z, _)
        float radius;
    };

    struct AABBSIMD
    {
        __m128 min; // (x, y, z, _)
        __m128 max; // (x, y, z, _)
    };

    struct CapsuleSIMD
    {
        __m128 a; // (x, y, z, _): 캡슐의 한쪽 끝
        __m128 b; // (x, y, z, _): 캡슐의 다른쪽 끝
        float radius;
    };

    inline static float dot3(__m128 v1, __m128 v2);

    inline static float dist3_sq(__m128 a, __m128 b);

    inline static float SegmentDistSq_SIMD(
        __m128 p1, __m128 q1, // Ray: p1 + t*(q1-p1)
        __m128 p2, __m128 q2, // Capsule axis: p2~q2
        float& s_c, float& t_c,
        __m128& c1, __m128& c2);

    inline static bool RayIntersectsSphereSIMD(const RaySIMD& ray, const SphereSIMD& sphere, float* outT = nullptr);

    inline static bool RayIntersectsAABBSIMD(const RaySIMD& ray, const AABBSIMD& box, float* outT = nullptr);
    
    inline static bool RayIntersectsCapsuleSIMD(const RaySIMD& ray, const CapsuleSIMD& capsule, float* outT = nullptr);
};


