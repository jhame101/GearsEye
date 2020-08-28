// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MotionControllerComponent.h" 	
#include "SRanipal_FunctionLibrary_Eye.h"

// Sets default values
AVRPawn::AVRPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Component setup
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	SetRootComponent(DefaultSceneRoot);

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(RootComponent);

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VR Root"));
	VRRoot->SetupAttachment(RootComponent);

	HeadsetCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Headset Camera"));
	HeadsetCamera->SetupAttachment(VRRoot);

	HeadLamp = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spot Light"));
	HeadLamp->SetupAttachment(HeadsetCamera);

	ExternalCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("External Camera"));
	ExternalCamera->SetupAttachment(HeadsetCamera);

	ViveController_L = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Left Vive Controller"));
	ViveController_L->SetupAttachment(VRRoot);
	ViveController_L->SetTrackingSource(EControllerHand::Left);

	InteractionComponent_L = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Left Interaction Component"));
	InteractionComponent_L->SetupAttachment(ViveController_L);
	InteractionComponent_L->bShowDebug = true;
	InteractionComponent_L->bAutoActivate = false;
	InteractionComponent_L->InteractionDistance = 1200;


	Laser_L = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Laser"));
	Laser_L->SetupAttachment(ViveController_L);
	Laser_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ViveController_R = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Right Vive Controller"));
	ViveController_R->SetupAttachment(VRRoot);
	ViveController_R->SetTrackingSource(EControllerHand::Right);

	InteractionComponent_R = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("RIght Interaction Component"));
	InteractionComponent_R->SetupAttachment(ViveController_R);
	InteractionComponent_R->bShowDebug = true;
	InteractionComponent_R->bAutoActivate = false;
	InteractionComponent_R->InteractionDistance = 1200;


	Laser_R = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Right Laser"));
	Laser_R->SetupAttachment(ViveController_R);
	Laser_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AVRPawn::BeginPlay()
{
	Super::BeginPlay();
	
	// Creating and applying the dynamic post process material
	DynamicPPM = UMaterialInstanceDynamic::Create(PostProcessMaterial, this);
	ExternalCamera->AddOrUpdateBlendable(DynamicPPM, 1000.f);

	// Setting the FOV of the external camera. NOTE: if you want to be able to adjust the FOV during play, move this somewhere else
	ExternalCameraFOV.X = ExternalCamera->FOVAngle;
	ExternalCameraFOV.Y = 2 * UKismetMathLibrary::Atan(UKismetMathLibrary::Tan(ExternalCameraFOV.X * UKismetMathLibrary::GetPI() / 180 / 2) * 9 / 16) * 180 / UKismetMathLibrary::GetPI();
	//Assuming 16:9 aspect ratio (which it is by default). Formula from https://www.reddit.com/r/Planetside/comments/1xl1z5/brief_table_for_calculating_fieldofview_vertical/ but also other places on the internet

}

void AVRPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateActorLocation();

	UpdateEyeTrackLocation();

	MoveSmoothly(DeltaTime);
}

#pragma region EyeTracking
FVector2D AVRPawn::GetGazeLocationOnScreen() const
{
	FVector EyeTrackOrigin, EyeTrackDirection;

	if (!ensure(ExternalCamera)) return FVector2D();
	if (!GetCombinedGazeRay(EyeTrackOrigin, EyeTrackDirection)) return FVector2D();

	FRotator RelativeAngle = (EyeTrackOrigin + 1000 * EyeTrackDirection).Rotation();	//Assuming that ExternalCamera has the same transform as HeadsetCamera
	FVector2D ScreenLocation;
	
	// Calculate UV location
	ScreenLocation.X = 0.5 + (RelativeAngle.Yaw / (ExternalCameraFOV.X * 1.275));
	ScreenLocation.Y = 0.5 - (RelativeAngle.Pitch / (ExternalCameraFOV.Y * 1.05));			//Not sure why the extra corrections are needed; maybe I have something wrong

	return ScreenLocation;
}

bool AVRPawn::GetCombinedGazeRay(FVector& EyeTrackOrigin, FVector& EyeTrackDirection) const
{
	if (USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::COMBINE, EyeTrackOrigin, EyeTrackDirection)) {
		return true;
	} else if (USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::LEFT, EyeTrackOrigin, EyeTrackDirection)) {
		return true;
	} else if (USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::RIGHT, EyeTrackOrigin, EyeTrackDirection)) {
		return true;
	} else return false;
}

void AVRPawn::UpdateEyeTrackLocation()
{
	FVector2D TwoDCoordinates = GetGazeLocationOnScreen();
	DynamicPPM->SetVectorParameterValue(TEXT("Centre Coordinates"), FLinearColor(TwoDCoordinates.X, TwoDCoordinates.Y, 0.f, 0.f));

	// DynamicPPM->SetScalarParameterValue(TEXT("Radius"), 0.03f);		// In case uncertainty is added later to control the radius
}
#pragma endregion EyeTracking

void AVRPawn::UpdateActorLocation()		// Makes sure that the actor is at the same place as the VR headset despite translation from the physical movement of the headset in the play space
{
	FVector NewCameraOffset = HeadsetCamera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.X -= 10;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);
}

#pragma region Input
void AVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("RightTriggerAxis", this, &AVRPawn::SetRightTriggerAxis);
	PlayerInputComponent->BindAxis("LeftTriggerAxis", this, &AVRPawn::SetLeftTriggerAxis);

	PlayerInputComponent->BindAction("Move", IE_Pressed, this, &AVRPawn::MovePressed);
	PlayerInputComponent->BindAction("Move", IE_Released, this, &AVRPawn::MoveReleased);

	PlayerInputComponent->BindAction("RightTeleport", IE_Pressed, this, &AVRPawn::RightTeleport);
	PlayerInputComponent->BindAction("LeftTeleport", IE_Pressed, this, &AVRPawn::LeftTeleport);

	PlayerInputComponent->BindAction("RightMenu", IE_Pressed, this, &AVRPawn::RightMenu);
	PlayerInputComponent->BindAction("LeftMenu", IE_Pressed, this, &AVRPawn::LeftMenu);

	PlayerInputComponent->BindAction("Click_R", IE_Pressed, this, &AVRPawn::RClickPressed);
	PlayerInputComponent->BindAction("Click_L", IE_Pressed, this, &AVRPawn::LClickPressed);
	PlayerInputComponent->BindAction("Click_R", IE_Released, this, &AVRPawn::RClickReleased);
	PlayerInputComponent->BindAction("Click_L", IE_Released, this, &AVRPawn::LClickReleased);


}

void AVRPawn::RightTeleport()
{
	if (bMenuActive()) return;
	FVector Displacement = Laser_R->GetUpVector() * TeleportDistance;
	AddActorWorldOffset(Displacement, true);
}

void AVRPawn::LeftTeleport()
{
	if (bMenuActive()) return;
	FVector Displacement = Laser_L->GetUpVector() * TeleportDistance;
	AddActorWorldOffset(Displacement, true);
}

void AVRPawn::MoveSmoothly(float DeltaTime)
{
	if (!bMovePressed || bMenuActive()) return;
	FVector Displacement = (RightTriggerAxis * Laser_R->GetUpVector() + LeftTriggerAxis * Laser_L->GetUpVector()) * MovementScalingFactor * DeltaTime;
	AddActorWorldOffset(Displacement, true);
}

void AVRPawn::ToggleMenu(const EControllerHand& Hand)
{
	if (ActiveMenuActor) {
		ActiveMenuActor->Destroy();
		ActiveMenuActor = nullptr;

		InteractionComponent_L->Deactivate();
		InteractionComponent_R->Deactivate();
		Laser_L->SetVisibility(true);
		Laser_R->SetVisibility(true);
	}
	else
	{
		if (!ensure(MenuActor)) return;
		FVector Location = GetActorLocation();
		FRotator Rotation = FRotator(0.f, HeadsetCamera->GetComponentRotation().Yaw, 0.f);
		FActorSpawnParameters SpawnParams;
		ActiveMenuActor = GetWorld()->SpawnActor(MenuActor);
		ActiveMenuActor->SetActorTransform(FTransform(Rotation, Location, FVector(1)));

		Laser_L->SetVisibility(false);
		Laser_R->SetVisibility(false);
		switch (Hand) {
		case EControllerHand::Left:
			InteractionComponent_L->Activate();
			break;
		case EControllerHand::Right:
			InteractionComponent_R->Activate();
			break;
		default:
			InteractionComponent_L->Activate();
			InteractionComponent_R->Activate();
		}

	}
}

#pragma region BoringInput
void AVRPawn::RClickPressed() { InteractionComponent_R->PressPointerKey(EKeys::LeftMouseButton); }
void AVRPawn::LClickPressed() { InteractionComponent_L->PressPointerKey(EKeys::LeftMouseButton); }
void AVRPawn::RClickReleased() { InteractionComponent_R->ReleasePointerKey(EKeys::LeftMouseButton); }
void AVRPawn::LClickReleased() { InteractionComponent_L->ReleasePointerKey(EKeys::LeftMouseButton); }
#pragma endregion BoringInput

#pragma endregion Input
