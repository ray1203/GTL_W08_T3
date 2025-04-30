#include "CameraModifier_CameraShake.h"
#include "Curve/CurveManager.h"

const UCameraModifier_CameraShake::FShakeProperty UCameraModifier_CameraShake::ShakePropertyNames[7] = {
    { UCameraModifier_CameraShake::PositionX, TEXT("X") },
    { UCameraModifier_CameraShake::PositionY, TEXT("Y") },
    { UCameraModifier_CameraShake::PositionZ, TEXT("Z") },
    { UCameraModifier_CameraShake::Pitch,     TEXT("Pitch") },
    { UCameraModifier_CameraShake::Yaw,       TEXT("Yaw") },
    { UCameraModifier_CameraShake::Roll,      TEXT("Roll") },
    { UCameraModifier_CameraShake::FOV,       TEXT("FOV") },
};


UCameraModifier_CameraShake::UCameraModifier_CameraShake()
{
}

void UCameraModifier_CameraShake::SetShakeCurve(const FString& InCurveName, uint32 InTargetProperty, float InDuration, float InAmplitude)
{
    CurveName = InCurveName;
    Duration = InDuration;
    Amplitude = InAmplitude;
    TargetPropertiesMask = InTargetProperty;
}

void UCameraModifier_CameraShake::StartShake()
{
    bShaking = true;
}

void UCameraModifier_CameraShake::EnableModifier()
{
    Super::EnableModifier();
    StartShake();
}

void UCameraModifier_CameraShake::DisableModifier()
{
    Super::DisableModifier();
    bShaking = false;
    ElapsedTime = 0.f;
    OnTransitionEnd.Broadcast();
}

void UCameraModifier_CameraShake::OnAdded()
{
}

void UCameraModifier_CameraShake::OnRemoved()
{
}

bool UCameraModifier_CameraShake::ModifyCamera(float DeltaTime, FCameraParams& OutParams)
{
    Super::ModifyCamera(DeltaTime, OutParams);

    if (!bShaking)
    {
        return false;
    }

    ElapsedTime += DeltaTime;

    float T = FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f);
    if (T >= 1)
    {
        DisableModifier();
    }

    // Bit/Name/Setter 구조체 (Unreal 스타일)
    struct FShakePropertySetter
    {
        uint32 Bit;
        const TCHAR* Name;
        std::function<void(FCameraParams&, float)> Setter;
    };

    static const FShakePropertySetter ShakePropertySetters[] = {
        { UCameraModifier_CameraShake::PositionX, TEXT("X"), [](FCameraParams& P, float V) { P.Position.X += V; } },
        { UCameraModifier_CameraShake::PositionY, TEXT("Y"), [](FCameraParams& P, float V) { P.Position.Y += V; } },
        { UCameraModifier_CameraShake::PositionZ, TEXT("Z"), [](FCameraParams& P, float V) { P.Position.Z += V; } },
        { UCameraModifier_CameraShake::Pitch,     TEXT("Pitch"), [](FCameraParams& P, float V) { P.Rotation.Pitch += V; } },
        { UCameraModifier_CameraShake::Yaw,       TEXT("Yaw"), [](FCameraParams& P, float V) { P.Rotation.Yaw += V; } },
        { UCameraModifier_CameraShake::Roll,      TEXT("Roll"), [](FCameraParams& P, float V) { P.Rotation.Roll += V; } },
        { UCameraModifier_CameraShake::FOV,       TEXT("FOV"), [](FCameraParams& P, float V) { P.FOV += V; } },
    };

    // 각 property 비트가 켜져 있을 때만 수행
    for (const auto& Prop : ShakePropertySetters)
    {
        if ((TargetPropertiesMask & Prop.Bit) != 0)
        {
            FCurveData* Curve = FCurveManager::Get().GetCurve(CurveName, Prop.Name);
            if (Curve)
            {
                float Val = Curve->EvaluateSmooth(T) * Amplitude;
                Prop.Setter(OutParams, Val);
            }
        }
    }

    return true;
}


