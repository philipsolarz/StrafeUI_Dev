// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ModalWidget.cpp

#include "UI/S_UI_ModalWidget.h"
#include "CommonTextBlock.h"
#include "CommonButtonBase.h"

void US_UI_ModalWidget::SetupModal(const F_UIModalPayload& Payload)
{
	if (Text_Message)
	{
		Text_Message->SetText(Payload.Message);
	}

	// Configure buttons based on modal type
	if (Btn_Confirm && Btn_Decline)
	{
		switch (Payload.ModalType)
		{
		case E_UIModalType::ConfirmDecline:
			Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
			Btn_Decline->SetVisibility(ESlateVisibility::Visible);
			// Optionally set text: Btn_Confirm->SetButtonText(Payload.ConfirmText);
			break;
		case E_UIModalType::YesNo:
			Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
			Btn_Decline->SetVisibility(ESlateVisibility::Visible);
			// Optionally set text: Btn_Confirm->SetButtonText(FText::FromString("Yes"));
			break;
		case E_UIModalType::OK:
			Btn_Confirm->SetVisibility(ESlateVisibility::Visible);
			Btn_Decline->SetVisibility(ESlateVisibility::Collapsed);
			// Optionally set text: Btn_Confirm->SetButtonText(FText::FromString("OK"));
			break;
		}
	}
}

void US_UI_ModalWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Confirm)
	{
		Btn_Confirm->OnClicked().AddUObject(this, &US_UI_ModalWidget::HandleConfirmClicked);
	}

	if (Btn_Decline)
	{
		Btn_Decline->OnClicked().AddUObject(this, &US_UI_ModalWidget::HandleDeclineClicked);
	}
}

void US_UI_ModalWidget::HandleConfirmClicked()
{
	OnDismissed.Broadcast(true);
	RemoveFromParent();
}

void US_UI_ModalWidget::HandleDeclineClicked()
{
	OnDismissed.Broadcast(false);
	RemoveFromParent();
}