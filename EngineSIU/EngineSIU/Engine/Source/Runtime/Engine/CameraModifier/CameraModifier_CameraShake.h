#pragma once
#include "CameraModifier.h"

DECLARE_MULTICAST_DELEGATE(FOnTransitionEndDelegate)

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
        ScaleX = 1 << 6,
        ScaleY = 1 << 7,
        ScaleZ = 1 << 8,
        FOV = 1 << 9,
    };

    UCameraModifier_CameraShake();

    void SetShakeCurve(const FString& InCurveName)
    {
        CurveName = InCurveName;
    }

    void StartShake();

    virtual void EnableModifier() override;
    virtual void DisableModifier() override;

    virtual void OnAdded() override;
    virtual void OnRemoved() override;
    virtual bool ModifyCamera(float DeltaTime, FCameraParams& OutParams) override;

    void SetTransitionTime(float InTransitionTime);


protected:
    bool bShaking = false;
    FString CurveName;

public:
    FOnTransitionEndDelegate OnTransitionEnd;
};
