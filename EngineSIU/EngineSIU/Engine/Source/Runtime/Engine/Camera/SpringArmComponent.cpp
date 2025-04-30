#include "SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "Launch/EngineLoop.h"
#include "World/World.h"

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
    Super::InitializeComponent();

}

void USpringArmComponent::BeginPlay()
{
    // SpringArmComponent는 BeginPlay에서 카메라 찾거나 생성
    Super::BeginPlay();
    AActor* Owner = GetOwner();
    if (Owner == nullptr)
    {
        UE_LOG(ELogLevel::Error, TEXT("SpringArmComponent With No Owner"));
        return;
    }
    Camera = Owner->GetComponentByClass<UCameraComponent>();
    if (Camera != nullptr)
    {
        Camera->bShouldAttachedToViewport = true;
        bIsCameraAttached = true;
    }

    // 에디터에서는 인풋바인딩 하지 않는다
    UWorld* World = GetWorld();
    if (World && World->WorldType != EWorldType::Editor)
    {
        MouseInputHandle = GEngineLoop.GetAppMessageHandler()->OnRawMouseInputDelegate.AddDynamic(this, &USpringArmComponent::OnRawMouseInput);
    }
}

void USpringArmComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (MouseInputHandle.has_value() && MouseInputHandle->IsValid())
    {
        GEngineLoop.GetAppMessageHandler()->OnRawMouseInputDelegate.Remove(*MouseInputHandle);
        MouseInputHandle.reset();
    }
}

void USpringArmComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
    UpdateCameraTransform(DeltaTime);
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
	return SocketOffset;
}

void USpringArmComponent::AttachCamera(UCameraComponent* InCamera)
{
    if (InCamera == nullptr)
        return;

    if (Camera != InCamera)
    {
        //원래 카메라와 다른 카메라가 들어오면 기존 카메라 날리고 새로운 카메라로 세팅
        GetOwner()->RemoveOwnedComponent(Camera);
        // !TODO : 기존재하는 컴포넌트를 액터에 붙이는 API 필요함
        //GetOwner()->AddComponent(InCamera);
        //Camera = InCamera;

        //Camera->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
    }

    bIsCameraAttached = true;
}

void USpringArmComponent::DetachCamera()
{
    // 플래그만 false처리해주고, 원래 카메라 참조는 유지
    bIsCameraAttached = false;
}

void USpringArmComponent::OnRawMouseInput(const FPointerEvent& InEvent)
{
    FVector2D MouseDelta = InEvent.GetCursorDelta();
    HandleRotation(MouseDelta);
}

void USpringArmComponent::HandleRotation(const FVector2D& Vector)
{
    if (!Camera || !Camera->IsAttachedToViewport())
        return;
    //return;
    float yaw = Vector.X;
    float pitch = -Vector.Y;

    AActor* Owner = GetOwner();
    if (Owner)
    {
        USceneComponent* RootComp = Owner->GetRootComponent();
        if (RootComp)
        {
            RootComp->Rotate(FRotator(0, yaw / 10.f, 0));
            CurrentPitchAngle = FMath::Clamp(CurrentPitchAngle + pitch / 10.f, -90.f, 70.f);
            //SocketOffset.Z = FMath::Sin(FMath::DegreesToRadians(CurrentPitchAngle) * TargetArmLength);
        }
    }
}

void USpringArmComponent::UpdateCameraTransform(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Camera)
        return;

    if (!Camera->IsAttachedToViewport())
        return;

    if (!bIsCameraAttached)
        return;

    FVector BaseLocation = Owner->GetActorLocation();
    FRotator BaseRotation = Owner->GetActorRotation();

    FVector DesiredLocation = Owner->GetActorLocation()
        - (Owner->GetActorForwardVector() * TargetArmLength * FMath::Cos(FMath::DegreesToRadians(CurrentPitchAngle)))
        + FVector(0, 0, FMath::Sin(FMath::DegreesToRadians(CurrentPitchAngle)) * TargetArmLength)
        + SocketOffset;

    TargetLocation = FMath::Lerp(DesiredLocation, TargetLocation, DeltaTime);

    FHitResult Hit;
	// fixme
  //  if (GetWorld()->LineTraceSingle(Hit, BaseLocation, TargetLocation))
  //  {
		//if (Hit.Component->GetOwner() == Owner)
		//	return;
  //      //UE_LOG(ELogLevel::Warning, TEXT("SpringArm Hit Length : %d"), Hit.Distance);
  //      TargetLocation = Hit.ImpactPoint;
  //  }

    FVector LookDirection = Owner->GetActorLocation() - TargetLocation;
    LookDirection.Normalize();
    FRotator TargetLookRotation = LookDirection.Rotation();


    Camera->SetWorldLocation(TargetLocation);
    Camera->SetWorldRotation(TargetLookRotation);
}
