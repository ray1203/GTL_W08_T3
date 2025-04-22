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

struct ShadowSettingData
{
    float ShadowBias;
    FVector _padding;
};


struct FDepthMapData
{
    FMatrix ViewProj;
    FVector4 Params;
};

struct  FDirectionalLightViewProj
{
    FMatrix DirectionalLightView;
    FMatrix DirectionalLightProj;
};

