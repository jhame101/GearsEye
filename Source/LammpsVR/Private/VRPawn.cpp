// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
#include "MotionControllerComponent.h" 	
#include "SRanipal_FunctionLibrary_Eye.h"

// Sets default values
AVRPawn::AVRPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Default Scene Root"));
	RootComponent = DefaultSceneRoot;

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

	// TODO: Remove
	EyeTrackMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Eye Tracking Mesh"));
	EyeTrackMesh->SetupAttachment(HeadsetCamera);
	EyeTrackMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

// Called when the game starts or when spawned
void AVRPawn::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AVRPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Keeping the actor location the same as the player location. Maybe turn into a seperate function later?
	FVector NewCameraOffset = HeadsetCamera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.X -= 10;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);

	//TEMP section to set the location of a SM directly where the player is looking
	FVector EyeTrackOrigin, EyeTrackDirection;
	if (USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::COMBINE, EyeTrackOrigin, EyeTrackDirection)) {
		EyeTrackMesh->SetRelativeLocation(EyeTrackOrigin + EyeTrackRadius * EyeTrackDirection);
	}

	//TODO: call function to update location of eye track overlay

}

// Called to bind functionality to input
void AVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

FVector2D AVRPawn::GetGazeLocationOnScreen() const
{
	FVector EyeTrackOrigin, EyeTrackDirection;

	if (!USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::COMBINE, EyeTrackOrigin, EyeTrackDirection)) return FVector2D();
	if (!ensure(ExternalCamera)) return FVector2D();

	// auto PlayerController = Cast<APlayerController>(GetController()); if (!PlayerController) return FVector2D();

	FVector RelativePosition = EyeTrackOrigin + 1000*EyeTrackDirection;		//Assuming that ExternalCamera has the same transform as HeadsetCamera
	FRotator RelativeAngle = RelativePosition.Rotation();

	// Rotation in Z corresponds to x on screen, Y rotation corresponds to Y
	FVector2D ScreenLocation = (RelativeAngle.Yaw, RelativeAngle.Pitch);

	float FOVx = ExternalCamera->FOVAngle;
	float FOVy = FOVx * 9 / 16;	//Assuming 16:9 aspect ratio (which it is by default)

	//Convert to UF coords
	// ScreenLocation.X /= FOVx;
	ScreenLocation.X = 0.5;		//TODO: set properly
	ScreenLocation.Y = 0.5 - (ScreenLocation.Y / (FOVy * 1.1875));		//Don't ask me why that extra correction of 1.1875 is needed, it just is. Maybe I have the aspect ratio wrong?

	// PlayerController->ProjectWorldLocationToScreen(WorldPosition, ScreenLocation);

	/* //Convert to UV coords
	int32 VPSizeX, VPSizeY;
	PlayerController->GetViewportSize(VPSizeX, VPSizeY);
	ScreenLocation.X /= VPSizeX;
	ScreenLocation.Y /= VPSizeY; */

	UE_LOG(LogTemp, Warning, TEXT("%s"), *ScreenLocation.ToString());
	// UE_LOG(LogTemp, Warning, TEXT("%d. %d"), VPSizeX, VPSizeY);

	return ScreenLocation;
}