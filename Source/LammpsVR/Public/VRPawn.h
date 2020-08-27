// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VRPawn.generated.h"

class UStaticMeshComponent;
class USceneCaptureComponent2D;
class USpotLightComponent;
class UCapsuleComponent;
class UCameraComponent;
class UMotionControllerComponent;

UCLASS()
class LAMMPSVR_API AVRPawn : public APawn
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AVRPawn();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* Capsule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* VRRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* HeadsetCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* HeadLamp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* ExternalCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* ViveController_L;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Laser_L;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* ViveController_R;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Laser_R;

protected:
	FVector2D GetGazeLocationOnScreen() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UMaterialInterface* PostProcessMaterial;

	// Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float TeleportDistance = 5000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MovementScalingFactor = 10000.f;

	UPROPERTY(EditDefaultsOnly)
	// TSubclassOf<AActor> MenuActor;
	UClass* MenuActor;

	// Input variables
	UPROPERTY(BlueprintReadOnly)
	float RightTriggerAxis = 0.f;
	UPROPERTY(BlueprintReadOnly)
	float LeftTriggerAxis = 0.f;
	UPROPERTY(BlueprintReadOnly)
	int bMovePressed = 0;		//Int rather than bool in case both are pressed at once. It should be treated as a bool (but keep in mind that it could be up to 2)

	bool bMenuActive() { return (bool)ActiveMenuActor; }
	AActor* ActiveMenuActor;

private:
	// Input functions
	void RightTeleport();
	void LeftTeleport();

	void ToggleMenu();

	//Boring input functions
	void SetRightTriggerAxis(float Value) { RightTriggerAxis = Value; }
	void SetLeftTriggerAxis(float Value) { LeftTriggerAxis = Value; }

	void MovePressed() { ++bMovePressed; }
	void MoveReleased() { --bMovePressed; }

	//Internal variables
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicPPM;
	FVector2D ExternalCameraFOV;

	//Internal functions
	bool GetCombinedGazeRay(FVector& EyeTrackOrigin, FVector& EyeTrackDirection) const;
	void UpdateActorLocation();
	void UpdateEyeTrackLocation();
	void MoveSmoothly(float DeltaTime);
};
