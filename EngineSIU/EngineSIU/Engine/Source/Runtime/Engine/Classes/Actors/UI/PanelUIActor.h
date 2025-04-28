#pragma once

#include "GameFramework/Actor.h"

enum class EUIAnchor : uint8;
class UUIPanelComponent;

class APanelUIActor : public AActor
{
    DECLARE_CLASS(APanelUIActor, AActor)

public:
    APanelUIActor();

    void SetTexture(const FWString& Path);
    void SetSize(float Width, float Height);
    void SetOffset(float X, float Y);
    void SetAnchor(EUIAnchor Anchor);

protected:
    UPROPERTY(UUIPanelComponent*, PanelComponent, = nullptr);
};
