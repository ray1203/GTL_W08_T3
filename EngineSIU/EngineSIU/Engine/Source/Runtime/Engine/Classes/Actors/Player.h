#pragma once
#include "GameFramework/Actor.h"

class APlayer : public AActor
{
    DECLARE_CLASS(APlayer, AActor)
    APlayer() = default;

    virtual void Tick(float DeltaTime) override;
};
