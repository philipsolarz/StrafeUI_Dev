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

	// Only set these for the primary player to avoid conflicts
	if (GetLocalPlayer() && GetLocalPlayer()->GetControllerId() == 0)
	{
		SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}

void AS_UI_PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up this player's UI
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (US_UI_Subsystem* UISubsystem = LocalPlayer->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
		{
			UISubsystem->CleanupPlayerUI(this);
		}
	}

	// Only reset input mode for primary player
	if (GetLocalPlayer() && GetLocalPlayer()->GetControllerId() == 0)
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		SetShowMouseCursor(false);
	}

	Super::EndPlay(EndPlayReason);
}