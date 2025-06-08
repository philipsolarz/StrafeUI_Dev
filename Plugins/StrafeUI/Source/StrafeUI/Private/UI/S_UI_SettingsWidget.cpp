// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_SettingsWidget.cpp

#include "UI/S_UI_SettingsWidget.h"
#include "UI/S_UI_TabControl.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"
#include "S_UI_Settings.h"

void US_UI_SettingsWidget::SetViewModel(US_UI_VM_Settings* InViewModel)
{
    if (InViewModel)
    {
        ViewModel = InViewModel;

        // Defer tab initialization to ensure the widget is fully constructed
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
                {
                    InitializeSettingsTabs();
                });
        }
    }
}

void US_UI_SettingsWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind UI interaction events to handler functions
    if (Btn_Apply)
    {
        Btn_Apply->OnClicked().AddUObject(this, &US_UI_SettingsWidget::HandleApplyClicked);
    }
    if (Btn_Revert)
    {
        Btn_Revert->OnClicked().AddUObject(this, &US_UI_SettingsWidget::HandleRevertClicked);
    }
    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_UI_SettingsWidget::HandleBackClicked);
    }
    if (TabControl)
    {
        TabControl->OnTabSelected.AddDynamic(this, &US_UI_SettingsWidget::OnSettingsTabSelected);
    }
}

void US_UI_SettingsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // If we already have a view model but tabs haven't been initialized, do it now
    if (ViewModel.IsValid() && TabControl && TabControl->GetContentSwitcher()->GetNumWidgets() == 0)
    {
        InitializeSettingsTabs();
    }
}

void US_UI_SettingsWidget::NativeDestruct()
{
    if (TabControl)
    {
        TabControl->OnTabSelected.RemoveDynamic(this, &US_UI_SettingsWidget::OnSettingsTabSelected);
    }

    Super::NativeDestruct();
}

void US_UI_SettingsWidget::InitializeSettingsTabs()
{
    if (!TabControl || !ViewModel.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("SettingsWidget: Cannot initialize tabs - TabControl or ViewModel is null"));
        return;
    }

    // Get settings configuration
    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsWidget: No settings found"));
        return;
    }

    // Define the tabs
    TArray<FTabDefinition> TabDefs;

    // Only add tabs that have valid widget classes
    if (Settings->AudioSettingsTabClass.IsValid())
    {
        FTabDefinition AudioTab;
        AudioTab.TabName = FText::FromString(TEXT("Audio"));
        AudioTab.ContentWidgetClass = Settings->AudioSettingsTabClass;
        AudioTab.TabTag = "Audio";
        TabDefs.Add(AudioTab);
    }

    if (Settings->VideoSettingsTabClass.IsValid())
    {
        FTabDefinition VideoTab;
        VideoTab.TabName = FText::FromString(TEXT("Video"));
        VideoTab.ContentWidgetClass = Settings->VideoSettingsTabClass;
        VideoTab.TabTag = "Video";
        TabDefs.Add(VideoTab);
    }

    if (Settings->ControlsSettingsTabClass.IsValid())
    {
        FTabDefinition ControlsTab;
        ControlsTab.TabName = FText::FromString(TEXT("Controls"));
        ControlsTab.ContentWidgetClass = Settings->ControlsSettingsTabClass;
        ControlsTab.TabTag = "Controls";
        TabDefs.Add(ControlsTab);
    }

    if (Settings->GameplaySettingsTabClass.IsValid())
    {
        FTabDefinition GameplayTab;
        GameplayTab.TabName = FText::FromString(TEXT("Gameplay"));
        GameplayTab.ContentWidgetClass = Settings->GameplaySettingsTabClass;
        GameplayTab.TabTag = "Gameplay";
        TabDefs.Add(GameplayTab);
    }

    if (TabDefs.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SettingsWidget: No valid tab definitions found"));
        return;
    }

    // Initialize the tab control
    TabControl->InitializeTabs(TabDefs, 0);

    // Pass the view model to all tab content widgets
    SettingsTabs.Empty();
    if (UCommonActivatableWidgetSwitcher* Switcher = TabControl->GetContentSwitcher())
    {
        for (int32 i = 0; i < Switcher->GetNumWidgets(); ++i)
        {
            if (US_UI_SettingsTabBase* TabContent = Cast<US_UI_SettingsTabBase>(Switcher->GetWidgetAtIndex(i)))
            {
                TabContent->SetViewModel(ViewModel.Get());
                SettingsTabs.Add(TabContent);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("SettingsWidget: Initialized %d tabs"), SettingsTabs.Num());
}

void US_UI_SettingsWidget::OnSettingsTabSelected(int32 TabIndex, FName TabTag)
{
    UE_LOG(LogTemp, Verbose, TEXT("Settings tab selected: %d (%s)"), TabIndex, *TabTag.ToString());
}

void US_UI_SettingsWidget::HandleApplyClicked()
{
    // Apply settings in all tabs
    for (US_UI_SettingsTabBase* Tab : SettingsTabs)
    {
        if (Tab)
        {
            Tab->ApplySettings();
        }
    }

    // Apply through view model
    if (ViewModel.IsValid())
    {
        ViewModel->ApplySettings();
    }
}

void US_UI_SettingsWidget::HandleRevertClicked()
{
    // Revert settings in all tabs
    for (US_UI_SettingsTabBase* Tab : SettingsTabs)
    {
        if (Tab)
        {
            Tab->RevertSettings();
        }
    }

    // Revert through view model
    if (ViewModel.IsValid())
    {
        ViewModel->RevertChanges();
    }
}

void US_UI_SettingsWidget::HandleBackClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        // Check if any tab has unsaved changes
        bool bHasUnsavedChanges = false;
        for (US_UI_SettingsTabBase* Tab : SettingsTabs)
        {
            if (Tab && Tab->HasUnsavedChanges())
            {
                bHasUnsavedChanges = true;
                break;
            }
        }

        if (bHasUnsavedChanges)
        {
            // Show confirmation modal
            F_UIModalPayload Payload;
            Payload.Message = FText::FromString(TEXT("You have unsaved changes. Do you want to save before leaving?"));
            Payload.ModalType = E_UIModalType::YesNo;

            UISubsystem->RequestModal(Payload, FOnModalDismissedSignature::CreateLambda([this, UISubsystem](bool bConfirmed)
                {
                    if (bConfirmed)
                    {
                        HandleApplyClicked();
                    }
                    UISubsystem->PopContentScreen();
                }));
        }
        else
        {
            UISubsystem->PopContentScreen();
        }
    }
}