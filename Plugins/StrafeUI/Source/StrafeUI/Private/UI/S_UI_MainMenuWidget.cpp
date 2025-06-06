// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_MainMenuWidget.cpp

#include "UI/S_UI_MainMenuWidget.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"
#include "Data/S_UI_ScreenTypes.h"
#include "Kismet/KismetSystemLibrary.h"

void US_UI_MainMenuWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind button clicks to their respective handler functions.
    if (Btn_Create)
    {
        Btn_Create->OnClicked().AddUObject(this, &US_UI_MainMenuWidget::HandleCreateGameClicked);
    }
    if (Btn_Find)
    {
        Btn_Find->OnClicked().AddUObject(this, &US_UI_MainMenuWidget::HandleFindGameClicked);
    }
    if (Btn_Settings)
    {
        Btn_Settings->OnClicked().AddUObject(this, &US_UI_MainMenuWidget::HandleSettingsClicked);
    }
    if (Btn_Quit)
    {
        Btn_Quit->OnClicked().AddUObject(this, &US_UI_MainMenuWidget::HandleQuitClicked);
    }
}

void US_UI_MainMenuWidget::HandleCreateGameClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->PushScreen(E_UIScreenId::CreateGame);
    }
}

void US_UI_MainMenuWidget::HandleFindGameClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->PushScreen(E_UIScreenId::FindGame);
    }
}

void US_UI_MainMenuWidget::HandleSettingsClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->PushScreen(E_UIScreenId::Settings);
    }
}

void US_UI_MainMenuWidget::HandleQuitClicked()
{
    // Example of a modal dialog request.
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        F_UIModalPayload Payload;
        Payload.Message = FText::FromString(TEXT("Are you sure you want to quit?"));
        Payload.ModalType = E_UIModalType::YesNo;

        // The lambda function will be executed when the modal is dismissed.
        // Use the new FOnModalDismissedSignature and its CreateLambda method.
        UISubsystem->RequestModal(Payload, FOnModalDismissedSignature::CreateLambda([this](bool bConfirmed)
            {
                if (bConfirmed)
                {
                    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, true);
                }
            }));
    }
}