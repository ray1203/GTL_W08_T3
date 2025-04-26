#include "UUITextComponent.h"

void UUITextComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
void UUITextComponent::RenderUI()
{
    if (!BeginWidget())
        return;

    ImGui::Text("%s", Text.c_str());

    EndWidget();
}
