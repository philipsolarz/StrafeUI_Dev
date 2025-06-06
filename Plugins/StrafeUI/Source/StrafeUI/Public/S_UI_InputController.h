// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_InputController.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "S_UI_InputController.generated.h"

// Forward declarations
class US_UI_Subsystem;
class UEnhancedInputComponent;
class UInputAction;
struct FInputActionValue;

/**
 * @class US_UI_InputController
 * @brief Manages UI-specific input, binding actions to handler functions.
 *
 * This object is owned by the S_UI_Subsystem and is responsible for all
 * UI-related input processing, such as navigation, accept, and back actions.
 */
UCLASS(Blueprintable)
class STRAFEUI_API US_UI_InputController : public UObject
{
    GENERATED_BODY()

public:
    /**
     * Initializes the input controller, binding actions to the provided input component.
     * @param InSubsystem The owner UI subsystem.
     * @param InputComponent The Enhanced Input component to bind actions to.
     */
    void Initialize(US_UI_Subsystem* InSubsystem, UEnhancedInputComponent* InputComponent);

private:
    /** Handler for the Navigate input action. */
    void OnNavigate(const FInputActionValue& Value);

    /** Handler for the Accept/Confirm input action. */
    void OnAccept(const FInputActionValue& Value);

    /** Handler for the Back/Cancel input action. */
    void OnBack(const FInputActionValue& Value);

    /** Input Action for UI Navigation (e.g., Gamepad D-pad, WASD). */
    UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> NavigateAction;

    /** Input Action for confirming a selection. */
    UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> AcceptAction;

    /** Input Action for going back or canceling. */
    UPROPERTY(EditDefaultsOnly, Category = "Input Actions")
    TObjectPtr<UInputAction> BackAction;

    /** Cached pointer to the UI Subsystem. */
    UPROPERTY()
    TObjectPtr<US_UI_Subsystem> UISubsystem;
};