#include "CameraModifier_CameraShake.h"
#include "Curve/CurveManager.h"

UCameraModifier_CameraShake::UCameraModifier_CameraShake()
{
}

void UCameraModifier_CameraShake::SetShakeCurve(const FString& InCurveName, uint32 TargetProperty, float Duration, float Amplitude, float Offset)
{

}

void UCameraModifier_CameraShake::StartShake()
{
    bShaking = true;
}

void UCameraModifier_CameraShake::EnableModifier()
{
    Super::EnableModifier();
    StartShake();
}

void UCameraModifier_CameraShake::DisableModifier()
{
}

void UCameraModifier_CameraShake::OnAdded()
{
}

void UCameraModifier_CameraShake::OnRemoved()
{
}

bool UCameraModifier_CameraShake::ModifyCamera(float DeltaTime, FCameraParams& OutParams)
{
    Super::ModifyCamera(DeltaTime, OutParams);

    if (!bShaking)
    {
        return false;
    }

    ElapsedTime += DeltaTime;

    float T = FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);
    if (T >= 1)
    {
        bShaking = false;
        ElapsedTime = 0.f;
        OnTransitionEnd.Broadcast();
    }
    return true;
}

void UCameraModifier_CameraShake::SetTransitionTime(float InTransitionTime)
{
}
