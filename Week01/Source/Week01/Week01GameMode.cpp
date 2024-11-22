// Copyright Epic Games, Inc. All Rights Reserved.

#include "Week01GameMode.h"
#include "Week01Character.h"
#include "UObject/ConstructorHelpers.h"

AWeek01GameMode::AWeek01GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
