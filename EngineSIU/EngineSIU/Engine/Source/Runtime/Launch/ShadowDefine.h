#pragma once
#include "Math/Matrix.h"

struct FShadowViewProj
{
    FMatrix ViewProj;
};

struct FShadowObjWorld
{
    FMatrix World;
};

struct FShadowSettingData
{
    float ShadowBias;
    FVector _padding;
};


struct FDepthMapData
{
    FMatrix ViewProj;
    FVector4 Params;
};
