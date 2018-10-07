// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FPTemplateGameMode.h"
#include "FPTemplateHUD.h"
#include "FPTemplateCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPTemplateGameMode::AFPTemplateGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPTemplateHUD::StaticClass();
}
