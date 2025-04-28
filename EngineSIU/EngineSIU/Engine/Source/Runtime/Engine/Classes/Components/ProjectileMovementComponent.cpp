#include "ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"

UProjectileMovementComponent::UProjectileMovementComponent()
{
    InitialSpeed = 0;
    MaxSpeed = 0;
    Acceleration = FVector(0.f, 0.f, -20.f);
    Velocity = FVector(0.f, 0.f, 0.f);
    AccumulatedTime = 0;
}

UProjectileMovementComponent::~UProjectileMovementComponent()
{
}

UObject* UProjectileMovementComponent::Duplicate(UObject* InOuter)
{

    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->AccumulatedTime = AccumulatedTime;
    NewComponent->InitialSpeed = InitialSpeed;
    NewComponent->MaxSpeed = MaxSpeed;
    NewComponent->Acceleration = Acceleration;
    NewComponent->Velocity = Velocity;

    return NewComponent;
    
}

void UProjectileMovementComponent::BeginPlay()
{
    FVector Forward = GetOwner()->GetActorForwardVector();
    //Velocity = Forward * InitialSpeed;
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    // UProjectileMovementComponent는 물리 계산, 이동을 하지 않고 property만 가집니다
    // 물리 연산은 FPhysicsSolver에서 처리합니다.
}

void UProjectileMovementComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("AccumulatedTime"), FString::Printf(TEXT("%f"), AccumulatedTime));
    OutProperties.Add(TEXT("InitialSpeed"), FString::Printf(TEXT("%f"), InitialSpeed));
    OutProperties.Add(TEXT("MaxSpeed"), FString::Printf(TEXT("%f"), MaxSpeed));
    OutProperties.Add(TEXT("Acceleration"), Acceleration.ToString());
    OutProperties.Add(TEXT("Velocity"), Velocity.ToString());
}

void UProjectileMovementComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("AccumulatedTime"));
    if (TempStr)
    {
        AccumulatedTime = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("InitialSpeed"));
    if (TempStr)
    {
        InitialSpeed = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("MaxSpeed"));
    if (TempStr)
    {
        MaxSpeed = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Acceleration"));
    if (TempStr)
    {
        Acceleration.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Velocity"));
    if (TempStr)
    {
        Velocity.InitFromString(*TempStr);
    }
    
}
