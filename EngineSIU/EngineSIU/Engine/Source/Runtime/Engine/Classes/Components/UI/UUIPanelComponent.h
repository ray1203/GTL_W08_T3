#pragma once

#include "Components/UI/UUIComponent.h"

class UUIPanelComponent : public UUIComponent
{
    DECLARE_CLASS(UUIPanelComponent, UUIComponent)

public:
    UUIPanelComponent();
    virtual ~UUIPanelComponent() = default;

    virtual void TickComponent(float DeltaTime) override;
    virtual void RenderUI() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    virtual void SetProperties(const TMap<FString, FString>& InProperties) override;

    void SetTexture(const FWString& InPath);
    std::shared_ptr<FTexture> GetTexture();
    FString GetTexturePath() const { return TexturePath; }

private:
    std::shared_ptr<FTexture> Texture = nullptr;
    FString TexturePath = TEXT("Contents/Texture/bggo.png");
};
