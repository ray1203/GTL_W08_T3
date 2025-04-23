
#define MAX_LIGHTS 16 

#define MAX_DIRECTIONAL_LIGHT 16
#define MAX_POINT_LIGHT 16
#define MAX_SPOT_LIGHT 16
#define MAX_AMBIENT_LIGHT 16

#define POINT_LIGHT         1
#define SPOT_LIGHT          2
#define DIRECTIONAL_LIGHT   3
#define AMBIENT_LIGHT       4

#define PI 3.14159

struct FAmbientLightInfo
{
    float4 AmbientColor;
};

struct FDirectionalLightInfo
{
    float4 LightColor;

    float3 Direction;
    float Intensity;
    row_major matrix LightView;
    row_major matrix LightProj;
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
    row_major matrix PointLightViewProj[6];
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
    row_major matrix SpotLightViewProj;
};

cbuffer Lighting : register(b0)
{   
    int DirectionalLightsCount;
    int PointLightsCount;
    int SpotLightsCount;
    int AmbientLightsCount;
};

StructuredBuffer<FAmbientLightInfo> AmbientLights : register(t75);
StructuredBuffer<FDirectionalLightInfo> DirectionalLights : register(t76);
StructuredBuffer<FPointLightInfo> PointLights : register(t77);
StructuredBuffer<FSpotLightInfo> SpotLights : register(t78);

// 3종류 Light에 대해서 t10부터 16개씩 texture 할당

// Directional
Texture2D<float> DirectionalShadowTexture[MAX_DIRECTIONAL_LIGHT] : register(t10);
// Spot
Texture2D<float> SpotShadowTexture[MAX_SPOT_LIGHT] : register(t26);
Texture2D<float2> SpotVSMTexture[MAX_SPOT_LIGHT] : register(t42); // For VSM (Example register)
SamplerState VSM_Sampler : register(s11);
// Point
TextureCube<float> PointShadowCube[MAX_POINT_LIGHT] : register(t58);
SamplerComparisonState ShadowSampler : register(s10);


//TODO ShadowSetting해주삼
cbuffer ShadowSettingData : register(b6)
{
    float ShadowBias;
    float VSM_MinVariance; // Minimum variance to prevent division by zero in VSM
    float VSM_LightBleedReduction; // Factor to reduce light bleeding [0, 1]
    int FilterType;
}


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

float SampleSpotShadow(int Index, float3 worldPos)
{
    float4 lightSpacePos = mul(float4(worldPos, 1.0f), SpotLights[Index].SpotLightViewProj);
    float3 projCoords = lightSpacePos.xyz / lightSpacePos.w; // 원근 나누기
    float2 uv = projCoords.xy * 0.5f + 0.5f; // UV 좌표 [0, 1]
    uv.y = 1.0f - uv.y; // Y축 뒤집기
    
    float currentDepth = projCoords.z;
    
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f || currentDepth > 1.0f)
    {
        return 1.0f;
    }
    
    float shadowFactor = 1.0f;

    if (FilterType == 1) // VSM 필터링
    {
        float2 moments = SpotVSMTexture[Index].Sample(VSM_Sampler, uv);
        
        float variance = moments.y - (moments.x * moments.x);
        
        variance = max(variance, VSM_MinVariance);

        float delta = currentDepth - moments.x; // 바이어스 없는 깊이 사용

        float litFactor = variance / (variance + delta * delta); // 체비셰프 부등식

        // 빛 번짐 감소
        litFactor = (delta <= ShadowBias) ? 1.0f : lerp(litFactor, 0.0f, VSM_LightBleedReduction);

        shadowFactor = saturate(litFactor);
    }
    else // PCF 필터링
    {
        float depthForComparison = currentDepth - ShadowBias;

        float shadowAccum = 0.0f;
        uint width, height;
        SpotShadowTexture[Index].GetDimensions(width, height);
        float2 texelSize = float2(1.0 / width, 1.0 / height);

        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            [unroll]
            for (int y = -1; y <= 1; ++y)
            {
                float2 offset = float2(x, y) * texelSize;
                shadowAccum += SpotShadowTexture[Index].SampleCmpLevelZero(
                    ShadowSampler,
                    uv + offset,
                    depthForComparison 
                );
            }
        }
        shadowFactor = shadowAccum / 9.0f;
    }
    return shadowFactor;
}
// ——— 그림자 샘플링 헬퍼 ———
float SamplePointShadow(int Index, float3 ToLight, float3 worldPos)
{
    int faceIdx = 0;
    float3 absToL = abs(ToLight);
    if (absToL.x > absToL.y && absToL.x > absToL.z)
        faceIdx = (ToLight.x > 0) ? 1 : 0;
    else if (absToL.y > absToL.z)
        faceIdx = (ToLight.y > 0) ? 3 : 2;
    else
        faceIdx = (ToLight.z > 0) ? 5 : 4;
    
    // 뷰·프로젝션 변환
    float4 proj = mul(float4(worldPos, 1), PointLights[Index].PointLightViewProj[faceIdx]);
    proj.xyz /= proj.w;
    
    float depth = proj.z - ShadowBias;
    float shadow = 0.0f;
    
    // Begin Sphere PCF Sampling
    float3 sampleDirections[20];
    
    for (uint i = 0; i < 20; i++)
    {
        float offset = 0.002 * (i / 20.0);
        
        float theta = i * (PI * 2) / 20;
        sampleDirections[i] = normalize(ToLight) +
                             float3(cos(theta) * offset,
                                    sin(theta) * offset,
                                    (i % 5) * 0.001 - 0.002);
    }
    
    for (i = 0; i < 20; i++)
    {
        shadow += PointShadowCube[Index].SampleCmpLevelZero(
            ShadowSampler,
            normalize(-1.0f * sampleDirections[i]),
            depth
        );
    }
    
    return shadow / 20.0;
    
    // End Sphere PCF Sampling
    
    //// Begin Disk PCF Sampling
    //float diskRadius = 0.005;
    
    //float3 lightDir = normalize(ToLight);
    //float3 tangent, bitangent;
    
    //if (abs(lightDir.x) < 0.99)
    //    tangent = normalize(cross(float3(1, 0, 0), lightDir));
    //else
    //    tangent = normalize(cross(float3(0, 1, 0), lightDir));
        
    //bitangent = normalize(cross(lightDir, tangent));
    
    //for (int i = 0; i < 12; i++)
    //{
    //    float angle = i * (PI * 2) / 12;
    //    float r = diskRadius * (i % 3) / 3.0; 
        
    //    float3 offset = tangent * (r * cos(angle)) +
    //                   bitangent * (r * sin(angle));
                        
    //    float3 sampleDir = normalize(lightDir + offset);
        
    //    float4 proj = mul(float4(worldPos, 1), PointLightViewProj[faceIdx]);
    //    proj.xyz /= proj.w;
    //    float depth = proj.z - ShadowBias;
        
    //    shadow += ShadowCube.SampleCmpLevelZero(
    //        ShadowSampler,
    //        -sampleDir,
    //        depth
    //    );
    //}
    
    //return shadow / 12.0;
    //// End Disk PCF Sampling
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

    float shadow = SamplePointShadow(Index, ToLight, WorldPosition);
    
    //return float4(shadow, 0, 0, 1);
    
    return float4(Lit * Attenuation * LightInfo.Intensity * shadow, 1.0);
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
    float shadow = SampleSpotShadow(Index, WorldPosition);
    //float shadow = SampleSpotShadow(WorldNormal, WorldPosition, LightInfo.Position, LightInfo.Direction, LightInfo.OuterRad);
    
    return float4(Lit * Attenuation * SpotlightFactor * LightInfo.Intensity * shadow, 1.0);
    //return float4(shadow, shadow, shadow, 1.0);
}

float4 DirectionalLight(int nIndex, float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    FDirectionalLightInfo LightInfo = DirectionalLights[nIndex];
    
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
bool InRange(float val, float min, float max)
{
    return (min <= val && val <= max);
}

float GetLightFromShadowMap(int idx, float3 WorldPos, float3 WorldNorm)
{
    float NdotL = dot(normalize(WorldNorm), DirectionalLights[idx].Direction);
    
    float slopeScale = 0.5f;
    float minBias = 0.0001f;
    float maxBias = 0.001f;
    float bias = saturate(slopeScale * (1 - NdotL));
    bias = clamp(bias, minBias, maxBias);

    float angle = acos(NdotL);
    bias *= pow(angle / (0.5f * 3.14159265f), 2.0f);
    

    float4 lp = mul(float4(WorldPos, 1), DirectionalLights[idx].LightView);
    float4 cp = mul(lp, DirectionalLights[idx].LightProj);
    
    float2 uv = cp.xy / cp.w * 0.5f + 0.5f;
    float dist = cp.z / cp.w - bias;
    uv.y = 1 - uv.y;
    uv = saturate(uv);
    
    const int R = 1;
    const float kernel[3][3] =
    {
        { 1, 2, 1 },
        { 2, 4, 2 },
        { 1, 2, 1 }
    };
    float sum = 0, weightSum = 16;
    for (int i = -R; i <= R; ++i)
    {
        for (int j = -R; j <= R; ++j)
        {
            float2 off = uv + float2(i, j) * (1.0f / 2048);
            float s = InRange(off.x, 0, 1) && InRange(off.y, 0, 1)
                ? DirectionalShadowTexture[idx].SampleCmpLevelZero(ShadowSampler, off, dist).r
                : 1.0f;
            sum += s * kernel[i + R][j + R];
        }
    }
    float Light = 0;
    Light = sum / weightSum;
    return Light;

    
   // // Ambient 및 감마 보정
   // //const float ambient = 1.f;
   //// Light = pow(Light, 1/2.2f);
   // return Light;
}

float4 Lighting(float3 WorldPosition, float3 WorldNormal, float3 WorldViewPosition, float3 DiffuseColor)
{
    float4 FinalColor = float4(0.0, 0.0, 0.0, 0.0);
    
    // 다소 비효율적일 수도 있음.
    [unroll(MAX_POINT_LIGHT)]
    for (int i = 0; i < PointLightsCount; i++)
    {
        FinalColor += PointLight(i, WorldPosition, WorldNormal, WorldViewPosition, DiffuseColor);
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
        FinalColor += float4(AmbientLights[l].AmbientColor.rgb, 0.0);
        FinalColor.a = 1.0;
    }
    
    [unroll(MAX_DIRECTIONAL_LIGHT)]
    for (k = 0; k < DirectionalLightsCount; k++)
    {
        FinalColor *= GetLightFromShadowMap(k, WorldPosition, WorldNormal);
    }
    return FinalColor;
}
