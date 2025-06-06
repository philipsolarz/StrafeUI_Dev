// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_InputController.cpp

#include "S_UI_InputController.h"
#include "EnhancedInputComponent.h"
#include "S_UI_Subsystem.h"
#include "InputActionValue.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
// #include "Data/DA_UIInputActions.h" // Assumed Data Asset header

void US_UI_InputController::Initialize(US_UI_Subsystem* InSubsystem, UEnhancedInputComponent* InputComponent)
{
    check(InSubsystem);
    check(InputComponent);

    UISubsystem = InSubsystem;

    // In a real project, the Subsystem would hold a TSoftObjectPtr to this Data Asset, load it,
    // and pass it into this Initialize function. For this example, we still rely on the properties
    // being set on the Blueprint subclass of this controller.
    //
    // Example of loading from a Data Asset:
    // if (const UDA_UIInputActions* InputActionsAsset = LoadInputActionsAsset())
    // {
    //     NavigateAction = InputActionsAsset->NavigateAction;
    //     AcceptAction = InputActionsAsset->AcceptAction;
    //     BackAction = InputActionsAsset->BackAction;
    // }

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
    const FVector2D NavDirection = Value.Get<FVector2D>();
    UE_LOG(LogTemp, Verbose, TEXT("InputController: Navigate triggered with value: %s"), *NavDirection.ToString());

    if (UISubsystem && UISubsystem->GetRootWidget())
    {
        if (US_UI_BaseScreenWidget* ActiveWidget = Cast<US_UI_BaseScreenWidget>(UISubsystem->GetRootWidget()->GetMainStack()->GetActiveWidget()))
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
        if (US_UI_BaseScreenWidget* ActiveWidget = Cast<US_UI_BaseScreenWidget>(UISubsystem->GetRootWidget()->GetMainStack()->GetActiveWidget()))
        {
            ActiveWidget->HandleAccept();
        }
    }
}

void US_UI_InputController::OnBack(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Verbose, TEXT("InputController: Back triggered."));

    // The most common use for a global "Back" is to pop the current screen.
    if (UISubsystem)
    {
        UISubsystem->PopScreen();
    }
}