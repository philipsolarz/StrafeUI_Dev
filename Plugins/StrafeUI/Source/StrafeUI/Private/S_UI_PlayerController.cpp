// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_PlayerController.cpp

#include "S_UI_PlayerController.h"
#include "S_UI_Subsystem.h" // Include the subsystem header
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"

void AS_UI_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get the Enhanced Input Local Player Subsystem
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		// Tell the UI Subsystem to create the UI for this player.
		if (US_UI_Subsystem* UISubsystem = LocalPlayer->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
		{
			UISubsystem->InitializeUIForPlayer(this);
		}

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

	// Set the input mode to UI Only so the mouse can interact with widgets
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}