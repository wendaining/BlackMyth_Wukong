// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlackMythGameMode.h"
#include "WukongCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABlackMythGameMode::ABlackMythGameMode()
{
	// Set default pawn class to WukongCharacter C++ class
	DefaultPawnClass = AWukongCharacter::StaticClass();
}
