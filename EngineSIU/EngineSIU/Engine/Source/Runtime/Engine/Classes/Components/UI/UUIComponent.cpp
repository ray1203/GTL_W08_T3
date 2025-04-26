#include "UUIComponent.h"

#include "GameFramework/Actor.h"
#include "ImGUI/imgui.h"
#include "Misc/Parse.h"
void UUIComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (AActor* Owner = GetOwner())
    {
        Owner->SetActorTickInEditor(true);
        // 고유 WidgetName 지정
        FString UniqueName = FString::Printf(TEXT("%s_%s_%d"),
            *Owner->GetName(), TEXT("UI"), GlobalWidgetCounter++);
        SetWidgetName(TCHAR_TO_UTF8(*UniqueName));
    }
}

void UUIComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    // 기본 UI 없는 베이스는 아무것도 안 그려줌
}
void UUIComponent::RenderUI()
{
}
bool UUIComponent::BeginWidget()
{
    if (!bVisible)
        return false;

    ImGui::SetNextWindowPos(ImVec2(Position.X, Position.Y), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(Size.X, Size.Y), ImGuiCond_Once);

    ImGui::Begin(WidgetName.c_str(), nullptr, ImGuiWindowFlags_NoSavedSettings);
    return true;
}

void UUIComponent::EndWidget()
{
    ImGui::End();
}

void UUIComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

    OutProperties.Add(TEXT("UIPosition"), FString::Printf(TEXT("%.2f,%.2f"), Position.X, Position.Y));
    OutProperties.Add(TEXT("UISize"), FString::Printf(TEXT("%.2f,%.2f"), Size.X, Size.Y));
}

void UUIComponent::SetProperties(const TMap<FString, FString>& Properties)
{
    Super::SetProperties(Properties);

    if (const FString* Value = Properties.Find(TEXT("UIPosition")))
    {
        FVector2D Parsed;
        if (FParse::Value(**Value, TEXT(""), Parsed)) // Match 없이 직접 파싱
        {
            Position = Parsed;
        }
    }

    if (const FString* Value = Properties.Find(TEXT("UISize")))
    {
        FVector2D Parsed;
        if (FParse::Value(**Value, TEXT(""), Parsed))
        {
            Size = Parsed;
        }
    }
}
