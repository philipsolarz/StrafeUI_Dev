// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_InputController.cpp

#include "S_UI_InputController.h"
#include "EnhancedInputComponent.h"
#include "S_UI_Subsystem.h" // For future use, e.g., UISubsystem->GetActiveWidget()->OnNavigate();
#include "InputActionValue.h"

void US_UI_InputController::Initialize(US_UI_Subsystem* InSubsystem, UEnhancedInputComponent* InputComponent)
{
    check(InSubsystem);
    check(InputComponent);

    UISubsystem = InSubsystem;

    // A common pattern is to load these actions from a Data Asset, but for simplicity, we assume they are set on a Blueprint subclass.
    if (NavigateAction)
    {
        InputComponent->BindAction(NavigateAction, ETriggerEvent::Triggered, this, &US_UI_InputController::OnNavigate);
    }
    if (AcceptAction)
    {
        InputComponent->BindAction(AcceptAction, ETriggerEvent::Started, this, &US_UI_InputController::OnAccept);
    }
    if (BackAction)
    {
        InputComponent->BindAction(BackAction, ETriggerEvent::Started, this, &US_UI_InputController::OnBack);
    }
}

void US_UI_InputController::OnNavigate(const FInputActionValue& Value)
{
    // In a real implementation, you would forward this to the active widget or UI system.
    const FVector2D NavDirection = Value.Get<FVector2D>();
    UE_LOG(LogTemp, Verbose, TEXT("InputController: Navigate triggered with value: %s"), *NavDirection.ToString());
}

void US_UI_InputController::OnAccept(const FInputActionValue& Value)
{
    // In a real implementation, you would forward this to the active widget or UI system.
    UE_LOG(LogTemp, Verbose, TEXT("InputController: Accept triggered."));

    // Example: UISubsystem->GetTopmostScreen()->HandleConfirm();
}

void US_UI_InputController::OnBack(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Verbose, TEXT("InputController: Back triggered."));

    // The most common use for a global "Back" is to pop the current screen.
    UISubsystem->PopScreen();
}