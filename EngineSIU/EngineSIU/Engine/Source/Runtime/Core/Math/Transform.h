#pragma once
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Matrix.h"
class FTransform
{
public:
    FQuat Rotation;
    FVector Translation;
    FVector Scale3D;

    FORCEINLINE void AddToTranslation(const FVector& Delta)
    {
        Translation += Delta;
    }

    FORCEINLINE void AddToScale(const FVector& Delta)
    {
        Scale3D += Delta;
    }

    FORCEINLINE void AddToRotation(const FQuat& Delta)
    {
        Rotation = Rotation * Delta;
    }

    FORCEINLINE FMatrix ToMatrixWithScale() const
    {
        FMatrix OutMatrix;

        OutMatrix.M[3][0] = Translation.X;
        OutMatrix.M[3][1] = Translation.Y;
        OutMatrix.M[3][2] = Translation.Z;

        const float x2 = Rotation.X + Rotation.X;
        const float y2 = Rotation.Y + Rotation.Y;
        const float z2 = Rotation.Z + Rotation.Z;
        {
            const float xx2 = Rotation.X * x2;
            const float yy2 = Rotation.Y * y2;
            const float zz2 = Rotation.Z * z2;

            OutMatrix.M[0][0] = (1.0f - (yy2 + zz2)) * Scale3D.X;
            OutMatrix.M[1][1] = (1.0f - (xx2 + zz2)) * Scale3D.Y;
            OutMatrix.M[2][2] = (1.0f - (xx2 + yy2)) * Scale3D.Z;
        }
        {
            const float yz2 = Rotation.Y * z2;
            const float wx2 = Rotation.W * x2;

            OutMatrix.M[2][1] = (yz2 - wx2) * Scale3D.Z;
            OutMatrix.M[1][2] = (yz2 + wx2) * Scale3D.Y;
        }
        {
            const float xy2 = Rotation.X * y2;
            const float wz2 = Rotation.W * z2;

            OutMatrix.M[1][0] = (xy2 - wz2) * Scale3D.Y;
            OutMatrix.M[0][1] = (xy2 + wz2) * Scale3D.X;
        }
        {
            const float xz2 = Rotation.X * z2;
            const float wy2 = Rotation.W * y2;

            OutMatrix.M[2][0] = (xz2 + wy2) * Scale3D.Z;
            OutMatrix.M[0][2] = (xz2 - wy2) * Scale3D.X;
        }

        OutMatrix.M[0][3] = 0.0f;
        OutMatrix.M[1][3] = 0.0f;
        OutMatrix.M[2][3] = 0.0f;
        OutMatrix.M[3][3] = 1.0f;

        return OutMatrix;
    }

    FORCEINLINE FTransform operator*(const FTransform& Other) const
    {
        // Negative Scale이 있을 경우 정상작동 안함
        // Matrix 연산으로 변경해야함
        // \Engine\Source\Runtime\Core\Public\Math\TransformNonVectorized.h Line 1243

        FTransform Result;
        Result.Rotation = Rotation * Other.Rotation;
        Result.Translation = Rotation.RotateVector(Other.Translation * Scale3D) + Translation;
        Result.Scale3D = Scale3D * Other.Scale3D;
        return Result;
    }
};

