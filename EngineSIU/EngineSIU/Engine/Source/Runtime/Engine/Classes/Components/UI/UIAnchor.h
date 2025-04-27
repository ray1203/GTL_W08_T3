#pragma once
#include "Math/Vector.h"

enum class EUIAnchor : uint8
{
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight,
};
FVector2D GetAnchorPosition(EUIAnchor Anchor, const FVector2D& ScreenSize);
