// Fill out your copyright notice in the Description page of Project Settings.


#include "S_UI_PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void AS_UI_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get the Enhanced Input Local Player Subsystem
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			// Add the UI Input Mapping Context
			if (UIInputMappingContext)
			{
				InputSubsystem->AddMappingContext(UIInputMappingContext, 0);
			}
		}
	}

	// Show the mouse cursor
	SetShowMouseCursor(true);
}
