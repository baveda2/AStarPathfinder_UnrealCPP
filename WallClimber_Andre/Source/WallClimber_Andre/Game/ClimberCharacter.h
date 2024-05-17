// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "PointAndClickController.h"
#include "PathfindingComponent.h"

#include "ClimberCharacter.generated.h"



UCLASS()
class WALLCLIMBER_ANDRE_API AClimberCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AClimberCharacter();

	// SpringArm component for handling camera motion
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* CameraBoom;

	// Camera component for the player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FollowCamera;

	// Camera controls
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	float ZoomSpeed = -100.f;
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	float ZoomInterpSpeed = 50.f;
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	float MinZoom = 60.f;
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	float MaxZoom = 2000.f;
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	float RotationSpeed = 1.f;
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	float RotationInterpSpeed = 1.f;

	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	FRotator CharacterStartRotation = FRotator(90.f, 180.f, 180.f);

	// Camera Rotation 
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	FRotator FrontRotation = FRotator(0.f, 180.f, 0.f);
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	FRotator BackRotation = FRotator(0.f, 0.f, 0.f);
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	FRotator RightRotation = FRotator(0.f, -90.f, 0.f);
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	FRotator LeftRotation = FRotator(0.f, 90.f, 0.f);
	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	FRotator DefaultRotation = FRotator(-70.f, 0.f, 0.f);

	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	float RotationTolerance = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	FVector CharacterPositionOffset = FVector(-90.f, 0.f, 0.f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
	UPathfindingComponent* PathfindingComp;

	UPROPERTY(EditAnywhere, Category = "Climber Functionalities")
	float CharacterSpeed = 150.f;

	// Handle Camera Rotation
	void UpdateCameraRotation(float DeltaTime);

	// Method to smoothly rotate the SpringArm
	void SmoothRotateSpringArm(FRotator NewRotation, float RotationTime);

	// Method to reset the SpringArm to its original position
	void ResetSpringArmRotation();

	void RotateCameraToFront();
	void RotateCameraToBack();
	void RotateCameraToRight();
	void RotateCameraToLeft();


	// Method to Zoom (Control springarm lenght)
	void HandleZoomInput(float AxisValue);

	// Method to follow path provided by the pathfinding component
	void Move(TArray<FPathfindingNode> TrackNodes);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	void UpdateMovement();
	void ResetMovementState();

	APointAndClickController* PointAndClickController;
	FRotator OriginalBoomRotation; // Stores the original relative rotation of the SpringArm
	FRotator DesiredBoomRotation;
	bool bIsCameraRotating;

	// Track Movement
	TArray<FPathfindingNode> CurrentTrackNodes;
	int32 CurrentNodeIndex;
	FTimerHandle MovementTimerHandle;
	bool bIsMovingAlongPath;


};