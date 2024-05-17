// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "NiagaraSystem.h"

#include "PointAndClickController.generated.h"

// Forward declaration
class UInputAction;
class AClimberCharacter;
class UPathfindingComponent;

UCLASS()
class WALLCLIMBER_ANDRE_API APointAndClickController : public APlayerController
{
	GENERATED_BODY()

public:
	APointAndClickController();

	// Those variables must be set in child blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* ClimberInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ZoomCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClickMovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RotateCameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* PointClickFX;



protected:

	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;

	void ProcessPathfinding(const FVector& TargetLocation);

	void HandleCameraRotation(const FInputActionValue& Value);
	void HandleCameraZoom(const FInputActionValue& Value);
	void ResetRotation(const FInputActionValue& Value);
	void ClickMove(const FInputActionValue& Value);

private:


	UInputMappingContext* ClimberContext;
	UInputAction* RotationAction;
	AClimberCharacter* ControlledCharacter;
	UNiagaraComponent* LastPointClickFXSpawned;
	UPathfindingComponent* PathfindingComp;
};

