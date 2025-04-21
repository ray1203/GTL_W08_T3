
#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16
#define MAX_AMBIENT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3
#define AMBIENT_LIGHT       4

struct FAmbientLightInfo
{
    float4 AmbientColor;
};

struct FDirectionalLightInfo
{
    float4 LightColor;

    float3 Direction;
    float Intensity;
};

struct FPointLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    int Type;
    float Intensity;
    float Attenuation;
    float Padding;
};

struct FSpotLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;

    float3 Direction;
    float Intensity;

    int Type;
    float InnerRad;
    float OuterRad;
    float Attenuation;
};

cbuffer Lighting : register(b0)
{
    FAmbientLightInfo Ambient[MAX_AMBIENT_LIGHT];
    FDirectionalLightInfo Directional[MAX_DIRECTIONAL_LIGHT];
    FPointLightInfo PointLights[MAX_POINT_LIGHT];
    FSpotLightInfo SpotLights[MAX_SPOT_LIGHT];
    
    int DirectionalLightsCount;
    int PointLightsCount;
    int SpotLightsCount;
    int AmbientLightsCount;
};

// 6개의 페이스별로 개별 바인딩(t10~t15)
Texture2D ShadowMap : register(t10);
SamplerComparisonState ShadowSampler : register(s10);

// Point Light 의 6개 face 뷰·프로젝션 행렬과 바이어스
cbuffer ShadowData : register(b6)
{
    row_major matrix SpotLightViewProj;
    float ShadowBias; // 깊이 비교 시 바이어스
    float3 _padding; // 16바이트 정렬용
};

float CalculateAttenuation(float Distance, float AttenuationFactor, float Radius)
{
    if (Distance > Radius)
    {
        return 0.0;
    }

    float Falloff = 1.0 / (1.0 + AttenuationFactor * Distance * Distance);
    float SmoothFactor = (1.0 - (Distance / Radius)); // 부드러운 falloff

    return Falloff * SmoothFactor;
}

float CalculateSpotEffect(float3 LightDir, float3 SpotDir, float InnerRadius, float OuterRadius, float SpotFalloff)
{
    float Dot = dot(-LightDir, SpotDir); // [-1,1]
    
    float SpotEffect = smoothstep(cos(OuterRadius / 2), cos(InnerRadius / 2), Dot);
    
    return SpotEffect * pow(max(Dot, 0.0), SpotFalloff);
}

float CalculateDiffuse(float3 WorldNormal, float3 LightDir)
{
    return max(dot(WorldNormal, LightDir), 0.0);
}

float CalculateSpecular(float3 WorldNormal, float3 ToLightDir, float3 ViewDir, float Shininess, float SpecularStrength = 0.5)
{
#ifdef LIGHTING_MODEL_GOURAUD
    float3 ReflectDir = reflect(-ToLightDir, WorldNormal);
    float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess);
#else
    float3 HalfDir = normalize(ToLightDir + ViewDir); // Blinn-Phong
    float Spec = pow(max(dot(WorldNormal, HalfDir), 0.0), Shininess);
#endif
    return Spec * SpecularStrength;
}

 //Begin Test
//float SampleSpotShadow(float3 worldNormal, float3 worldPos, float3 spotLightPos, float3 spotLightDir, float spotOuterAngle)
//{
//    float3 toFragment = worldPos - spotLightPos;
//    float distToFragment = length(toFragment);
//    toFragment = normalize(toFragment);
    
//    float cosAngle = dot(toFragment, normalize(spotLightDir));
//    float cosOuterCone = cos(spotOuterAngle / 2);
    
//    if (cosAngle < cosOuterCone)
//        return 0.0f;
    
//    float4 lightSpacePos = mul(float4(worldPos, 1.0f), SpotLightViewProj);
    
//    lightSpacePos.xy /= lightSpacePos.w;
//    float2 uv = lightSpacePos.xy / lightSpacePos.w * 0.5f + 0.5f;
//    uv.y = 1 - uv.y;
    
//    // 이거는 사실 큰 의미가 없어보이긴 하는데
//    float bias = max(0.01 * (1.0 - dot(worldNormal, spotLightDir)), ShadowBias);
//    float depth = lightSpacePos.z / lightSpacePos.w - bias;
    
//    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
//        return 1.0f;
    
//    //float shadow = ShadowMap.SampleCmpLevelZero(
//    //    ShadowSampler,
//    //    uv,
//    //    depth
//    //);
    
//    //return shadow;
    
//    float shadow = 0.0f;
//    uint width, height;
//    ShadowMap.GetDimensions(width, height);
//    float2 texelSize = float2(1.0 / width, 1.0 / height);

//    for (int x = -1; x <= 1; ++x)
//    {
//        for (int y = -1; y <= 1; ++y)
//        {
//            float2 offset = float2(x, y) * texelSize;
//            shadow += ShadowMap.SampleCmpLevelZero(
//                ShadowSampler,
//                uv.xy + offset,
//                depth
//            );
//        }
//    }
//    shadow /= 9.0;
//    return shadow;
//}
// End Test

float SampleSpotShadow(float3 worldPos, float3 spotLightPos, float3 spotLightDir, float spotOuterAngle)
{
    //float3 toFragment = worldPos - spotLightPos;
    //float distToFragment = length(toFragment);
    //toFragment = normalize(toFragment);
    
    //float cosAngle = dot(toFragment, normalize(spotLightDir));
    //float cosOuterCone = cos(spotOuterAngle / 2);
    
    //if (cosAngle < cosOuterCone)
    //    return 0.0f;
    
    float4 lightSpacePos = mul(float4(worldPos, 1.0f), SpotLightViewProj);
    
    float2 uv = lightSpacePos.xy / lightSpacePos.w * 0.5f + 0.5f;
    uv.y = 1 - uv.y;
    float depth = lightSpacePos.z / lightSpacePos.w - ShadowBias;
    
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
        return 1.0f;
    
    // pcf
    float shadow = 0.0f;
    uint width, height;
    ShadowMap.GetDimensions(width, height);
    float2 texelSize = float2(1.0 / width, 1.0 / height);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            shadow += ShadowMap.SampleCmpLevelZero(
                ShadowSampler,
                uv.xy + offset,
                depth
            );
        }
    }
    shadow /= 9.0;
    return shadow;
}

float4 PointLight(int Index, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FPointLightInfo LightInfo = PointLights[Index];
    
    float3 ToLight = LightInfo.Position - WorldPosition;
    float Distance = length(ToLight);
    
    float Attenuation = CalculateAttenuation(Distance, LightInfo.Attenuation, LightInfo.Radius);
    if (Attenuation <= 0.0)
    {
        return float4(0.f, 0.f, 0.f, 0.f);
    }
    
    float3 LightDir = normalize(ToLight);
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);
#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = (DiffuseFactor * DiffuseColor) * LightInfo.LightColor.rgb;
#else
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif
    
    return float4(Lit * Attenuation * LightInfo.Intensity, 1.0);
}

float4 SpotLight(int Index, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FSpotLightInfo LightInfo = SpotLights[Index];
    
    float3 ToLight = LightInfo.Position - WorldPosition;
    float Distance = length(ToLight);
    
    float Attenuation = CalculateAttenuation(Distance, LightInfo.Attenuation, LightInfo.Radius);
    if (Attenuation <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    
    float3 LightDir = normalize(ToLight);
    float SpotlightFactor = CalculateSpotEffect(LightDir, normalize(LightInfo.Direction), LightInfo.InnerRad, LightInfo.OuterRad, LightInfo.Attenuation);
    if (SpotlightFactor <= 0.0)
    {
        return float4(0.0, 0.0, 0.0, 0.0);
    }
    
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);

#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = DiffuseFactor * DiffuseColor * LightInfo.LightColor.rgb;
#else
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif
    
    //return float4(Lit * Attenuation * SpotlightFactor * LightInfo.Intensity, 1.0);
    float shadow = SampleSpotShadow(WorldPosition, LightInfo.Position, LightInfo.Direction, LightInfo.OuterRad);
    //float shadow = SampleSpotShadow(WorldNormal, WorldPosition, LightInfo.Position, LightInfo.Direction, LightInfo.OuterRad);
    
    return float4(Lit * Attenuation * SpotlightFactor * LightInfo.Intensity * shadow, 1.0);
    //return float4(shadow, shadow, shadow, 1.0);
}

float4 DirectionalLight(int nIndex, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FDirectionalLightInfo LightInfo = Directional[nIndex];
    
    float3 LightDir = normalize(-LightInfo.Direction);
    float3 ViewDir = normalize(WorldViewPosition - WorldPosition);
    float DiffuseFactor = CalculateDiffuse(WorldNormal, LightDir);
    
#ifdef LIGHTING_MODEL_LAMBERT
    float3 Lit = DiffuseFactor * DiffuseColor * LightInfo.LightColor.rgb;
#else
    float SpecularFactor = CalculateSpecular(WorldNormal, LightDir, ViewDir, Material.SpecularScalar);
    float3 Lit = ((DiffuseFactor * DiffuseColor) + (SpecularFactor * Material.SpecularColor)) * LightInfo.LightColor.rgb;
#endif
    return float4(Lit * LightInfo.Intensity, 1.0);
}

float4 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    float4 FinalColor = float4(0.0, 0.0, 0.0, 0.0);
    
    // 다소 비효율적일 수도 있음.
    [unroll]
    for (int i = 0; i < MAX_POINT_LIGHT; i++)
    {
        if (i < PointLightsCount)
        {
            FinalColor += PointLight(i, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
        }
    }
    
    [unroll(MAX_SPOT_LIGHT)]
    for (int j = 0; j < SpotLightsCount; j++)
    {
       FinalColor += SpotLight(j, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }
    
    [unroll(MAX_DIRECTIONAL_LIGHT)]
    for (int k = 0; k < DirectionalLightsCount; k++)
    {
        FinalColor += DirectionalLight(k, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
    }
    [unroll(MAX_AMBIENT_LIGHT)]
    for (int l = 0; l < AmbientLightsCount; l++)
    {
        FinalColor += float4(Ambient[l].AmbientColor.rgb, 0.0);
        FinalColor.a = 1.0;
    }
    
    return FinalColor;
}
