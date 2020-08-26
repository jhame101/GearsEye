// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"
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

	Laser_L = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Left Laser"));
	Laser_L->SetupAttachment(ViveController_L);
	Laser_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ViveController_R = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Right Vive Controller"));
	ViveController_R->SetupAttachment(VRRoot);

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
}

// Called to bind functionality to input
void AVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

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

void AVRPawn::UpdateActorLocation()
{
	FVector NewCameraOffset = HeadsetCamera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.X -= 10;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);
}

void AVRPawn::UpdateEyeTrackLocation()
{
	FVector2D TwoDCoordinates = GetGazeLocationOnScreen();
	DynamicPPM->SetVectorParameterValue(TEXT("Centre Coordinates"), FLinearColor(TwoDCoordinates.X, TwoDCoordinates.Y, 0.f, 0.f));

	// DynamicPPM->SetScalarParameterValue(TEXT("Radius"), 0.03f);		// In case uncertainty is added later to control the radius
}