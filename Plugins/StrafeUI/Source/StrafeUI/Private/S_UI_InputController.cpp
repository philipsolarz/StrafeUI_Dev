// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_InputController.cpp

#include "S_UI_InputController.h"
#include "S_UI_Settings.h"
#include "EnhancedInputComponent.h"
#include "S_UI_Subsystem.h"
#include "InputAction.h" // Include for UInputAction
#include "InputActionValue.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void US_UI_InputController::Initialize(US_UI_Subsystem* InSubsystem, UEnhancedInputComponent* InputComponent, const US_UI_Settings* Settings)
{
	check(InSubsystem);
	check(InputComponent);
	check(Settings);

	UISubsystem = InSubsystem;

	// Load the actions from the soft pointers before binding
	if (UInputAction* NavigateAction = Settings->NavigateAction.LoadSynchronous())
	{
		InputComponent->BindAction(NavigateAction, ETriggerEvent::Triggered, this, &US_UI_InputController::OnNavigate);
	}
	if (UInputAction* AcceptAction = Settings->AcceptAction.LoadSynchronous())
	{
		InputComponent->BindAction(AcceptAction, ETriggerEvent::Started, this, &US_UI_InputController::OnAccept);
	}
	if (UInputAction* BackAction = Settings->BackAction.LoadSynchronous())
	{
		InputComponent->BindAction(BackAction, ETriggerEvent::Started, this, &US_UI_InputController::OnBack);
	}
}

void US_UI_InputController::OnNavigate(const FInputActionValue& Value)
{
	const FVector2D NavDirection = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Verbose, TEXT("InputController: Navigate triggered with value: %s"), *NavDirection.ToString());

	if (UISubsystem && UISubsystem->GetRootWidget())
	{
		// Navigation should now be routed to the active widget in the *content* stack
		if (US_UI_BaseScreenWidget* ActiveWidget = Cast<US_UI_BaseScreenWidget>(UISubsystem->GetRootWidget()->GetContentStack()->GetActiveWidget()))
		{
			ActiveWidget->HandleNavigation(NavDirection);
		}
	}
}

void US_UI_InputController::OnAccept(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Verbose, TEXT("InputController: Accept triggered."));

	if (UISubsystem && UISubsystem->GetRootWidget())
	{
		if (US_UI_BaseScreenWidget* ActiveWidget = Cast<US_UI_BaseScreenWidget>(UISubsystem->GetRootWidget()->GetContentStack()->GetActiveWidget()))
		{
			ActiveWidget->HandleAccept();
		}
	}
}

void US_UI_InputController::OnBack(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Verbose, TEXT("InputController: Back triggered."));

	if (UISubsystem)
	{
		// Use the new function to pop from the content stack
		UISubsystem->PopContentScreen();
	}
}