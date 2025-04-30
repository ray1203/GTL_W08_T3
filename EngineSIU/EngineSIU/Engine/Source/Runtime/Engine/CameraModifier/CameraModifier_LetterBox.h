#pragma once
#include "CameraModifier.h"

DECLARE_MULTICAST_DELEGATE(FOnLetterBoxEndDelegate)
class UCameraModifier_LetterBox :
    public UCameraModifier
{
    DECLARE_CLASS(UCameraModifier_LetterBox, UCameraModifier)

public:
    enum TransitionType
    {
        TransitionIn = 0,
        TransitionOut = 1,
        TransitionNone = 2,
    };

    UCameraModifier_LetterBox();
    ~UCameraModifier_LetterBox();

    void SetLetterBox(FLinearColor InBoxColor, float InRatio, float InDuration, bool InTransitionByAlpha, bool InTransitionByMove, TransitionType InType);

    void StartLetterBox();

    virtual void EnableModifier() override;
    virtual void DisableModifier() override;

    virtual void OnAdded() override;
    virtual void OnRemoved() override;
    virtual bool ModifyCamera(float DeltaTime, FCameraParams& OutParams) override;

    FOnLetterBoxEndDelegate OnLetterBoxEnd;
private:
    FLinearColor BoxColor = FLinearColor(0,0,0,1);
    float Ratio = 0.f;
    bool bEnabled = false;
    bool DisappearByAlpha = false;
    bool DisappearByMove = false;
    TransitionType Type = TransitionNone;
};
