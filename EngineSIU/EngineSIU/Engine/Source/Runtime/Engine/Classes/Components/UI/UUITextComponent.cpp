#include "UUITextComponent.h"

#include "UObject/Casts.h"

#include "ImGUI/imgui.h"
void UUITextComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
void UUITextComponent::RenderUI()
{
    if (!BeginWidget())
        return;
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(TextColor.R, TextColor.G, TextColor.B, TextColor.A));
    ImGui::SetWindowFontScale(TextScale); // 폰트 크기 조절
    ImGui::Text("%s", Text.c_str());
    ImGui::SetWindowFontScale(1.0f); // 원복
    ImGui::PopStyleColor();
    EndWidget();
}
UObject* UUITextComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComp = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComp->SetText(GetText());
    NewComp->TextColor = TextColor;
    NewComp->TextScale = TextScale;
    return NewComp;
}
void UUITextComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Text"), FString(Text.c_str()));
    OutProperties.Add(TEXT("TextScale"), FString::SanitizeFloat(TextScale));
    OutProperties.Add(TEXT("TextColor"), TextColor.ToString());
}

void UUITextComponent::SetProperties(const TMap<FString, FString>& Properties)
{
    Super::SetProperties(Properties);

    if (const FString* Value = Properties.Find(TEXT("Text")))
        SetText(TCHAR_TO_UTF8(**Value));

    if (const FString* Value = Properties.Find(TEXT("TextScale")))
        TextScale = FCString::Atof(**Value);

    if (const FString* Value = Properties.Find(TEXT("TextColor")))
    {
        TextColor.InitFromString(*Value);
    }
}
