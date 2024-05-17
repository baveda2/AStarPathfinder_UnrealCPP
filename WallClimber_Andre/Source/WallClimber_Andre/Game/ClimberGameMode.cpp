// Fill out your copyright notice in the Description page of Project Settings.


#include "ClimberGameMode.h"

#include "PointAndClickController.h"
#include "ClimberCharacter.h"
#include "UObject/ConstructorHelpers.h"

AClimberGameMode::AClimberGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = APointAndClickController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Character/BP_ClimberCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FAIL TO ASSIGN PLAYER PAWN IN GAMEMODE"));
	}
	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Script/WallClimber_Andre.PointAndClickController"));
	if (PlayerControllerBPClass.Class != NULL)
	{
		
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FAIL TO ASSIGN PLAYER CONTROLLER IN GAMEMODE"));
	}
}
