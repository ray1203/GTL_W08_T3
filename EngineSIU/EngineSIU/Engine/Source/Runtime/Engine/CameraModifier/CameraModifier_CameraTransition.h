#pragma once
#include "CameraModifier.h"


DECLARE_MULTICAST_DELEGATE(FOnTransitionEndDelegate)

class UCameraModifier_CameraTransition : public UCameraModifier
{
    DECLARE_CLASS(UCameraModifier_CameraTransition, UCameraModifier)

public:
    UCameraModifier_CameraTransition();

    void SetTransitionTarget(const FVector& InTargetPosition, const FRotator& InTargetRotation, float InTargetFOV);

    void StartTransition();

    virtual void EnableModifier() override;
    virtual void DisableModifier() override;

    virtual void OnAdded() override;
    virtual void OnRemoved() override;
    virtual bool ModifyCamera(float DeltaTime, FCameraParams& OutParams) override;

    void SetTransitionTime(float InTransitionTime);


protected:
    FVector TargetPosition;
    FRotator TargetRotation;
    float TargetFOV;

    FVector StartPosition;
    FRotator StartRotation;
    float StartFOV;

    FVector CurrentPosition;
    FRotator CurrentRotation;
    float CurrentFOV;

    bool bTransitioning;

public:
    FOnTransitionEndDelegate OnTransitionEnd;
};
