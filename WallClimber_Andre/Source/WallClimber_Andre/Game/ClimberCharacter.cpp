// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimberCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

// Sets default values
AClimberCharacter::AClimberCharacter()
{
	// Set this character to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// Create pathfinding component
	PathfindingComp = CreateDefaultSubobject<UPathfindingComponent>(TEXT("PathfindingComponent"));
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 500.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	CameraBoom->SetRelativeRotation(FRotator(-70.f, 0.f, 0.f));

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Configure character movement
	GetCharacterMovement()->GravityScale = 0.0f; // prevent falling
	GetCharacterMovement()->SetPlaneConstraintEnabled(true); // Enable movement on a plane
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(1.0f, 1.0f, 1.0f)); // Set the plane to be vertical (XY plane)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bIsMovingAlongPath = false;
	
}



// Called when the game starts or when spawned
void AClimberCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Store the original rotation of the SpringArm
	OriginalBoomRotation = CameraBoom->GetRelativeRotation();

	// Update rotation to start rotation
	SetActorRotation(CharacterStartRotation);


}

// Called every frame
void AClimberCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCameraRotation(DeltaTime);

	// Check if the character should be moving along the path
	if (bIsMovingAlongPath)
	{
		UpdateMovement();
	}
}

// Called to bind functionality to input
void AClimberCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AClimberCharacter::UpdateMovement()
{
	// Check if the character is currently set to move along a path
	if (!bIsMovingAlongPath)
	{
		return; // Exit the function if the character is not supposed to be moving
	}

	// Check if there are any nodes left in the path
	if (CurrentNodeIndex >= CurrentTrackNodes.Num())
	{
		// If there are no more nodes to move to, stop the movement
		bIsMovingAlongPath = false;
		UE_LOG(LogTemp, Warning, TEXT("End of track reached or no valid nodes left. Stopping movement."));
		return; // Exit the function as there are no nodes to move to
	}

	// Proceed with movement towards the next node
	FPathfindingNode NextNode = CurrentTrackNodes[CurrentNodeIndex];
	FVector TargetLocation = NextNode.Location + CharacterPositionOffset; // Apply the offset to the node's location
	FVector Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
	float Distance = FVector::Dist(GetActorLocation(), TargetLocation);
	float StepSize = CharacterSpeed * GetWorld()->GetDeltaSeconds();

	// If the character is not close enough to the target node, move towards it
	if (StepSize < Distance)
	{
		SetActorLocation(GetActorLocation() + Direction * StepSize);
	}
	else
	{
		// If the character is close enough to the target node, snap to it and increment the node index
		SetActorLocation(TargetLocation);
		CurrentNodeIndex++; // Move to the next node

		// Check if we have reached the end of the path
		if (CurrentNodeIndex >= CurrentTrackNodes.Num())
		{
			bIsMovingAlongPath = false; // Stop moving if we've reached the last node
			UE_LOG(LogTemp, Warning, TEXT("Reached the final node. Stopping movement."));
		}
	}
}



void AClimberCharacter::SmoothRotateSpringArm(FRotator NewRotation, float RotationTime)
{
	DesiredBoomRotation = NewRotation;
	bIsCameraRotating = true;
}

void AClimberCharacter::ResetSpringArmRotation()
{
	SmoothRotateSpringArm(DefaultRotation, RotationSpeed);
}

void AClimberCharacter::RotateCameraToFront()
{
	SmoothRotateSpringArm(FrontRotation, RotationSpeed);
}

void AClimberCharacter::RotateCameraToBack()
{
	SmoothRotateSpringArm(BackRotation, RotationSpeed);
}

void AClimberCharacter::RotateCameraToRight()
{
	SmoothRotateSpringArm(RightRotation, RotationSpeed);
}

void AClimberCharacter::RotateCameraToLeft()
{
	SmoothRotateSpringArm(LeftRotation, RotationSpeed);
}

void AClimberCharacter::HandleZoomInput(float AxisValue)
{
	// Adjust the SpringArm length based on the AxisValue
	float NewTargetArmLength = CameraBoom->TargetArmLength + (AxisValue * ZoomSpeed);

	// Clamp the value to prevent excessive zooming in or out
	NewTargetArmLength = FMath::Clamp(NewTargetArmLength, MinZoom, MaxZoom);

	// Smoothly interpolate to the new length
	CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, NewTargetArmLength, GetWorld()->GetDeltaSeconds(), ZoomInterpSpeed);
}

void AClimberCharacter::ResetMovementState()
{
	// Ensure the character stops moving
	bIsMovingAlongPath = false;

	// Clear the current path nodes
	CurrentTrackNodes.Empty();

	// Reset the current node index
	CurrentNodeIndex = 0;
}

void AClimberCharacter::Move(TArray<FPathfindingNode> TrackNodes)
{
	// If the character is already moving, do not allow a new movement to start
	if (bIsMovingAlongPath)
	{
		UE_LOG(LogTemp, Warning, TEXT("Movement already in progress. Wait until the current path is complete."));
		return;
	}

	// Log the move call
	UE_LOG(LogTemp, Warning, TEXT("Move function called with %d nodes."), TrackNodes.Num());

	// Set the new track nodes
	CurrentTrackNodes = TrackNodes;

	// Check if there are nodes to move to
	if (CurrentTrackNodes.Num() > 0)
	{
		// Reset the current node index and start moving along the path
		CurrentNodeIndex = 0;
		bIsMovingAlongPath = true;
		UE_LOG(LogTemp, Warning, TEXT("Character will start moving along the path."));
	}
	else
	{
		// If there are no nodes, log and do not start the movement
		UE_LOG(LogTemp, Warning, TEXT("No nodes to move to."));
	}
}



void AClimberCharacter::UpdateCameraRotation(float DeltaTime)
{
	if (bIsCameraRotating)
	{
		// Interpolate to the desired rotation
		FRotator NewRotation = FMath::RInterpTo(CameraBoom->GetRelativeRotation(), DesiredBoomRotation, DeltaTime, RotationInterpSpeed);

		// Apply the new rotation
		CameraBoom->SetRelativeRotation(NewRotation);

		// Determine if the rotation is close enough to the desired rotation
		if (NewRotation.Equals(DesiredBoomRotation, RotationTolerance))
		{
			bIsCameraRotating = false;
		}
	}
}