#pragma once
#include "CameraModifier.h"

DECLARE_MULTICAST_DELEGATE(FOnShakeEndDelegate)

class UCameraModifier_CameraShake : public UCameraModifier
{
    DECLARE_CLASS(UCameraModifier_CameraShake, UCameraModifier)

public:
    enum ShakeProperty
    {
        PositionX = 1 << 0,
        PositionY = 1 << 1,
        PositionZ = 1 << 2,
        Pitch = 1 << 3,
        Yaw = 1 << 4,
        Roll = 1 << 5,
        FOV = 1 << 6,
    };

    UCameraModifier_CameraShake();

    void SetShakeCurve(const FString& InCurveName)
    {
        CurveName = InCurveName;
    }

    void SetShakeCurve(const FString& InCurveName, uint32 InTargetProperty, float InDuration, float InAmplitude);

    void StartShake();

    virtual void EnableModifier() override;
    virtual void DisableModifier() override;

    virtual void OnAdded() override;
    virtual void OnRemoved() override;
    virtual bool ModifyCamera(float DeltaTime, FCameraParams& OutParams) override;

protected:
    bool bShaking = false;
    FString CurveName;
    uint32 TargetPropertiesMask;
    float Duration = 0.f;
    float Amplitude = 0.f;

public:
    FOnShakeEndDelegate OnTransitionEnd;

private:
    struct FShakeProperty
    {
        uint32 Bit;
        const TCHAR* Name;
    };

    static const FShakeProperty ShakePropertyNames[7];

    static TArray<FString> GetShakePropertyNames(uint32 Bitmask)
    {
        TArray<FString> Result;
        for (const auto& Pair : ShakePropertyNames)
        {
            if ((Bitmask & Pair.Bit) != 0)
            {
                Result.Add(Pair.Name);
            }
        }
        return Result;
    }

};
