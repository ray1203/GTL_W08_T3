#include "UUIComponent.h"

#include "GameFramework/Actor.h"
#include "ImGUI/imgui.h"
#include "ImGui/imgui_internal.h"

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
    ImGuiWindowFlags Flags = 0;
    if (bNoMove)          Flags |= ImGuiWindowFlags_NoMove;
    if (bNoResize)        Flags |= ImGuiWindowFlags_NoResize;
    if (bNoTitleBar)      Flags |= ImGuiWindowFlags_NoTitleBar;
    if (bNoSavedSettings) Flags |= ImGuiWindowFlags_NoSavedSettings;
    if (bNoBackground)    Flags |= ImGuiWindowFlags_NoBackground;
    if (bNoInputs)        Flags |= ImGuiWindowFlags_NoInputs;
    if (bAutoSize)        Flags |= ImGuiWindowFlags_AlwaysAutoResize;
    Flags |= ImGuiWindowFlags_NoScrollbar;
    Flags |= ImGuiWindowFlags_NoScrollWithMouse;
    ImVec2 ScreenSize = ImGui::GetIO().DisplaySize;
    FVector2D AnchorPos = GetAnchorPosition(Anchor, FVector2D(ScreenSize.x, ScreenSize.y));
    FVector2D FinalPos = AnchorPos + Offset;
    FinalPos -= Size * 0.5f;

    ImGui::SetNextWindowPos(ImVec2(FinalPos.X, FinalPos.Y), ImGuiCond_Always);

    //ImGui::SetNextWindowPos(ImVec2(Position.X, Position.Y), ImGuiCond_Always);
    if (!bAutoSize)
    {
        ImGui::SetNextWindowSize(ImVec2(Size.X, Size.Y), ImGuiCond_Always);
    }
    ImGui::Begin(WidgetName.c_str(), nullptr, Flags);
    if (bAutoSize)
    {
        ImVec2 ActualSize = ImGui::GetWindowSize();
        Size = FVector2D(ActualSize.x, ActualSize.y);
    }
    return true;
}

void UUIComponent::EndWidget()
{
    ImGui::End();
}
UObject* UUIComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    /*FString UniqueName = FString::Printf(TEXT("%s_%s_%d"),
        *GetOwner()->GetName(), TEXT("UI"), GlobalWidgetCounter++);
    SetWidgetName(TCHAR_TO_UTF8(*UniqueName));*/
    //NewComponent->Position = Position;
    NewComponent->Anchor = Anchor;
    NewComponent->Offset = Offset;
    NewComponent->Size = Size;
    NewComponent->bNoMove = bNoMove;
    NewComponent->bNoResize = bNoResize;
    NewComponent->bNoTitleBar = bNoTitleBar;
    NewComponent->bNoSavedSettings = bNoSavedSettings;
    NewComponent->bNoBackground = bNoBackground;
    NewComponent->bNoInputs = bNoInputs;
    NewComponent->bAutoSize = bAutoSize;
    NewComponent->bVisible = bVisible;
    NewComponent->ZOrder = ZOrder;

    return NewComponent;
}

void UUIComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);

    //WidgetName는 식별자로 작동만 하면 되니 저장 X
    //OutProperties.Add(TEXT("UIPosition"), Position.ToString());

    OutProperties.Add(TEXT("Anchor"), FString::FromInt(static_cast<int32>(Anchor)));
    OutProperties.Add(TEXT("Offset"), Offset.ToString());
    OutProperties.Add(TEXT("UISize"), Size.ToString());
    OutProperties.Add(TEXT("bNoMove"), bNoMove ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bNoResize"), bNoResize ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bNoTitleBar"), bNoTitleBar ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bNoSavedSettings"), bNoSavedSettings ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bNoBackground"), bNoBackground ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bNoInputs"), bNoInputs ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bAutoSize"), bAutoSize ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("bVisible"), bVisible ? TEXT("true") : TEXT("false"));
    OutProperties.Add(TEXT("ZOrder"), FString::FromInt(ZOrder));

}

void UUIComponent::SetProperties(const TMap<FString, FString>& Properties)
{
    Super::SetProperties(Properties);
    if (const FString* Val = Properties.Find(TEXT("Anchor")))
        Anchor = static_cast<EUIAnchor>(FCString::Atoi(**Val));
    if (const FString* Val = Properties.Find(TEXT("Offset")))
    {
        Offset.InitFromString(*Val);
    }
    if (const FString* TempStr = Properties.Find(TEXT("UISize")))
    {
        Size.InitFromString(*TempStr);
    }
    auto BoolFromMap = [&](const TCHAR* Key, bool& OutBool) {
        if (const FString* Value = Properties.Find(Key))
            OutBool = Value->ToBool();
        };

    BoolFromMap(TEXT("bNoMove"), bNoMove);
    BoolFromMap(TEXT("bNoResize"), bNoResize);
    BoolFromMap(TEXT("bNoTitleBar"), bNoTitleBar);
    BoolFromMap(TEXT("bNoSavedSettings"), bNoSavedSettings);
    BoolFromMap(TEXT("bNoBackground"), bNoBackground);
    BoolFromMap(TEXT("bNoInputs"), bNoInputs);
    BoolFromMap(TEXT("bAutoSize"), bAutoSize);
    BoolFromMap(TEXT("bVisible"), bVisible);

    if (const FString* Value = Properties.Find(TEXT("ZOrder")))
    {
        ZOrder = FCString::Atoi(**Value);
    }

}
