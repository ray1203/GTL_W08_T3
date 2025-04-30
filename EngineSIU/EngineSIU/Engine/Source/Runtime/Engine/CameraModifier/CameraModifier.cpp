#include "CameraModifier.h"
#include "Camera/PlayerCameraManager.h"

UCameraModifier::UCameraModifier()
{
}

void UCameraModifier::SetOwner(APlayerCameraManager* InCameraOwner)
{
    CameraOwner = InCameraOwner;
}

void UCameraModifier::UpdateAlpha(float DeltaTime)
{
}
