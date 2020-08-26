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

	UPROPERTY(EditAnywhere)
	UMaterialInterface* PostProcessMaterial;

	// Private member variables
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicPPM;
	FVector2D ExternalCameraFOV;


	void UpdateActorLocation();

	bool GetCombinedGazeRay(FVector& EyeTrackOrigin, FVector& EyeTrackDirection) const;

	void UpdateEyeTrackLocation();

protected:
	FVector2D GetGazeLocationOnScreen() const;

};
