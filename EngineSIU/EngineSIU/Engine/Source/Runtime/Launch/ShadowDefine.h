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

struct FPointLightShadowData
{
    FMatrix PointLightViewProj[6];
    float ShadowBias;
    FVector _padding;
};
    
