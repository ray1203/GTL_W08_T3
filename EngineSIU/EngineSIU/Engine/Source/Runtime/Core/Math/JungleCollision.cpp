#include "JungleCollision.h"

bool JungleCollision::RayIntersectsAABB(const FRay& Ray, const FBox& AABB, float* outT)
{
    RaySIMD _Ray;
    _Ray.origin = _mm_set_ps(0.0f, Ray.Origin.Z, Ray.Origin.Y, Ray.Origin.X);
    _Ray.dir = _mm_set_ps(0.0f, Ray.Direction.Z, Ray.Direction.Y, Ray.Direction.X);

    AABBSIMD _AABB;
    _AABB.min = _mm_set_ps(0.0f, AABB.Min.Z, AABB.Min.Y, AABB.Min.X);
    _AABB.max = _mm_set_ps(0.0f, AABB.Max.Z, AABB.Max.Y, AABB.Max.X);

    return RayIntersectsAABBSIMD(_Ray, _AABB, outT);
}

bool JungleCollision::RayIntersectsSphere(const FRay& Ray, const FSphere& Sphere, float* outT)
{
    RaySIMD _Ray;
    _Ray.origin = _mm_set_ps(0.0f, Ray.Origin.Z, Ray.Origin.Y, Ray.Origin.X);
    _Ray.dir = _mm_set_ps(0.0f, Ray.Direction.Z, Ray.Direction.Y, Ray.Direction.X);
    SphereSIMD _Sphere;
    _Sphere.center = _mm_set_ps(0.0f, Sphere.Center.Z, Sphere.Center.Y, Sphere.Center.X);
    _Sphere.radius = Sphere.Radius;
    return RayIntersectsSphereSIMD(_Ray, _Sphere, outT);
}

bool JungleCollision::RayIntersectsCapsule(const FRay& Ray, const FCapsule& Capsule, float* outT)
{
    RaySIMD _Ray;
    _Ray.origin = _mm_set_ps(0.0f, Ray.Origin.Z, Ray.Origin.Y, Ray.Origin.X);
    _Ray.dir = _mm_set_ps(0.0f, Ray.Direction.Z, Ray.Direction.Y, Ray.Direction.X);
    CapsuleSIMD _Capsule;
    _Capsule.a = _mm_set_ps(0.0f, Capsule.A.Z, Capsule.A.Y, Capsule.A.X);
    _Capsule.b = _mm_set_ps(0.0f, Capsule.B.Z, Capsule.B.Y, Capsule.B.X);
    _Capsule.radius = Capsule.Radius;
    return RayIntersectsCapsuleSIMD(_Ray, _Capsule, outT);
}

bool JungleCollision::Intersects(const FBox& A, const FBox& B)
{
    AABBSIMD _A;
    _A.min = _mm_set_ps(0.0f, A.Min.Z, A.Min.Y, A.Min.X);
    _A.max = _mm_set_ps(0.0f, A.Max.Z, A.Max.Y, A.Max.X);

    AABBSIMD _B;
    _B.min = _mm_set_ps(0.0f, B.Min.Z, B.Min.Y, B.Min.X);
    _B.max = _mm_set_ps(0.0f, B.Max.Z, B.Max.Y, B.Max.X);
    
    __m128 Less = _mm_cmple_ps(_A.max, _B.min);
    __m128 Greater = _mm_cmple_ps(_B.max, _A.min);
    int mask = _mm_movemask_ps(_mm_or_ps(Less, Greater));
    // mask의 상위 3비트가 0이면 교차
    return (mask & 0x7) == 0;
}

bool JungleCollision::Intersects(const FSphere& A, const FSphere& B)
{
    SphereSIMD _A;
    _A.center = _mm_set_ps(0.0f, A.Center.Z, A.Center.Y, A.Center.X);
    _A.radius = A.Radius;

    SphereSIMD _B;
    _B.center = _mm_set_ps(0.0f, B.Center.Z, B.Center.Y, B.Center.X);
    _B.radius = B.Radius;

    __m128 Diff = _mm_sub_ps(_A.center, _B.center);
    __m128 Mul = _mm_mul_ps(Diff, Diff);
    // x+y+z
    float dist2 = _mm_cvtss_f32(
        _mm_add_ss(
            _mm_add_ss(
                _mm_shuffle_ps(Mul, Mul, _MM_SHUFFLE(0, 0, 0, 1)), // y
                _mm_shuffle_ps(Mul, Mul, _MM_SHUFFLE(0, 0, 0, 2))  // z
            ),
            Mul // x
        )
    );
    float r = A.Radius + B.Radius;
    return dist2 <= r * r;
}

bool JungleCollision::Intersects(const FCapsule& A, const FCapsule& B)
{
    CapsuleSIMD _A;
    _A.a = _mm_set_ps(0.0f, A.A.Z, A.A.Y, A.A.X);
    _A.b = _mm_set_ps(0.0f, A.B.Z, A.B.Y, A.B.X);
    _A.radius = A.Radius;

    CapsuleSIMD _B;
    _B.a = _mm_set_ps(0.0f, B.A.Z, B.A.Y, B.A.X);
    _B.b = _mm_set_ps(0.0f, B.B.Z, B.B.Y, B.B.X);
    _B.radius = B.Radius;

    float s, t;
    __m128 c1, c2; // 더미용
    float dist2 = SegmentDistSq_SIMD(_A.a, _A.b, _B.a, _B.b, s, t, c1, c2);
    float r = A.Radius + B.Radius;
    return dist2 <= r * r;
}

bool JungleCollision::Intersects(const FOrientedBox& A, const FOrientedBox& B)
{
    // 각 박스의 로컬 축
    FVector AxisA[3] = { A.AxisX, A.AxisY, A.AxisZ };
    FVector AxisB[3] = { B.AxisX, B.AxisY, B.AxisZ };
    // 두 박스 중심 벡터
    FVector D = B.Center - A.Center;
    // 15개의 축 모두 검사 (A0, A1, A2, B0, B1, B2, A0xB0, A0xB1, ..., A2xB2)
    for (int i = 0; i < 3; ++i)
    {
        if (!TestAxis(AxisA[i], A, B, D)) return false;
    }
    for (int i = 0; i < 3; ++i)
    {
        if (!TestAxis(AxisB[i], A, B, D)) return false;
    }
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            FVector Cross = FVector::CrossProduct(AxisA[i], AxisB[j]);
            if (Cross.LengthSquared() > SMALL_NUMBER)
            {
                if (!TestAxis(Cross, A, B, D)) return false;
            }
        }
    }
    return true; // 모든 축에서 분리 안됐으면 충돌
}

bool JungleCollision::Intersects(const FSphere& Sphere, const FBox& AABB)
{
    SphereSIMD _Sphere;
    _Sphere.center = _mm_set_ps(0.0f, Sphere.Center.Z, Sphere.Center.Y, Sphere.Center.X);
    _Sphere.radius = Sphere.Radius;

    AABBSIMD _AABB;
    _AABB.min = _mm_set_ps(0.0f, AABB.Min.Z, AABB.Min.Y, AABB.Min.X);
    _AABB.max = _mm_set_ps(0.0f, AABB.Max.Z, AABB.Max.Y, AABB.Max.X);

    // Clamp Sphere.Center to Box
    __m128 Clamped = _mm_min_ps(_mm_max_ps(_Sphere.center, _AABB.min), _AABB.max);
    __m128 Diff = _mm_sub_ps(_Sphere.center, Clamped);
    __m128 Mul = _mm_mul_ps(Diff, Diff);
    float dist2 = _mm_cvtss_f32(
        _mm_add_ss(
            _mm_add_ss(
                _mm_shuffle_ps(Mul, Mul, _MM_SHUFFLE(0, 0, 0, 1)), // y
                _mm_shuffle_ps(Mul, Mul, _MM_SHUFFLE(0, 0, 0, 2))  // z
            ),
            Mul // x
        )
    );
    return dist2 <= Sphere.Radius * Sphere.Radius;
}

bool JungleCollision::Intersects(const FBox& AABB, const FSphere& Sphere)
{
    return Intersects(Sphere, AABB);
}

bool JungleCollision::Intersects(const FCapsule& Capsule, const FBox& AABB)
{
    CapsuleSIMD _Capsule;
    _Capsule.a = _mm_set_ps(0.0f, Capsule.A.Z, Capsule.A.Y, Capsule.A.X);
    _Capsule.b = _mm_set_ps(0.0f, Capsule.B.Z, Capsule.B.Y, Capsule.B.X);
    _Capsule.radius = Capsule.Radius;

    AABBSIMD _AABB;
    _AABB.min = _mm_set_ps(0.0f, AABB.Min.Z, AABB.Min.Y, AABB.Min.X);
    _AABB.max = _mm_set_ps(0.0f, AABB.Max.Z, AABB.Max.Y, AABB.Max.X);

    // 선분을 여러점으로 쪼개서 AABB로 클램프
    float minDist2 = FLT_MAX;
    constexpr int NumSteps = 5;
    for (int i = 0; i <= NumSteps; ++i)
    {
        float t = float(i) / float(NumSteps);
        __m128 p = _mm_add_ps(_Capsule.a, _mm_mul_ps(_mm_set1_ps(t), _mm_sub_ps(_Capsule.b, _Capsule.a)));
        __m128 clamped = _mm_min_ps(_mm_max_ps(p, _AABB.min), _AABB.max);
        float dist2 = dist3_sq(p, clamped);
        if (dist2 < minDist2) minDist2 = dist2;
    }
    return minDist2 <= _Capsule.radius * _Capsule.radius;
}

bool JungleCollision::Intersects(const FBox& AABB, const FCapsule& Capsule)
{
    return Intersects(Capsule, AABB);
}

bool JungleCollision::Intersects(const FSphere& Sphere, const FCapsule& Capsule)
{
    SphereSIMD _Sphere;
    _Sphere.center = _mm_set_ps(0.0f, Sphere.Center.Z, Sphere.Center.Y, Sphere.Center.X);
    _Sphere.radius = Sphere.Radius;

    CapsuleSIMD _Capsule;
    _Capsule.a = _mm_set_ps(0.0f, Capsule.A.Z, Capsule.A.Y, Capsule.A.X);
    _Capsule.b = _mm_set_ps(0.0f, Capsule.B.Z, Capsule.B.Y, Capsule.B.X);
    _Capsule.radius = Capsule.Radius;


    // 선분-점 최소거리
    __m128 ab = _mm_sub_ps(_Capsule.b, _Capsule.a);
    __m128 ap = _mm_sub_ps(_Sphere.center, _Capsule.a);
    float ab_dot_ab = dot3(ab, ab);
    float t = ab_dot_ab > 0.0f ? dot3(ap, ab) / ab_dot_ab : 0.0f;
    t = (t < 0.0f) ? 0.0f : ((t > 1.0f) ? 1.0f : t); // t = std::clamp(t, 0.0f, 1.0f);
    __m128 closest = _mm_add_ps(_Capsule.a, _mm_mul_ps(_mm_set1_ps(t), ab));
    float dist2 = dist3_sq(closest, _Sphere.center);
    float radsum = _Capsule.radius + _Sphere.radius;
    return dist2 <= radsum * radsum;
}

bool JungleCollision::Intersects(const FCapsule& Capsule, const FSphere& Sphere)
{
    return Intersects(Sphere, Capsule);
}

bool JungleCollision::Intersects(const FOrientedBox& Box, const FSphere& Sphere)
{
    FVector Local = Sphere.Center - Box.Center;
    FVector ClosestPoint = Box.Center;
    FVector Axes[3] = { Box.AxisX, Box.AxisY, Box.AxisZ };
    float Extents[3] = { Box.ExtentX, Box.ExtentY, Box.ExtentZ };
    for (int i = 0; i < 3; ++i)
    {
        float Distance = FVector::DotProduct(Local, Axes[i]);
        Distance = FMath::Clamp(Distance, -Extents[i], Extents[i]);
        ClosestPoint += Axes[i] * Distance;
    }
    return (ClosestPoint - Sphere.Center).LengthSquared() <= Sphere.Radius * Sphere.Radius;
}

bool JungleCollision::Intersects(const FSphere& Sphere, const FOrientedBox& Box)
{
    return Intersects(Box, Sphere);
}

bool JungleCollision::Intersects(const FOrientedBox& Box, const FCapsule& Capsule)
{
    FVector Top = Capsule.A;
    FVector Bottom = Capsule.B;
    // 캡슐 세그먼트의 Closest Point를 OBB에 대해 찾는다
    FVector ClosestA = ClosestPointOnOBB(Box, Top);
    FVector ClosestB = ClosestPointOnOBB(Box, Bottom);
    FVector ClosestOnSegment = ClosestPointOnSegment(Top, Bottom, (ClosestA + ClosestB) * 0.5f);
    float DistSq = (ClosestPointOnOBB(Box, ClosestOnSegment) - ClosestOnSegment).LengthSquared();
    return DistSq <= Capsule.Radius * Capsule.Radius;
}

bool JungleCollision::Intersects(const FCapsule& Capsule, const FOrientedBox& Box)
{
    return false;
}

bool JungleCollision::Intersects(const FOrientedBox& Box, const FBox& AABB)
{
    return false;
}

bool JungleCollision::Intersects(const FBox& AABB, const FOrientedBox& Box)
{
    return false;
}

inline float JungleCollision::dot3(__m128 v1, __m128 v2)
{
    __m128 dot = _mm_dp_ps(v1, v2, 0x71);
    return _mm_cvtss_f32(dot); // 결과값 꺼내기
}

inline float JungleCollision::dist3_sq(__m128 a, __m128 b)
{
    __m128 d = _mm_sub_ps(a, b);
    return dot3(d, d);
}

// 참조: Real-Time Collision Detection, Christer Ericson 5.1.9
inline float JungleCollision::SegmentDistSq_SIMD(__m128 p1, __m128 q1, __m128 p2, __m128 q2, float& s_c, float& t_c, __m128& c1, __m128& c2)
{
    __m128 d1 = _mm_sub_ps(q1, p1); // Ray 방향
    __m128 d2 = _mm_sub_ps(q2, p2); // Capsule 축 방향
    __m128 r = _mm_sub_ps(p1, p2);

    float a = dot3(d1, d1);
    float e = dot3(d2, d2);
    float f = dot3(d2, r);

    float s, t;
    const float EPS = 1e-6f;

    if (a <= EPS && e <= EPS)
    {
        // 둘 다 점
        s = t = 0.0f;
        c1 = p1;
        c2 = p2;
        float dist2 = dist3_sq(c1, c2);
        s_c = s; t_c = t;
        return dist2;
    }
    if (a <= EPS)
    {
        // 첫 번째가 점
        s = 0.0f;
        t = f / e;
        t = fmaxf(0.0f, fminf(1.0f, t));
    }
    else
    {
        float c = dot3(d1, r);
        if (e <= EPS)
        {
            // 두 번째가 점
            t = 0.0f;
            s = -c / a;
            s = fmaxf(0.0f, s); // Ray는 0 이상
        }
        else
        {
            float b = dot3(d1, d2);
            float denom = a * e - b * b;

            if (denom != 0.0f)
                s = fmaxf(0.0f, (b * f - c * e) / denom);
            else
                s = 0.0f;
            t = (b * s + f) / e;

            if (t < 0.0f)
            {
                t = 0.0f;
                s = fmaxf(0.0f, -c / a);
            }
            else if (t > 1.0f)
            {
                t = 1.0f;
                s = fmaxf(0.0f, (b - c) / a);
            }
        }
    }

    c1 = _mm_add_ps(p1, _mm_mul_ps(_mm_set1_ps(s), d1));
    c2 = _mm_add_ps(p2, _mm_mul_ps(_mm_set1_ps(t), d2));
    s_c = s;
    t_c = t;
    return dist3_sq(c1, c2);
}

// Ray-Sphere Intersection (returns true if hit)
inline bool JungleCollision::RayIntersectsSphereSIMD(const RaySIMD& ray, const SphereSIMD& sphere, float* outT)
{
    __m128 oc = _mm_sub_ps(ray.origin, sphere.center);
    __m128 dir = ray.dir;

    // a = dot(D, D)
    __m128 a = _mm_dp_ps(dir, dir, 0x71); // .xyz, result in .x
    // b = 2*dot(oc, D)
    __m128 b = _mm_dp_ps(oc, dir, 0x71);
    b = _mm_add_ps(b, b); // 2b
    // c = dot(oc, oc) - r^2
    __m128 c = _mm_dp_ps(oc, oc, 0x71);
    __m128 r2 = _mm_set1_ps(sphere.radius * sphere.radius);
    c = _mm_sub_ps(c, r2);

    // discriminant = b^2 - 4ac
    __m128 b2 = _mm_mul_ps(b, b);
    __m128 fourAC = _mm_mul_ps(_mm_set1_ps(4.0f), _mm_mul_ps(a, c));
    __m128 disc = _mm_sub_ps(b2, fourAC);
    float d = _mm_cvtss_f32(disc);
    if (d < 0.0f) return false;

    float aF = _mm_cvtss_f32(a), bF = _mm_cvtss_f32(b), cF = _mm_cvtss_f32(c);
    float sqrtD = sqrtf(d);
    float t1 = (-bF - sqrtD) / (2.0f * aF);
    float t2 = (-bF + sqrtD) / (2.0f * aF);

    float t = t1 > 0.0f ? t1 : t2; // 선택 기준은 상황에 따라 다름
    if (t < 0.0f) return false;
    if (outT) *outT = t;
    return true;
}

// Ray-AABB Intersection (returns true if hit)
// https://tavianator.com/fast-branchless-raybounding-box-intersections/
inline bool JungleCollision::RayIntersectsAABBSIMD(const RaySIMD& ray, const AABBSIMD& box, float* outT)
{
    __m128 invDir = _mm_div_ps(_mm_set1_ps(1.0f), ray.dir);

    __m128 t0 = _mm_mul_ps(_mm_sub_ps(box.min, ray.origin), invDir);
    __m128 t1 = _mm_mul_ps(_mm_sub_ps(box.max, ray.origin), invDir);

    __m128 tmin = _mm_min_ps(t0, t1);
    __m128 tmax = _mm_max_ps(t0, t1);

    // Find largest tmin and smallest tmax
    float tminF = std::max({ _mm_cvtss_f32(tmin), ((float*)&tmin)[1], ((float*)&tmin)[2] });
    float tmaxF = std::min({ _mm_cvtss_f32(tmax), ((float*)&tmax)[1], ((float*)&tmax)[2] });

    if (tmaxF < 0.0f || tminF > tmaxF) return false;
    if (outT) *outT = tminF > 0.0f ? tminF : tmaxF;
    return true;
}

// Ray-Capsule intersection (returns true if intersects, and hit distance t)
inline bool JungleCollision::RayIntersectsCapsuleSIMD(const RaySIMD& ray, const CapsuleSIMD& capsule, float* outT)
{
    // Ray: origin + t*dir, t >= 0
    // Capsule: [a, b], radius
    __m128 rayEnd = _mm_add_ps(ray.origin, _mm_mul_ps(ray.dir, _mm_set1_ps(1e6f))); // 충분히 긴 레이

    float s, t;
    __m128 c1, c2;
    // line segment 사이의 거리가 r1 + r2보다 작으면 교차
    float dist2 = SegmentDistSq_SIMD(
        ray.origin, rayEnd,
        capsule.a, capsule.b,
        s, t, c1, c2);

    if (dist2 > capsule.radius * capsule.radius)
        return false; // 교차 없음

    // 실제 교차점을 구하려면, 교차하는 구간의 t를 해석적으로 구해야 함
    // 여기서는 s가 t >= 0 인 최소 root를 hitT로 사용
    if (s < 0.0f) return false; // 레이 앞쪽일 때만
    if (outT) *outT = s;
    return true;
}

inline bool JungleCollision::TestAxis(const FVector& Axis, const FOrientedBox& A, const FOrientedBox& B, const FVector& D)
{
    float ProjectA =
        A.ExtentX * FMath::Abs(FVector::DotProduct(Axis, A.AxisX)) +
        A.ExtentY * FMath::Abs(FVector::DotProduct(Axis, A.AxisY)) +
        A.ExtentZ * FMath::Abs(FVector::DotProduct(Axis, A.AxisZ));
    float ProjectB =
        B.ExtentX * FMath::Abs(FVector::DotProduct(Axis, B.AxisX)) +
        B.ExtentY * FMath::Abs(FVector::DotProduct(Axis, B.AxisY)) +
        B.ExtentZ * FMath::Abs(FVector::DotProduct(Axis, B.AxisZ));
    float Distance = FMath::Abs(FVector::DotProduct(D, Axis));
    return Distance <= (ProjectA + ProjectB);
}

inline FVector JungleCollision::ClosestPointOnOBB(const FOrientedBox& Box, const FVector& Point)
{
    FVector Local = Point - Box.Center;
    FVector Result = Box.Center;
    FVector Axes[3] = { Box.AxisX, Box.AxisY, Box.AxisZ };
    float Extents[3] = { Box.ExtentX, Box.ExtentY, Box.ExtentZ };

    for (int i = 0; i < 3; ++i)
    {
        float Distance = FVector::DotProduct(Local, Axes[i]);
        Distance = FMath::Clamp(Distance, -Extents[i], Extents[i]);
        Result += Axes[i] * Distance;
    }
    return Result;
}

inline FVector JungleCollision::ClosestPointOnSegment(const FVector& A, const FVector& B, const FVector& P)
{
    FVector AB = B - A;
    float AB_LengthSq = AB.LengthSquared();
    if (AB_LengthSq <= SMALL_NUMBER) return A;
    float T = FVector::DotProduct(P - A, AB) / AB_LengthSq;
    T = FMath::Clamp(T, 0.0f, 1.0f);
    return A + AB * T;
}
