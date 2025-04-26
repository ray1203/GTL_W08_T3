#include "ButtonUIActor.h"
#include "Components/UI/UUIButtonComponent.h"

AButtonUIActor::AButtonUIActor()
{
    ButtonComponent = AddComponent<UUIButtonComponent>(TEXT("UIButtonComponent"));
}

void AButtonUIActor::SetLabel(const std::string& InLabel)
{
    if (ButtonComponent)
    {
        ButtonComponent->SetLabel(InLabel);
    }
}

void AButtonUIActor::SetOnClick(std::function<void()> Callback)
{
    if (ButtonComponent)
    {
        ButtonComponent->SetOnClick(std::move(Callback));
    }
}
