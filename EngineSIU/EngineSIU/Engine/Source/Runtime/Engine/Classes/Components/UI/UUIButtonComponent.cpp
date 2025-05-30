#include "UUIButtonComponent.h"

#include "ImGUI/imgui.h"
#include "UObject/Casts.h"


void UUIButtonComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UUIButtonComponent::RenderUI()
{
    if (!BeginWidget())
        return;

    ImGui::SetWindowFontScale(FontScale);
    if (ImGui::Button(Label.c_str(), ImVec2(ButtonSize.X, ButtonSize.Y)))
    {
        OnClick();
    }
    ImGui::SetWindowFontScale(1.0f); // reset after

    EndWidget();
}

UObject* UUIButtonComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComp = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComp->SetLabel(GetLabel());
    NewComp->SetButtonSize(ButtonSize);
    NewComp->SetFontScale(FontScale);
    return NewComp;
}

void UUIButtonComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Label"), FString(Label.c_str()));
    OutProperties.Add(TEXT("ButtonSize"), ButtonSize.ToString());
    OutProperties.Add(TEXT("FontScale"), FString::Printf(TEXT("%f"), FontScale));
}

void UUIButtonComponent::SetProperties(const TMap<FString, FString>& Properties)
{
    Super::SetProperties(Properties);
    if (const FString* Value = Properties.Find(TEXT("Label")))
        SetLabel(TCHAR_TO_UTF8(**Value));
    if (const FString* Value = Properties.Find(TEXT("ButtonSize")))
        ButtonSize.InitFromString(*Value);
    if (const FString* Value = Properties.Find(TEXT("FontScale")))
        FontScale = FString::ToFloat(*Value);
}

void UUIButtonComponent::OnClick()
{
    if (NativeCallback)
    {
        NativeCallback();
    }

    if (LuaCallback.valid())
    {
        sol::protected_function_result result = LuaCallback();
        if (!result.valid())
        {
            sol::error err = result;
            UE_LOG(ELogLevel::Error, TEXT("[Lua Button Callback Error] %s"), *FString(err.what()));
        }
    }
}
