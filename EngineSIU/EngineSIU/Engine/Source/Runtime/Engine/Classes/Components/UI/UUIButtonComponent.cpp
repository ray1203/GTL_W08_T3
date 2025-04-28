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

    if (ImGui::Button(Label.c_str()))
    {
        OnClick();
    }

    EndWidget();
}
UObject* UUIButtonComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComp = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComp->SetLabel(GetLabel());
    // OnClick은 복사되지 않음 (런타임 함수 포인터이므로)
    return NewComp;
}

void UUIButtonComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Label"), FString(Label.c_str()));
}

void UUIButtonComponent::SetProperties(const TMap<FString, FString>& Properties)
{
    Super::SetProperties(Properties);
    if (const FString* Value = Properties.Find(TEXT("Label")))
        SetLabel(TCHAR_TO_UTF8(**Value));
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
