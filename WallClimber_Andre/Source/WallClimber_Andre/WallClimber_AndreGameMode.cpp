// Copyright Epic Games, Inc. All Rights Reserved.

#include "WallClimber_AndreGameMode.h"
#include "WallClimber_AndrePlayerController.h"
#include "WallClimber_AndreCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWallClimber_AndreGameMode::AWallClimber_AndreGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AWallClimber_AndrePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}