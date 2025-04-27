#pragma once
#include "Serialization/Archive.h"

struct FVector;
struct FVector4;
struct FRotator;
struct FQuat;

#include "Vector.h"
#include "Vector4.h"

// 4x4 행렬 연산
struct alignas(16) FMatrix
{
public:
    union
    {
        alignas(16) float M[4][4];
        __m128       mVec[4];
    };

    // 명시적 생성자 (initializer list 사용 위함
    FMatrix(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33)
    {
        M[0][0] = m00; M[0][1] = m01; M[0][2] = m02; M[0][3] = m03;
        M[1][0] = m10; M[1][1] = m11; M[1][2] = m12; M[1][3] = m13;
        M[2][0] = m20; M[2][1] = m21; M[2][2] = m22; M[2][3] = m23;
        M[3][0] = m30; M[3][1] = m31; M[3][2] = m32; M[3][3] = m33;
    }
    // 기본 생성자: 0으로 초기화
    FMatrix() {
        for (int i = 0; i < 4; ++i)
            mVec[i] = _mm_setzero_ps();
    }

    // float[4][4] initializer list 생성자 지원
    FMatrix(const float(&inM)[4][4]) {
        for (int i = 0; i < 4; ++i)
            mVec[i] = _mm_loadu_ps(inM[i]);
    }

public:
    static const FMatrix Identity;

public:
    // 기본 연산자 오버로딩
    FMatrix operator+(const FMatrix& Other) const;
    FMatrix operator-(const FMatrix& Other) const;
    FMatrix operator*(const FMatrix& Other) const;
    FMatrix operator*(float Scalar) const;
    FMatrix operator/(float Scalar) const;
    float* operator[](int row);
    const float* operator[](int row) const;

    // 유틸리티 함수
    static FMatrix Transpose(const FMatrix& Mat);
    static FMatrix Inverse(const FMatrix& Mat);
    static FMatrix InverseTransform(const FMatrix& Mat);
    static FMatrix CreateRotationMatrix(float roll, float pitch, float yaw);
    static FMatrix CreateScaleMatrix(float scaleX, float scaleY, float scaleZ);
    static FVector TransformVector(const FVector& v, const FMatrix& m);
    static FVector4 TransformVector(const FVector4& v, const FMatrix& m);
    static FMatrix CreateTranslationMatrix(const FVector& position);

    FVector4 TransformFVector4(const FVector4& vector) const;
    FVector TransformPosition(const FVector& vector) const;

    static FMatrix GetScaleMatrix(const FVector& InScale);
    static FMatrix GetTranslationMatrix(const FVector& InPosition);
    static FMatrix GetRotationMatrix(const FRotator& InRotation);
    static FMatrix GetRotationMatrix(const FQuat& InRotation);

    FQuat ToQuat(const FMatrix& M) const;
};

inline FArchive& operator<<(FArchive& Ar, FMatrix& M)
{
    Ar << M.M[0][0] << M.M[0][1] << M.M[0][2] << M.M[0][3];
    Ar << M.M[1][0] << M.M[1][1] << M.M[1][2] << M.M[1][3];
    Ar << M.M[2][0] << M.M[2][1] << M.M[2][2] << M.M[2][3];
    Ar << M.M[3][0] << M.M[3][1] << M.M[3][2] << M.M[3][3];
    return Ar;
}
