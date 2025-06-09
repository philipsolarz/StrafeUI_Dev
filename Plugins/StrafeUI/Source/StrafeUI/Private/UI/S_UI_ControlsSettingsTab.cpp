// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ControlsSettingsTab.cpp

#include "UI/S_UI_ControlsSettingsTab.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "ViewModel/S_UI_VM_Settings.h"

void US_UI_ControlsSettingsTab::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // Bind UI events
    if (Slider_MouseSensitivity)
    {
        Slider_MouseSensitivity->OnValueChanged.AddDynamic(this, &US_UI_ControlsSettingsTab::OnMouseSensitivityChanged);
        Slider_MouseSensitivity->SetMinValue(0.1f);
        Slider_MouseSensitivity->SetMaxValue(3.0f);
        Slider_MouseSensitivity->SetStepSize(0.1f);
    }

    if (Chk_InvertY)
    {
        Chk_InvertY->OnCheckStateChanged.AddDynamic(this, &US_UI_ControlsSettingsTab::OnInvertYChanged);
    }
}

void US_UI_ControlsSettingsTab::OnViewModelDataChanged()
{
    if (!ViewModel.IsValid())
    {
        return;
    }

    // Update Mouse Sensitivity
    if (Slider_MouseSensitivity)
    {
        Slider_MouseSensitivity->SetValue(ViewModel->MouseSensitivity);
        OriginalMouseSensitivity = ViewModel->MouseSensitivity;
    }

    if (Text_MouseSensitivityValue)
    {
        Text_MouseSensitivityValue->SetText(FText::AsNumber(ViewModel->MouseSensitivity, &FNumberFormattingOptions::DefaultNoGrouping()));
    }

    // Update Invert Y
    if (Chk_InvertY)
    {
        Chk_InvertY->SetIsChecked(ViewModel->bInvertYAxis);
        bOriginalInvertY = ViewModel->bInvertYAxis;
    }
}

void US_UI_ControlsSettingsTab::OnMouseSensitivityChanged(float Value)
{
    if (ViewModel.IsValid())
    {
        ViewModel->MouseSensitivity = Value;

        if (Text_MouseSensitivityValue)
        {
            Text_MouseSensitivityValue->SetText(FText::AsNumber(Value, &FNumberFormattingOptions::DefaultNoGrouping()));
        }
    }
}

void US_UI_ControlsSettingsTab::OnInvertYChanged(bool bIsChecked)
{
    if (ViewModel.IsValid())
    {
        ViewModel->bInvertYAxis = bIsChecked;
    }
}

void US_UI_ControlsSettingsTab::ApplySettings()
{
    if (ViewModel.IsValid())
    {
        OriginalMouseSensitivity = ViewModel->MouseSensitivity;
        bOriginalInvertY = ViewModel->bInvertYAxis;
    }
}

void US_UI_ControlsSettingsTab::RevertSettings()
{
    if (ViewModel.IsValid())
    {
        ViewModel->MouseSensitivity = OriginalMouseSensitivity;
        ViewModel->bInvertYAxis = bOriginalInvertY;

        // Update UI
        OnViewModelDataChanged();
    }
}

bool US_UI_ControlsSettingsTab::HasUnsavedChanges() const
{
    if (!ViewModel.IsValid())
    {
        return false;
    }

    return FMath::Abs(ViewModel->MouseSensitivity - OriginalMouseSensitivity) > KINDA_SMALL_NUMBER ||
        ViewModel->bInvertYAxis != bOriginalInvertY;
}