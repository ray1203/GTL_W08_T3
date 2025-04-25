#pragma once
#include <immintrin.h>
#include "HAL/PlatformType.h"

/**
 * @param A0	Selects which element (0-3) from 'A' into 1st slot in the result
 * @param A1	Selects which element (0-3) from 'A' into 2nd slot in the result
 * @param B2	Selects which element (0-3) from 'B' into 3rd slot in the result
 * @param B3	Selects which element (0-3) from 'B' into 4th slot in the result
 */
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )

#define VectorReplicate(Vec, Index) VectorReplicateTemplate<Index>(Vec)

struct FMatrix;


// 4 floats
typedef __m128	VectorRegister4Float;


namespace SSE
{
/**
 * Vector의 특정 인덱스를 복제합니다.
 * @tparam Index 복제할 Index (0 ~ 3)
 * @param Vector 복제할 대상
 * @return 복제된 레지스터
 */
template <int Index>
FORCEINLINE VectorRegister4Float VectorReplicateTemplate(const VectorRegister4Float& Vector)
{
    return _mm_shuffle_ps(Vector, Vector, SHUFFLEMASK(Index, Index, Index, Index));
}


/** Vector4 곱연산 */
FORCEINLINE VectorRegister4Float VectorMultiply(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
{
    return _mm_mul_ps(Vec1, Vec2);
}

FORCEINLINE VectorRegister4Float VectorAdd(const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2)
{
    return _mm_add_ps(Vec1, Vec2);
}

FORCEINLINE VectorRegister4Float VectorMultiplyAdd(
    const VectorRegister4Float& Vec1,
    const VectorRegister4Float& Vec2,
    const VectorRegister4Float& Vec3
)
{
    return VectorAdd(VectorMultiply(Vec1, Vec2), Vec3);
}

inline void VectorMatrixMultiply(FMatrix* Result, const FMatrix* Matrix1, const FMatrix* Matrix2)
{
    // 행렬 B의 열을 SIMD 레지스터에 로드
    __m128 B0 = _mm_load_ps(Matrix2->M[0]);
    __m128 B1 = _mm_load_ps(Matrix2->M[1]);
    __m128 B2 = _mm_load_ps(Matrix2->M[2]);
    __m128 B3 = _mm_load_ps(Matrix2->M[3]);

    for (int i = 0; i < 4; ++i) {
        // 행렬 A의 i번째 행 로드
        __m128 A_row = _mm_load_ps(Matrix1->M[i]);

        // 행 요소 브로드캐스트
        __m128 A0 = _mm_shuffle_ps(A_row, A_row, 0x00);
        __m128 A1 = _mm_shuffle_ps(A_row, A_row, 0x55);
        __m128 A2 = _mm_shuffle_ps(A_row, A_row, 0xAA);
        __m128 A3 = _mm_shuffle_ps(A_row, A_row, 0xFF);

        // 열별 곱셈 및 누적
        __m128 sum = _mm_add_ps(
            _mm_add_ps(_mm_mul_ps(A0, B0), _mm_mul_ps(A1, B1)),
            _mm_add_ps(_mm_mul_ps(A2, B2), _mm_mul_ps(A3, B3))
        );

        // 결과 저장
        _mm_store_ps(Result->M[i], sum);
    }

//    // 레지스터에 값 로드
//    const VectorRegister4Float* Matrix1Ptr = reinterpret_cast<const VectorRegister4Float*>(Matrix1);
//    const VectorRegister4Float* Matrix2Ptr = reinterpret_cast<const VectorRegister4Float*>(Matrix2);
//    VectorRegister4Float* Ret = reinterpret_cast<VectorRegister4Float*>(Result);
//    VectorRegister4Float Temp, R0, R1, R2;
//
//    // 첫번째 행 계산
//    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[0], 0), Matrix2Ptr[0]);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 1), Matrix2Ptr[1], Temp);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 2), Matrix2Ptr[2], Temp);
//    R0 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[0], 3), Matrix2Ptr[3], Temp);
//
//    // 두번째 행 계산
//    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[1], 0), Matrix2Ptr[0]);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 1), Matrix2Ptr[1], Temp);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 2), Matrix2Ptr[2], Temp);
//    R1 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[1], 3), Matrix2Ptr[3], Temp);
//
//    // 세번째 행 계산
//    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[2], 0), Matrix2Ptr[0]);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 1), Matrix2Ptr[1], Temp);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 2), Matrix2Ptr[2], Temp);
//    R2 = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[2], 3), Matrix2Ptr[3], Temp);
//
//    // 네번째 행 계산
//    Temp = VectorMultiply(VectorReplicate(Matrix1Ptr[3], 0), Matrix2Ptr[0]);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 1), Matrix2Ptr[1], Temp);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 2), Matrix2Ptr[2], Temp);
//    Temp = VectorMultiplyAdd(VectorReplicate(Matrix1Ptr[3], 3), Matrix2Ptr[3], Temp);
//
//    // 결과 저장
//    Ret[0] = R0;
//    Ret[1] = R1;
//    Ret[2] = R2;
//    Ret[3] = Temp;
}
}
