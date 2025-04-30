#include "CameraModifier_CameraTransition.h"
#include "Camera/CameraComponent.h"

UCameraModifier_CameraTransition::UCameraModifier_CameraTransition()
    : TargetPosition(FVector::ZeroVector)
    , TargetRotation(FRotator())
    , TargetFOV(90.f)
    , CurrentPosition(FVector::ZeroVector)
    , CurrentRotation(FRotator())
    , CurrentFOV(90.f)
    , bTransitioning(false)
    , StartPosition(FVector::ZeroVector)
    , StartRotation(FRotator())
    , StartFOV(90.f)
{
    Duration = 5.f; // 기본 트랜지션 시간
    Priority = 255; // 가장 높은 우선순위
}

void UCameraModifier_CameraTransition::SetTransitionTarget(const FVector& InTargetPosition, const FRotator& InTargetRotation, float InTargetFOV)
{
    TargetPosition = InTargetPosition;
    TargetRotation = InTargetRotation;
    TargetFOV = InTargetFOV;
}

void UCameraModifier_CameraTransition::StartTransition()
{
    bTransitioning = true;
}

void UCameraModifier_CameraTransition::EnableModifier()
{
    Super::EnableModifier();
    
    // 카메라의 현재 위치, 회전, FOV를 저장
    UCameraComponent* OwnerCamera = CameraOwner->GetCameraComponent();
    if (!OwnerCamera)
    {
        return;
    }

    StartPosition = OwnerCamera->GetCameraLocation();
    StartRotation = OwnerCamera->GetWorldRotation();
    StartFOV = OwnerCamera->GetFOV();
    
    StartTransition(); // 기본값으로 시작 5f
}

void UCameraModifier_CameraTransition::DisableModifier()
{
    Super::DisableModifier();
}

void UCameraModifier_CameraTransition::OnAdded()
{  

}

void UCameraModifier_CameraTransition::OnRemoved()
{
}

bool UCameraModifier_CameraTransition::ModifyCamera(float DeltaTime, FCameraParams& OutParams)
{
    Super::ModifyCamera(DeltaTime, OutParams);

    if (!bTransitioning)
        return false;

    ElapsedTime += DeltaTime;

    float T = FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);

    CurrentPosition = FMath::Lerp(StartPosition, TargetPosition, T);
    CurrentRotation = FMath::Lerp(StartRotation, TargetRotation, T);

    CurrentFOV = FMath::Lerp(StartFOV, TargetFOV, T);

    OutParams.Position = CurrentPosition;
    OutParams.Rotation = CurrentRotation;
    OutParams.FOV = CurrentFOV;

    if (T >= 1)
    {
        bTransitioning = false;
        ElapsedTime = 0.f;
        OnTransitionEnd.Broadcast();
    }

    return true;
}

void UCameraModifier_CameraTransition::SetTransitionTime(float InTransitionTime)
{
    Duration = InTransitionTime;
}
