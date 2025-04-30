#include "CameraModifier_LetterBox.h"
#include "Engine/Engine.h"
#include "Renderer/PostProcess/CameraPostProcess.h"
#include "Camera/CameraComponent.h"

UCameraModifier_LetterBox::UCameraModifier_LetterBox()
{
}

UCameraModifier_LetterBox::~UCameraModifier_LetterBox()
{
}

void UCameraModifier_LetterBox::SetLetterBox(FLinearColor InBoxColor, float InRatio, float InDuration, bool InTransitionByAlpha, bool InTransitionByMove, TransitionType InType)
{
    BoxColor = InBoxColor;
    Ratio = InRatio;
    Duration = InDuration;
    DisappearByAlpha = InTransitionByAlpha;
    DisappearByMove = InTransitionByMove;
    Type = InType;
}

void UCameraModifier_LetterBox::StartLetterBox()
{
    bEnabled = true;
}

void UCameraModifier_LetterBox::EnableModifier()
{
    Super::EnableModifier();
    StartLetterBox();
}

void UCameraModifier_LetterBox::DisableModifier()
{
    Super::DisableModifier();
    bEnabled = false;
    ElapsedTime = 0.f;
    OnLetterBoxEnd.Broadcast();
}

void UCameraModifier_LetterBox::OnAdded()
{
}

void UCameraModifier_LetterBox::OnRemoved()
{
}

bool UCameraModifier_LetterBox::ModifyCamera(float DeltaTime, FCameraParams& OutParams)
{
    Super::ModifyCamera(DeltaTime, OutParams);

    if (!bEnabled)
    {
        return false;
    }

    ElapsedTime += DeltaTime;

    float Progress = 0.f;
    bool bInfiniteDuration = (Duration < 0.f);

    if (!bInfiniteDuration)
    {
        Progress = FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);
    }

    // 실제 적용될 값
    float CurrentAlpha = BoxColor.A;
    float CurrentRatio = Ratio;

    switch (Type)
    {
    case TransitionIn:
        if (DisappearByAlpha)
        {
            CurrentAlpha = FMath::Lerp(0.f, BoxColor.A, Progress);
        }
        if (DisappearByMove)
        {
            CurrentRatio = FMath::Lerp(FEngineLoop::Renderer.CameraPostProcess->GetWindowRatio(), Ratio, Progress);
        }
        if (!bInfiniteDuration && Progress >= 1.f)
        {
            // In 트랜지션 끝
            Type = TransitionNone;
            ElapsedTime = 0.f;
        }
        break;

    case TransitionOut:
        if (DisappearByAlpha)
        {
            CurrentAlpha = FMath::Lerp(BoxColor.A, 0.f, Progress);
        }
        if (DisappearByMove)
        {
            CurrentRatio = FMath::Lerp(Ratio, FEngineLoop::Renderer.CameraPostProcess->GetWindowRatio(), Progress);
        }
        if (!bInfiniteDuration && Progress >= 1.f)
        {
            // Out 트랜지션 끝
            bEnabled = false;
            ElapsedTime = 0.f;
            OnLetterBoxEnd.Broadcast();
        }
        break;

    case TransitionNone:
    default:
        CurrentAlpha = BoxColor.A;
        CurrentRatio = Ratio;
        break;
    }

    FLinearColor FinalBoxColor = BoxColor;
    FinalBoxColor.A = CurrentAlpha;
    CameraOwner->LetterBoxColor = FinalBoxColor;
    CameraOwner->LetterBoxRatio = CurrentRatio;

    return true;
}
