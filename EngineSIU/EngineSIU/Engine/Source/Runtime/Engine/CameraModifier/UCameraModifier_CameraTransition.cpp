#include "UCameraModifier_CameraTransition.h"
#include "Camera/CameraComponent.h"

UCameraModifier_CameraTransition::UCameraModifier_CameraTransition()
    : TargetPosition(FVector::ZeroVector)
    , TargetRotation(FRotator())
    , TargetFOV(90.f)
    , CurrentPosition(FVector::ZeroVector)
    , CurrentRotation(FRotator())
    , CurrentFOV(90.f)
    , TransitionTime(5.f)
    , bTransitioning(false)
    , TransitionElapsed(0.f)
    , StartPosition(FVector::ZeroVector)
    , StartRotation(FRotator())
    , StartFOV(90.f)
{
    Priority = 255; // 가장 높은 우선순위
}

void UCameraModifier_CameraTransition::SetTransitionTarget(const FVector& InTargetPosition, const FRotator& InTargetRotation, float InTargetFOV)
{
    TargetPosition = InTargetPosition;
    TargetRotation = InTargetRotation;
    TargetFOV = InTargetFOV;
}

void UCameraModifier_CameraTransition::StartTransition(float InTransitionTime)
{
    TransitionTime = InTransitionTime;
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
    // FInterpTo를 사용하여 각 값들을 부드럽게 목표로 이동

    TransitionElapsed += DeltaTime;

    float T = FMath::Clamp(TransitionElapsed / TransitionTime, 0.f, 1.f);

    CurrentPosition = FMath::Lerp(StartPosition, TargetPosition, T);
    CurrentRotation = FMath::Lerp(StartRotation, TargetRotation, T);
    //CurrentPosition.X = FMath::FInterpTo(CurrentPosition.X, TargetPosition.X, DeltaTime, InterpSpeed);
    //CurrentPosition.Y = FMath::FInterpTo(CurrentPosition.Y, TargetPosition.Y, DeltaTime, InterpSpeed);
    //CurrentPosition.Z = FMath::FInterpTo(CurrentPosition.Z, TargetPosition.Z, DeltaTime, InterpSpeed);

    //CurrentRotation.Pitch = FMath::FInterpTo(CurrentRotation.Pitch, TargetRotation.Pitch, DeltaTime, InterpSpeed * 5);
    //CurrentRotation.Yaw = FMath::FInterpTo(CurrentRotation.Yaw, TargetRotation.Yaw, DeltaTime, InterpSpeed * 5);
    //CurrentRotation.Roll = FMath::FInterpTo(CurrentRotation.Roll, TargetRotation.Roll, DeltaTime, InterpSpeed * 5);

    //CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, InterpSpeed);
    CurrentFOV = FMath::Lerp(StartFOV, TargetFOV, T);

    OutParams.Position = CurrentPosition;
    OutParams.Rotation = CurrentRotation;
    OutParams.FOV = CurrentFOV;

    // 목표에 충분히 가까워지면 트랜지션 종료
    //const float PosTolerance = 0.01f;
    //const float RotTolerance = 0.1f;
    //const float FOVTolerance = 0.01f;
    //if (FVector::Distance(CurrentPosition, TargetPosition) < PosTolerance &&
    //    FMath::Abs(CurrentRotation.Pitch - TargetRotation.Pitch) < RotTolerance &&
    //    FMath::Abs(CurrentRotation.Yaw - TargetRotation.Yaw) < RotTolerance &&
    //    FMath::Abs(CurrentRotation.Roll - TargetRotation.Roll) < RotTolerance &&
    //    FMath::Abs(CurrentFOV - TargetFOV) < FOVTolerance)
    //{
    //    bTransitioning = false;
    //    OnTransitionEnd.Broadcast();
    //}

    if (T >= 1)
    {
        bTransitioning = false;
        TransitionElapsed = 0.f;
        OnTransitionEnd.Broadcast();
    }

    return true;
}

