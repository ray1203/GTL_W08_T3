#pragma once
#include "GameFramework/Actor.h"

class UUITextComponent;

class ATextUIActor : public AActor
{
    DECLARE_CLASS(ATextUIActor, AActor)

public:
    ATextUIActor();

    void SetText(const std::string& InText);

protected:
    UPROPERTY(UUITextComponent*, TextComponent, = nullptr);
};
