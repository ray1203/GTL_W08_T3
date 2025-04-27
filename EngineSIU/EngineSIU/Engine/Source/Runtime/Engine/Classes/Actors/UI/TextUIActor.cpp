#include "TextUIActor.h"

#include "Components/UI/UUITextComponent.h"

ATextUIActor::ATextUIActor()
{
    TextComponent = AddComponent<UUITextComponent>(TEXT("UITextComponent"));
}

void ATextUIActor::SetText(const std::string& InText)
{
    if (TextComponent)
    {
        TextComponent->SetText(InText);
    }
}
