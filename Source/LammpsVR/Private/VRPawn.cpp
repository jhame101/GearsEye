// Fill out your copyright notice in the Description page of Project Settings.


#include "VRPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
#include "MotionControllerComponent.h" 	
#include "SRanipal_FunctionLibrary_Eye.h"

//Debug lines
#include "DrawDebugHelpers.h"

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

	RecordingCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Recording Camera"));
	RecordingCamera->SetupAttachment(HeadsetCamera);

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

	FVector EyeTrackOrigin, EyeTrackDirection;

	UE_LOG(LogTemp, Warning, TEXT("Tick called"));


	if (USRanipal_FunctionLibrary_Eye::GetGazeRay(GazeIndex::COMBINE, EyeTrackOrigin, EyeTrackDirection)) {
		// EyeTrackMesh->SetRelativeLocation(EyeTrackOrigin + EyeTrackRadius * EyeTrackDirection);
		FHitResult SightlineHit;
		FVector LineTraceStart = GetActorLocation() + EyeTrackOrigin;
		FVector LineTraceEnd = LineTraceStart + EyeTrackRadius * (HeadsetCamera->GetRelativeRotation() + EyeTrackDirection.Rotation()).Vector();
		DrawDebugLine(GetWorld(), LineTraceStart, LineTraceEnd, FColor(0, 240, 150), false, 0.f, 0.f, 10.f);
		if (GetWorld()->LineTraceSingleByChannel(SightlineHit, LineTraceStart, LineTraceEnd, ECC_Visibility/*, const FCollisionQueryParams & Params, const FCollisionResponseParams & ResponseParam*/)) {
			EyeTrackMesh->SetWorldLocation(SightlineHit.Location);
		}
	}

	FVector NewCameraOffset = HeadsetCamera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.X -= 10;
	AddActorWorldOffset(NewCameraOffset);
	VRRoot->AddWorldOffset(-NewCameraOffset);



}

// Called to bind functionality to input
void AVRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

