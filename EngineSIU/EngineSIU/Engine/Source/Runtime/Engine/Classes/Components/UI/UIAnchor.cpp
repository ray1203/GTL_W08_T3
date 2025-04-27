#include "UIAnchor.h"
FVector2D GetAnchorPosition(EUIAnchor Anchor, const FVector2D& ScreenSize)
{
    float X = 0.f, Y = 0.f;

    switch (Anchor)
    {
    case EUIAnchor::TopLeft:       X = 0.f;           Y = 0.f; break;
    case EUIAnchor::TopCenter:     X = 0.5f;          Y = 0.f; break;
    case EUIAnchor::TopRight:      X = 1.f;           Y = 0.f; break;
    case EUIAnchor::CenterLeft:    X = 0.f;           Y = 0.5f; break;
    case EUIAnchor::Center:        X = 0.5f;          Y = 0.5f; break;
    case EUIAnchor::CenterRight:   X = 1.f;           Y = 0.5f; break;
    case EUIAnchor::BottomLeft:    X = 0.f;           Y = 1.f; break;
    case EUIAnchor::BottomCenter:  X = 0.5f;          Y = 1.f; break;
    case EUIAnchor::BottomRight:   X = 1.f;           Y = 1.f; break;
    }

    return FVector2D(ScreenSize.X * X, ScreenSize.Y * Y);
}
