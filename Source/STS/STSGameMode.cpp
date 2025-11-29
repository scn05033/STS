// Copyright Epic Games, Inc. All Rights Reserved.

#include "STSGameMode.h"
#include "STSCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASTSGameMode::ASTSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
