// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_SettingsWidget.cpp

#include "UI/S_UI_SettingsWidget.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"

void US_UI_SettingsWidget::SetViewModel(US_UI_VM_Settings* InViewModel)
{
    if (InViewModel)
    {
        ViewModel = InViewModel;
        ViewModel->OnDataChanged.AddUniqueDynamic(this, &US_UI_SettingsWidget::OnSettingsUpdated);
        OnSettingsUpdated(); // Initial population of UI
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
    if (Slider_MasterVolume)
    {
        Slider_MasterVolume->OnValueChanged.AddDynamic(this, &US_UI_SettingsWidget::OnVolumeSliderChanged);
    }
    if (CheckBox_VSync)
    {
        CheckBox_VSync->OnCheckStateChanged.AddDynamic(this, &US_UI_SettingsWidget::OnVSyncCheckStateChanged);
    }
}

void US_UI_SettingsWidget::OnSettingsUpdated()
{
    if (ViewModel.IsValid())
    {
        // Update the UI controls with values from the ViewModel.
        if (Slider_MasterVolume)
        {
            Slider_MasterVolume->SetValue(ViewModel->MasterVolume);
        }
        if (CheckBox_VSync)
        {
            CheckBox_VSync->SetIsChecked(ViewModel->bUseVSync);
        }
        // ... update other controls like ShadowQuality (e.g., a ComboBox)
    }
}

void US_UI_SettingsWidget::OnVolumeSliderChanged(float Value)
{
    if (ViewModel.IsValid())
    {
        ViewModel->MasterVolume = Value;
    }
}

void US_UI_SettingsWidget::OnVSyncCheckStateChanged(bool bIsChecked)
{
    if (ViewModel.IsValid())
    {
        ViewModel->bUseVSync = bIsChecked;
    }
}

void US_UI_SettingsWidget::HandleApplyClicked()
{
    if (ViewModel.IsValid())
    {
        ViewModel->ApplySettings();
    }
}

void US_UI_SettingsWidget::HandleRevertClicked()
{
    if (ViewModel.IsValid())
    {
        ViewModel->RevertChanges();
    }
}

void US_UI_SettingsWidget::HandleBackClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        // Optionally, ask the user if they want to apply changes before leaving.
        UISubsystem->PopScreen();
    }
}