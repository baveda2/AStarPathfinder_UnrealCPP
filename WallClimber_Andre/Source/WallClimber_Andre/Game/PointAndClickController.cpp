

#include "PointAndClickController.h"

#include "GameFramework/Pawn.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "ClimberCharacter.h"
#include "Kismet/GameplayStatics.h"

#include "PathfindingComponent.h"


APointAndClickController::APointAndClickController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;


}

void APointAndClickController::BeginPlay()
{
	Super::BeginPlay();

    ControlledCharacter = Cast<AClimberCharacter>(GetPawn());
    
    // Add Enhanced Mapping Context to Input System
    if (ClimberInputMappingContext && ControlledCharacter)
    {
        if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
        {
            if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {

                InputSystem->AddMappingContext(ClimberInputMappingContext, 1);
                
            }
        }
    }

    // Get the PathfindingComponent from the character
    PathfindingComp = Cast<UPathfindingComponent>(ControlledCharacter->GetComponentByClass(UPathfindingComponent::StaticClass()));

}

void APointAndClickController::SetupInputComponent()
{
	Super::SetupInputComponent();


    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) 
    {
        EnhancedInputComponent->BindAction(RotateCameraAction, ETriggerEvent::Triggered, this, &APointAndClickController::HandleCameraRotation);
        EnhancedInputComponent->BindAction(RotateCameraAction, ETriggerEvent::Completed, this, &APointAndClickController::ResetRotation);
        EnhancedInputComponent->BindAction(ZoomCameraAction, ETriggerEvent::Started, this, &APointAndClickController::HandleCameraZoom);
        EnhancedInputComponent->BindAction(ClickMovementAction, ETriggerEvent::Completed, this, &APointAndClickController::ClickMove);
    }

}

void APointAndClickController::HandleCameraRotation(const FInputActionValue& Value)
{
    const FVector2D RotationVector = Value.Get<FVector2D>();


    if (ControlledCharacter)
    {
        if (RotationVector.X > 0) // Right
        {
            ControlledCharacter->RotateCameraToRight();
        }
        else if (RotationVector.X < 0) // Left
        {
            ControlledCharacter->RotateCameraToLeft();
        }

        if (RotationVector.Y > 0) // Front
        {
            ControlledCharacter->RotateCameraToFront();
        }
        else if (RotationVector.Y < 0) // Back
        {
            ControlledCharacter->RotateCameraToBack();
        }
    }

}

void APointAndClickController::ResetRotation(const FInputActionValue& Value)
{
    const FVector2D RotationVector = Value.Get<FVector2D>();

    if (ControlledCharacter)
    {
        if (RotationVector == FVector2D(0.f, 0.f)) // Only if no keys are pressed
        {
            ControlledCharacter->ResetSpringArmRotation();
        }
    }
}


void APointAndClickController::HandleCameraZoom(const FInputActionValue& Value)
{
    const float ZoomValue = Value.Get<float>();
    UE_LOG(LogTemp, Warning, TEXT("Zoom: %s"), *FString::SanitizeFloat(ZoomValue));
    ControlledCharacter->HandleZoomInput(ZoomValue);

}

void APointAndClickController::ClickMove(const FInputActionValue& Value)
{
    const bool bClick = Value.Get<bool>();
    UE_LOG(LogTemp, Warning, TEXT("__LeftClick__"));

    FlushPersistentDebugLines(GetWorld());

    // Perform a line trace to get the hit location on the mesh
    FHitResult HitResult;
    if (GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), false, HitResult))
    {
        if (HitResult.GetActor()->Tags.Contains("Walkable"))
        {
            // Kill last spawned Niagara system if still active.
            if (LastPointClickFXSpawned && LastPointClickFXSpawned->IsValidLowLevelFast())
            {
                LastPointClickFXSpawned->DestroyComponent();
            }

            // Spawn Niagara system
            FVector ParticleSpawnLocation = HitResult.Location + FVector(-5.f, 0.f, 0.f); // Slightly above the mesh
            LastPointClickFXSpawned = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PointClickFX, ParticleSpawnLocation);

            if (ControlledCharacter && PathfindingComp)
            {
                // Process pathfinding and movement
                ProcessPathfinding(HitResult.Location);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("ControlledCharacter or PathfindingComp is null"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("INVALID NAVIGATION TARGET"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("NO HIT RESULT"));
    }
}

void APointAndClickController::ProcessPathfinding(const FVector& TargetLocation)
{
    // Get player location less player offset to the node grid
    FVector PlayerLocation = ControlledCharacter->GetActorLocation() - FVector(-45.f, 0.f, 0.f);

    // Find closest node to that location
    FPathfindingNode* ClosestNode = PathfindingComp->GetClosestNode(PlayerLocation);
    if (!ClosestNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("NO CLOSEST NODE FOUND"));
        return;
    }

    // Highlight the closest node
    DrawDebugPoint(GetWorld(), ClosestNode->Location, 10.f, FColor::Emerald, true, -1.f);
    UE_LOG(LogTemp, Warning, TEXT("Closest Node Found at %s"), *ClosestNode->Location.ToString());

    // Find Closest Node to target location
    FPathfindingNode* TargetNode = PathfindingComp->GetClosestNode(TargetLocation);
    if (!TargetNode)
    {
        UE_LOG(LogTemp, Warning, TEXT("NO TARGET NODE FOUND"));
        return;
    }

    DrawDebugPoint(GetWorld(), TargetNode->Location, 10.f, FColor::Magenta, true, -1.f);
    UE_LOG(LogTemp, Warning, TEXT("Target Node Found at %s"), *TargetNode->Location.ToString());

    // Make Path
    TArray<FPathfindingNode> CurrentPath = PathfindingComp->CalculateAStarPath(ClosestNode, TargetNode);
    if (CurrentPath.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NO PATH FOUND"));
        return;
    }

    // Draw debug points for the path
    for (int32 i = 1; i < CurrentPath.Num(); i++)
    {
        DrawDebugPoint(GetWorld(), CurrentPath[i].Location, 10.f, FColor::Cyan, true, -1.f);
    }
    UE_LOG(LogTemp, Warning, TEXT("Path created. Total nodes: %d"), CurrentPath.Num());

    // Move the character along the path
    //ControlledCharacter->Move(CurrentPath);
}




