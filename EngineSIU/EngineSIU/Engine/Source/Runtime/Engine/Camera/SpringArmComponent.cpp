#include "SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Launch/EngineLoop.h"

USpringArmComponent::USpringArmComponent()
{

}

UObject* USpringArmComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewComponent->TargetArmLength = TargetArmLength;
    NewComponent->SocketOffset = SocketOffset;
    return NewComponent;
}

void USpringArmComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
	Super::GetProperties(OutProperties);
	// 카메라의 FOV, AspectRatio, NearClip, FarClip을 OutProperties에 추가
	OutProperties.Add(TEXT("TargetArmLength"), FString::Printf(TEXT("%f"), TargetArmLength));
	OutProperties.Add(TEXT("SocketOffset"), *SocketOffset.ToString());
}

void USpringArmComponent::SetProperties(const TMap<FString, FString>& InProperties)
{// 카메라의 FOV, AspectRatio, NearClip, FarClip을 InProperties에서 읽어와 설정
	Super::SetProperties(InProperties);
	const FString* TempStr = nullptr;
	TempStr = InProperties.Find(TEXT("TargetArmLength"));
	if (TempStr) TargetArmLength = FCString::Atof(**TempStr);
	TempStr = InProperties.Find(TEXT("SocketOffset"));
	if (TempStr) SocketOffset.InitFromString(*TempStr);
}

void USpringArmComponent::InitializeComponent()
{
	// !TODO : Input 시스템 찾아서 바인드
    MouseInputHandle = GEngineLoop.GetAppMessageHandler()->OnRawMouseInputDelegate.AddDynamic(this, &USpringArmComponent::OnRawMouseInput);
}

void USpringArmComponent::BeginPlay()
{
    // SpringArmComponent는 BeginPlay에서 카메라 찾거나 생성
    AActor* Owner = GetOwner();
    if (Owner == nullptr)
    {
        UE_LOG(ELogLevel::Error, TEXT("SpringArmComponent With No Owner"));
        return;
    }
    Camera = Owner->GetComponentByClass<UCameraComponent>();
    if (Camera == nullptr)
    {
        Camera = Owner->AddComponent<UCameraComponent>(TEXT("Camera"));
        Camera->SetupAttachment(this);
    }
}

void USpringArmComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    //if ()
}

void USpringArmComponent::TickComponent(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || Camera == nullptr)
		return;

	FVector DesiredLocation = Owner->GetActorLocation() - Owner->GetActorForwardVector() * TargetArmLength + SocketOffset;
	TargetLocation = FMath::Lerp(DesiredLocation, TargetLocation, 5.f * DeltaTime);

	Camera->SetWorldLocation(TargetLocation);
	//Camera->LookAt
}

void USpringArmComponent::SetTargetArmLength(float InLength)
{
	TargetArmLength = InLength;
}

float USpringArmComponent::GetTargetArmLength() const
{
	return TargetArmLength;
}

void USpringArmComponent::SetSocketOffset(const FVector& InOffset)
{
	SocketOffset = InOffset;
}

FVector USpringArmComponent::GetSocketOffset() const
{
	return FVector();
}

void USpringArmComponent::OnRawMouseInput(const FPointerEvent& InEvent)
{
    FVector2D MouseDelta = InEvent.GetCursorDelta();
    HandleRotation(MouseDelta);
}

void USpringArmComponent::HandleRotation(const FVector2D Vector)
{
    float yaw = Vector.X;
    float pitch = Vector.Y;

    AActor* Owner = GetOwner();
    if (Owner)
    {
        USceneComponent* RootComp = Owner->GetRootComponent();
        if (RootComp)
        {
            // yaw는 돌린다
            RootComp->Rotate(FRotator(0, yaw, 0));
            // Pitch는?

        }
    }
}
