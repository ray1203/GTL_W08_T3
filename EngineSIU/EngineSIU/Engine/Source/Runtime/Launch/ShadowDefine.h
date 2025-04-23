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
    float VSM_MinVariance; // Minimum variance to prevent division by zero in VSM
    float VSM_LightBleedReduction; // Factor to reduce light bleeding [0, 1]
    int FilterType;
};


struct FDepthMapData
{
    FMatrix ViewProj;
    FVector4 Params;
};

enum EShadowFilter : uint8
{
    ESF_StandardPCF,
    ESF_VSM,
    ESF_MAX,
};
