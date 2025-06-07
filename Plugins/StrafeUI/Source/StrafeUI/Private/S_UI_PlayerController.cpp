// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_PlayerController.cpp

#include "S_UI_PlayerController.h"
#include "S_UI_Subsystem.h" 
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"

void AS_UI_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Set up the UI and input for the main menu.
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (US_UI_Subsystem* UISubsystem = LocalPlayer->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
		{
			UISubsystem->InitializeUIForPlayer(this);
		}

		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UIInputMappingContext)
			{
				InputSubsystem->AddMappingContext(UIInputMappingContext, 0);
			}
		}
	}

	SetShowMouseCursor(true);
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AS_UI_PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// When this PlayerController is destroyed (e.g., when traveling to a new level),
	// we must clean up the input mode to ensure the next PlayerController has control.
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);

	Super::EndPlay(EndPlayReason);
}