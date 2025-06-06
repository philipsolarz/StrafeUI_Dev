// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_ModalStack.cpp

#include "S_UI_ModalStack.h"
#include "S_UI_Subsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/S_UI_ModalWidget.h"
#include "Data/S_UI_ScreenTypes.h"


void US_UI_ModalStack::Initialize(US_UI_Subsystem* InSubsystem, TSoftClassPtr<US_UI_ModalWidget> InModalWidgetClass)
{
	check(InSubsystem);
	UISubsystem = InSubsystem;
	ModalWidgetClass = InModalWidgetClass;
}

void US_UI_ModalStack::QueueModal(const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback)
{
	ModalRequestQueue.Add({ Payload, OnDismissedCallback });
	TryDisplayNextModal();
}

void US_UI_ModalStack::TryDisplayNextModal()
{
	if (ActiveModal || ModalRequestQueue.Num() == 0)
	{
		return;
	}

	if (!ModalWidgetClass.IsValid() && !ModalWidgetClass.IsPending())
	{
		ModalWidgetClass.LoadSynchronous();
	}

	if (!ModalWidgetClass.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ModalStack: ModalWidgetClass is not set in Project Settings -> Strafe UI or failed to load!"));
		if (ModalRequestQueue.Num() > 0)
		{
			ModalRequestQueue.RemoveAt(0);
		}
		return;
	}


	const F_UIModalRequest& NextRequest = ModalRequestQueue[0];

	APlayerController* PC = UISubsystem->GetGameInstance()->GetFirstLocalPlayerController();
	if (!PC)
	{
		return;
	}

	US_UI_ModalWidget* NewModal = CreateWidget<US_UI_ModalWidget>(PC, ModalWidgetClass.Get());
	if (NewModal)
	{
		ActiveModal = NewModal;
		ActiveModalDismissedCallback = NextRequest.OnDismissedCallback;

		NewModal->SetupModal(NextRequest.Payload);
		NewModal->OnDismissed.AddDynamic(this, &US_UI_ModalStack::OnModalDismissed);
		NewModal->AddToViewport(100); // High Z-order to ensure it's on top
	}

	ModalRequestQueue.RemoveAt(0);
}

void US_UI_ModalStack::OnModalDismissed(bool bConfirmed)
{
	if (ActiveModalDismissedCallback.IsBound())
	{
		ActiveModalDismissedCallback.Execute(bConfirmed);
	}

	if (ActiveModal)
	{
		// The widget handles its own RemoveFromParent()
		ActiveModal = nullptr;
	}

	ActiveModalDismissedCallback.Unbind();

	TryDisplayNextModal();
}