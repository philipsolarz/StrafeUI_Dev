// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_ModalStack.cpp

#include "S_UI_ModalStack.h"
#include "S_UI_Subsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UserInterface/Public/CommonUserWidget.h" // Placeholder for your actual modal widget base class
// #include "S_UI_ModalWidget.h" // You will need to create and include this header.

// A dummy implementation for S_UI_ModalWidget for compilation purposes.
// Replace with your actual modal widget class.
class US_UI_ModalWidget : public UCommonUserWidget
{
public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnModalDismissed, bool, bConfirmed);
    FOnModalDismissed OnDismissed;
    void SetupModal(const F_UIModalPayload& Payload) {}
};


void US_UI_ModalStack::Initialize(US_UI_Subsystem* InSubsystem)
{
    check(InSubsystem);
    UISubsystem = InSubsystem;
}

void US_UI_ModalStack::QueueModal(const F_UIModalPayload& Payload, const FOnUIModalDismissed& OnDismissedCallback)
{
    ModalRequestQueue.Add({ Payload, OnDismissedCallback });
    TryDisplayNextModal();
}

void US_UI_ModalStack::TryDisplayNextModal()
{
    if (ActiveModal || ModalRequestQueue.Num() == 0)
    {
        // A modal is already active, or the queue is empty.
        return;
    }

    // You would typically use the Asset Manager to load this asynchronously.
    if (!ModalWidgetClass.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("ModalStack: ModalWidgetClass is not set!"));
        return;
    }

    const F_UIModalRequest& NextRequest = ModalRequestQueue[0];

    APlayerController* PC = UISubsystem->GetGameInstance()->GetFirstLocalPlayerController();
    if (!PC)
    {
        return;
    }

    US_UI_ModalWidget* NewModal = CreateWidget<US_UI_ModalWidget>(PC, ModalWidgetClass.LoadSynchronous());
    if (NewModal)
    {
        ActiveModal = NewModal;
        ActiveModalDismissedCallback = NextRequest.OnDismissedCallback;

        ActiveModal->SetupModal(NextRequest.Payload); // Configure the widget
        ActiveModal->OnDismissed.AddDynamic(this, &US_UI_ModalStack::OnModalDismissed);
        ActiveModal->AddToViewport(100); // High Z-order to ensure it's on top
    }

    ModalRequestQueue.RemoveAt(0);
}

void US_UI_ModalStack::OnModalDismissed(bool bConfirmed)
{
    if (ActiveModalDismissedCallback.IsBound())
    {
        ActiveModalDismissedCallback.Broadcast(bConfirmed);
    }

    if (ActiveModal)
    {
        ActiveModal->RemoveFromParent();
        ActiveModal = nullptr;
    }

    ActiveModalDismissedCallback.Clear();

    // Attempt to show the next modal in the queue.
    TryDisplayNextModal();
}