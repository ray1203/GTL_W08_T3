#include "UUIButtonComponent.h"

#include "ImGUI/imgui.h"

void UUIButtonComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UUIButtonComponent::RenderUI()
{
    if (!BeginWidget())
        return;

    if (ImGui::Button(Label.c_str()))
    {
        if (OnClick)
        {
            OnClick();
        }
    }

    EndWidget();
}
