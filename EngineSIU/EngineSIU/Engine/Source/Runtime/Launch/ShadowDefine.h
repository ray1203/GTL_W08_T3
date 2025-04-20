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

struct FSpotLightShadowData
{
    FMatrix SpotLightViewProj;
    float ShadowBias;
    FVector _padding;
};
    
