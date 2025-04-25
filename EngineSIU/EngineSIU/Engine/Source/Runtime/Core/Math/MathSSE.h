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


#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))

 // vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzleMask(vec, mask)          _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(vec), mask))
#define VecSwizzle(vec, x, y, z, w)        VecSwizzleMask(vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                VecSwizzleMask(vec, MakeShuffleMask(x,x,x,x))
// special swizzle
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)

// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
// special shuffle
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)

struct FMatrix;


// 4 floats
typedef __m128	VectorRegister4Float;


namespace MathSSE
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

    inline void MatrixMultiply(FMatrix* Result, const FMatrix* Matrix1, const FMatrix* Matrix2)
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

    // 검증 안됨
#if 1
    // https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html
    __forceinline __m128 Mat2Mul(__m128 vec1, __m128 vec2)
    {
        return
            _mm_add_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 0, 3, 0, 3)),
                _mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
    }
    // 2x2 row major Matrix adjugate multiply (A#)*B
    __forceinline __m128 Mat2AdjMul(__m128 vec1, __m128 vec2)
    {
        return
            _mm_sub_ps(_mm_mul_ps(VecSwizzle(vec1, 3, 3, 0, 0), vec2),
                _mm_mul_ps(VecSwizzle(vec1, 1, 1, 2, 2), VecSwizzle(vec2, 2, 3, 0, 1)));

    }
    // 2x2 row major Matrix multiply adjugate A*(B#)
    __forceinline __m128 Mat2MulAdj(__m128 vec1, __m128 vec2)
    {
        return
            _mm_sub_ps(_mm_mul_ps(vec1, VecSwizzle(vec2, 3, 0, 3, 0)),
                _mm_mul_ps(VecSwizzle(vec1, 1, 0, 3, 2), VecSwizzle(vec2, 2, 1, 2, 1)));
    }


    // General한 방법으로, Transform에는 사용하지 말것.
    // Transform Matrix의 경우에는 InverseTransformNoScale / InverseTransform 사용하기.
    // Transform의 경우에는 determinant가 작아서 FMatrix::Inverse와 결과가 다르게 나옴.
    inline FMatrix InverseMatrix(const FMatrix& Mat)
    {
        FMatrix Result;
        // use block matrix method
        // A is a matrix, then i(A) or iA means inverse of A, A# (or A_ in code) means adjugate of A, |A| (or detA in code) is determinant, tr(A) is trace

        // sub matrices
        __m128 A = VecShuffle_0101(Mat.mVec[0], Mat.mVec[1]);
        __m128 B = VecShuffle_2323(Mat.mVec[0], Mat.mVec[1]);
        __m128 C = VecShuffle_0101(Mat.mVec[2], Mat.mVec[3]);
        __m128 D = VecShuffle_2323(Mat.mVec[2], Mat.mVec[3]);

        // determinant as (|A| |B| |C| |D|)
        __m128 detSub = _mm_sub_ps(
            _mm_mul_ps(VecShuffle(Mat.mVec[0], Mat.mVec[2], 0, 2, 0, 2), VecShuffle(Mat.mVec[1], Mat.mVec[3], 1, 3, 1, 3)),
            _mm_mul_ps(VecShuffle(Mat.mVec[0], Mat.mVec[2], 1, 3, 1, 3), VecShuffle(Mat.mVec[1], Mat.mVec[3], 0, 2, 0, 2))
        );
        __m128 detA = VecSwizzle1(detSub, 0);
        __m128 detB = VecSwizzle1(detSub, 1);
        __m128 detC = VecSwizzle1(detSub, 2);
        __m128 detD = VecSwizzle1(detSub, 3);

        // let iM = 1/|M| * | X  Y |
        //                  | Z  W |

        // D#C
        __m128 D_C = Mat2AdjMul(D, C);
        // A#B
        __m128 A_B = Mat2AdjMul(A, B);
        // X# = |D|A - B(D#C)
        __m128 X_ = _mm_sub_ps(_mm_mul_ps(detD, A), Mat2Mul(B, D_C));
        // W# = |A|D - C(A#B)
        __m128 W_ = _mm_sub_ps(_mm_mul_ps(detA, D), Mat2Mul(C, A_B));

        // |M| = |A|*|D| + ... (continue later)
        __m128 detM = _mm_mul_ps(detA, detD);

        // Y# = |B|C - D(A#B)#
        __m128 Y_ = _mm_sub_ps(_mm_mul_ps(detB, C), Mat2MulAdj(D, A_B));
        // Z# = |C|B - A(D#C)#
        __m128 Z_ = _mm_sub_ps(_mm_mul_ps(detC, B), Mat2MulAdj(A, D_C));

        // |M| = |A|*|D| + |B|*|C| ... (continue later)
        detM = _mm_add_ps(detM, _mm_mul_ps(detB, detC));

        // tr((A#B)(D#C))
        __m128 tr = _mm_mul_ps(A_B, VecSwizzle(D_C, 0, 2, 1, 3));
        tr = _mm_hadd_ps(tr, tr);
        tr = _mm_hadd_ps(tr, tr);
        // |M| = |A|*|D| + |B|*|C| - tr((A#B)(D#C)
        detM = _mm_sub_ps(detM, tr);

        const __m128 adjSignMask = _mm_setr_ps(1.f, -1.f, -1.f, 1.f);
        // (1/|M|, -1/|M|, -1/|M|, 1/|M|)
        __m128 rDetM = _mm_div_ps(adjSignMask, detM);

        X_ = _mm_mul_ps(X_, rDetM);
        Y_ = _mm_mul_ps(Y_, rDetM);
        Z_ = _mm_mul_ps(Z_, rDetM);
        W_ = _mm_mul_ps(W_, rDetM);


        // apply adjugate and store, here we combine adjugate shuffle and store shuffle
        Result.mVec[0] = VecShuffle(X_, Y_, 3, 1, 3, 1);
        Result.mVec[1] = VecShuffle(X_, Y_, 2, 0, 2, 0);
        Result.mVec[2] = VecShuffle(Z_, W_, 3, 1, 3, 1);
        Result.mVec[3] = VecShuffle(Z_, W_, 2, 0, 2, 0);

        return Result;
    }

    // 변환 행렬 중 Scale=(1,1,1)인 경우에만 사용
    inline FMatrix InverseTransformNoScale(const FMatrix& Mat)
    {
        FMatrix Result;
        // transpose 3x3, we know m03 = m13 = m23 = 0
        __m128 t0 = VecShuffle_0101(Mat.mVec[0], Mat.mVec[1]); // 00, 01, 10, 11
        __m128 t1 = VecShuffle_2323(Mat.mVec[0], Mat.mVec[1]); // 02, 03, 12, 13
        Result.mVec[0] = VecShuffle(t0, Mat.mVec[2], 0, 2, 0, 3); // 00, 10, 20, 23(=0)
        Result.mVec[1] = VecShuffle(t0, Mat.mVec[2], 1, 3, 1, 3); // 01, 11, 21, 23(=0)
        Result.mVec[2] = VecShuffle(t1, Mat.mVec[2], 0, 2, 2, 3); // 02, 12, 22, 23(=0)

        // last line
        Result.mVec[3] = _mm_mul_ps(Result.mVec[0], VecSwizzle1(Mat.mVec[3], 0));
        Result.mVec[3] = _mm_add_ps(Result.mVec[3], _mm_mul_ps(Result.mVec[1], VecSwizzle1(Mat.mVec[3], 1)));
        Result.mVec[3] = _mm_add_ps(Result.mVec[3], _mm_mul_ps(Result.mVec[2], VecSwizzle1(Mat.mVec[3], 2)));
        Result.mVec[3] = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), Result.mVec[3]);

        return Result;
    }

    // 변환 행렬에만 사용
    inline FMatrix InverseTransform(const FMatrix& Mat)
    {
        FMatrix Result;
        // transpose 3x3, we know m03 = m13 = m23 = 0
        __m128 t0 = VecShuffle_0101(Mat.mVec[0], Mat.mVec[1]); // 00, 01, 10, 11
        __m128 t1 = VecShuffle_2323(Mat.mVec[0], Mat.mVec[1]); // 02, 03, 12, 13
        Result.mVec[0] = VecShuffle(t0, Mat.mVec[2], 0, 2, 0, 3); // 00, 10, 20, 23(=0)
        Result.mVec[1] = VecShuffle(t0, Mat.mVec[2], 1, 3, 1, 3); // 01, 11, 21, 23(=0)
        Result.mVec[2] = VecShuffle(t1, Mat.mVec[2], 0, 2, 2, 3); // 02, 12, 22, 23(=0)

        // (SizeSqr(mVec[0]), SizeSqr(mVec[1]), SizeSqr(mVec[2]), 0)
        __m128 sizeSqr;
        sizeSqr = _mm_mul_ps(Result.mVec[0], Result.mVec[0]);
        sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(Result.mVec[1], Result.mVec[1]));
        sizeSqr = _mm_add_ps(sizeSqr, _mm_mul_ps(Result.mVec[2], Result.mVec[2]));

        // optional test to avoid divide by 0
        __m128 one = _mm_set1_ps(1.f);
        // for each component, if(sizeSqr < SMALL_NUMBER) sizeSqr = 1;
        __m128 rSizeSqr = _mm_blendv_ps(
            _mm_div_ps(one, sizeSqr),
            one,
            _mm_cmplt_ps(sizeSqr, _mm_set1_ps(SMALL_NUMBER))
        );

        Result.mVec[0] = _mm_mul_ps(Result.mVec[0], rSizeSqr);
        Result.mVec[1] = _mm_mul_ps(Result.mVec[1], rSizeSqr);
        Result.mVec[2] = _mm_mul_ps(Result.mVec[2], rSizeSqr);

        // last line
        Result.mVec[3] = _mm_mul_ps(Result.mVec[0], VecSwizzle1(Mat.mVec[3], 0));
        Result.mVec[3] = _mm_add_ps(Result.mVec[3], _mm_mul_ps(Result.mVec[1], VecSwizzle1(Mat.mVec[3], 1)));
        Result.mVec[3] = _mm_add_ps(Result.mVec[3], _mm_mul_ps(Result.mVec[2], VecSwizzle1(Mat.mVec[3], 2)));
        Result.mVec[3] = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), Result.mVec[3]);

        return Result;
    }
#else
#endif
}
