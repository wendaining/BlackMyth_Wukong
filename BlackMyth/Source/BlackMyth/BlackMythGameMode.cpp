// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlackMythGameMode.h"
#include "BlackMythCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABlackMythGameMode::ABlackMythGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
