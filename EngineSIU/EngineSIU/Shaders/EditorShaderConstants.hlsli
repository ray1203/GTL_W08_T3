struct AABBData
{
    float3 Position;
    float Padding1;
    
    float3 Extent;
    float Padding2;
};
cbuffer ConstantBufferDebugAABB : register(b11)
{
    AABBData DataAABB[8];
}

struct SphereData
{
    float3 Position;
    float Radius;
};

cbuffer ConstantBufferDebugSphere : register(b11)
{
    SphereData DataSphere[8];
}

struct ConeData
{
    float3 ApexPosiiton;
    float Radius;
    float3 Direction;
    float Angle;
    float4 Color;
};

cbuffer ConstantBufferDebugCone : register(b11)
{
    ConeData DataCone[16];
}

cbuffer ConstantBufferDebugGrid : register(b11)
{
    float3 GridOrigin; // Grid의 중심
    float GridSpacing;
    int GridCount; // 총 grid 라인 수
    float GridColor;
    float GridAlpha;
    float GridPadding;
};

struct IconData
{
    float3 IconPosition;
    float IconScale;
    float4 IconColor;
};
cbuffer ConstantBufferDebugIcon : register(b11)
{
    IconData IconDatas[16];
}

cbuffer ConstantBufferDebugArrow : register(b11)
{
    float3 ArrowPosition;
    float ArrowScaleXYZ;
    float3 ArrowDirection;
    float ArrowScaleZ;
    float4 ArrowColor;
}
