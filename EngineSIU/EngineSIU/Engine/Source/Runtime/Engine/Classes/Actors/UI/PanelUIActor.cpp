#include "PanelUIActor.h"

#include "Components/UI/UUIPanelComponent.h"

APanelUIActor::APanelUIActor()
{
    PanelComponent = AddComponent<UUIPanelComponent>(TEXT("UIPanelComponent"));
}

void APanelUIActor::SetTexture(const FWString& Path)
{
    if (PanelComponent)
    {
        PanelComponent->SetTexture(Path);
    }
}

void APanelUIActor::SetSize(float Width, float Height)
{
    if (PanelComponent)
    {
        PanelComponent->Size = FVector2D(Width, Height);
    }
}

void APanelUIActor::SetOffset(float X, float Y)
{
    if (PanelComponent)
    {
        PanelComponent->Offset = FVector2D(X, Y);
    }
}

void APanelUIActor::SetAnchor(EUIAnchor Anchor)
{
    if (PanelComponent)
    {
        PanelComponent->Anchor = Anchor;
    }
}
