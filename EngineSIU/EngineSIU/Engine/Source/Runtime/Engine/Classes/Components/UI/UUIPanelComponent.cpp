#include "UUIPanelComponent.h"
#include "EngineLoop.h"
#include "UObject/Casts.h"

UUIPanelComponent::UUIPanelComponent()
{
    SetTexture(TexturePath.ToWideString());
    bNoFocus = true;
}

UObject* UUIPanelComponent::Duplicate(UObject* InOuter)
{
    UUIPanelComponent* NewComponent = Cast<UUIPanelComponent>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->TexturePath = TexturePath;
        NewComponent->Texture = Texture;
    }
    return NewComponent;
}



void UUIPanelComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UUIPanelComponent::RenderUI()
{
    if (!bVisible || !Texture || !Texture->TextureSRV)
        return;
    if (!BeginWidget())
        return;

    //ImGui::SetCursorPos(ImVec2(Offset.X, Offset.Y));
    ImGui::Image(
        reinterpret_cast<ImTextureID>(Texture->TextureSRV),
        ImVec2(Size.X, Size.Y)
    );

    EndWidget();

}

void UUIPanelComponent::SetTexture(const FWString& InPath)
{
    TexturePath = FString(InPath.c_str());
    Texture = FEngineLoop::ResourceManager.GetTexture(InPath);
}
std::shared_ptr<FTexture> UUIPanelComponent::GetTexture()
{
    return Texture;
}

void UUIPanelComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("TexturePath"), TexturePath);
}

void UUIPanelComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    const FString* TempStr = InProperties.Find(TEXT("TexturePath"));
    if (TempStr)
    {
        SetTexture(TempStr->ToWideString());
    }
}
