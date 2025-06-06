// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_ModalStack.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_ModalStack.generated.h"

// Forward declarations
class US_UI_Subsystem;
class US_UI_ModalWidget; // Assuming a base modal widget class

/**
 * @class US_UI_ModalStack
 * @brief Manages a queue of modal dialogs, ensuring they are displayed one by one.
 *
 * This system queues requests for modals and presents them sequentially. It handles
 * the creation, display, and dismissal of modal dialog widgets.
 */
UCLASS()
class STRAFEUI_API US_UI_ModalStack : public UObject
{
    GENERATED_BODY()

private:
    /**
     * @struct F_UIModalRequest
     * @brief A struct holding all information for a queued modal dialog request.
     */
    struct F_UIModalRequest
    {
        F_UIModalPayload Payload;
        FOnUIModalDismissed OnDismissedCallback;
    };

public:
    /**
     * Initializes the modal stack.
     * @param InSubsystem The owner UI subsystem.
     */
    void Initialize(US_UI_Subsystem* InSubsystem);

    /**
     * Queues a new modal dialog request.
     * @param Payload The data for the modal (message, buttons, etc.).
     * @param OnDismissedCallback The callback to execute when the modal is closed.
     */
    void QueueModal(const F_UIModalPayload& Payload, const FOnUIModalDismissed& OnDismissedCallback);

private:
    /** Attempts to display the next modal from the queue if one is not already active. */
    void TryDisplayNextModal();

    /** * Handles the dismissal of the active modal.
     * @param bConfirmed True if the user confirmed the modal, false otherwise.
     */
    UFUNCTION()
    void OnModalDismissed(bool bConfirmed);

    /** The queue of modal dialogs waiting to be displayed. */
    UPROPERTY()
    TArray<F_UIModalRequest> ModalRequestQueue;

    /** Pointer to the currently displayed modal widget, if any. */
    UPROPERTY()
    TObjectPtr<US_UI_ModalWidget> ActiveModal;

    /** The last callback associated with the active modal. */
    FOnUIModalDismissed ActiveModalDismissedCallback;

    /** Cached pointer to the UI Subsystem. */
    UPROPERTY()
    TObjectPtr<US_UI_Subsystem> UISubsystem;

    /** The widget class to use for creating modals. Must be set in a Blueprint subclass. */
    UPROPERTY(EditDefaultsOnly, Category = "Modal Stack")
    TSoftClassPtr<US_UI_ModalWidget> ModalWidgetClass;
};