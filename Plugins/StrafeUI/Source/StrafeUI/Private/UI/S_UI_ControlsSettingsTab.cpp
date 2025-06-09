// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_ControlsSettingsTab.cpp

#include "UI/S_UI_ControlsSettingsTab.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "CommonButtonBase.h"
#include "UI/S_UI_KeyBindingWidget.h"
#include "ViewModel/S_UI_VM_Settings.h"
#include "GameFramework/InputSettings.h"

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

    if (Btn_ResetToDefaults)
    {
        Btn_ResetToDefaults->OnClicked().AddUObject(this, &US_UI_ControlsSettingsTab::OnResetToDefaultsClicked);
    }

    // Load current input bindings
    LoadInputBindings();
}

void US_UI_ControlsSettingsTab::NativeConstruct()
{
    Super::NativeConstruct();

    // Populate key binding widgets
    PopulateKeyBindings();
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

void US_UI_ControlsSettingsTab::LoadInputBindings()
{
    InputBindings.Empty();

    // Define default input actions
    // Movement
    FInputActionBinding MoveForward;
    MoveForward.ActionName = "MoveForward";
    MoveForward.DisplayName = FText::FromString("Move Forward");
    MoveForward.PrimaryKey = EKeys::W;
    MoveForward.SecondaryKey = EKeys::Up;
    MoveForward.Category = "Movement";
    InputBindings.Add(MoveForward);

    FInputActionBinding MoveBackward;
    MoveBackward.ActionName = "MoveBackward";
    MoveBackward.DisplayName = FText::FromString("Move Backward");
    MoveBackward.PrimaryKey = EKeys::S;
    MoveBackward.SecondaryKey = EKeys::Down;
    MoveBackward.Category = "Movement";
    InputBindings.Add(MoveBackward);

    FInputActionBinding MoveLeft;
    MoveLeft.ActionName = "MoveLeft";
    MoveLeft.DisplayName = FText::FromString("Move Left");
    MoveLeft.PrimaryKey = EKeys::A;
    MoveLeft.SecondaryKey = EKeys::Left;
    MoveLeft.Category = "Movement";
    InputBindings.Add(MoveLeft);

    FInputActionBinding MoveRight;
    MoveRight.ActionName = "MoveRight";
    MoveRight.DisplayName = FText::FromString("Move Right");
    MoveRight.PrimaryKey = EKeys::D;
    MoveRight.SecondaryKey = EKeys::Right;
    MoveRight.Category = "Movement";
    InputBindings.Add(MoveRight);

    FInputActionBinding Jump;
    Jump.ActionName = "Jump";
    Jump.DisplayName = FText::FromString("Jump");
    Jump.PrimaryKey = EKeys::SpaceBar;
    Jump.Category = "Movement";
    InputBindings.Add(Jump);

    FInputActionBinding Crouch;
    Crouch.ActionName = "Crouch";
    Crouch.DisplayName = FText::FromString("Crouch");
    Crouch.PrimaryKey = EKeys::LeftControl;
    Crouch.SecondaryKey = EKeys::C;
    Crouch.Category = "Movement";
    InputBindings.Add(Crouch);

    // Combat
    FInputActionBinding Fire;
    Fire.ActionName = "Fire";
    Fire.DisplayName = FText::FromString("Fire");
    Fire.PrimaryKey = EKeys::LeftMouseButton;
    Fire.Category = "Combat";
    InputBindings.Add(Fire);

    FInputActionBinding AltFire;
    AltFire.ActionName = "AltFire";
    AltFire.DisplayName = FText::FromString("Alt Fire / Aim");
    AltFire.PrimaryKey = EKeys::RightMouseButton;
    AltFire.Category = "Combat";
    InputBindings.Add(AltFire);

    FInputActionBinding Reload;
    Reload.ActionName = "Reload";
    Reload.DisplayName = FText::FromString("Reload");
    Reload.PrimaryKey = EKeys::R;
    Reload.Category = "Combat";
    InputBindings.Add(Reload);

    // In a real implementation, load from saved settings
    OriginalInputBindings = InputBindings;
}

void US_UI_ControlsSettingsTab::PopulateKeyBindings()
{
    if (!ScrollBox_KeyBindings || !KeyBindingWidgetClass)
    {
        return;
    }

    // Clear existing widgets
    ScrollBox_KeyBindings->ClearChildren();
    KeyBindingWidgets.Empty();

    FString LastCategory;

    for (const FInputActionBinding& Binding : InputBindings)
    {
        // Add category header if changed
        if (Binding.Category != LastCategory)
        {
            LastCategory = Binding.Category;

            // Create category header (you'd have a proper widget for this)
            UTextBlock* CategoryHeader = NewObject<UTextBlock>(this);
            CategoryHeader->SetText(FText::FromString(Binding.Category));
            CategoryHeader->SetFont(FSlateFontInfo(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14));
            ScrollBox_KeyBindings->AddChild(CategoryHeader);
        }

        // Create primary key binding widget
        CreateKeyBindingWidget(Binding);
    }
}

void US_UI_ControlsSettingsTab::CreateKeyBindingWidget(const FInputActionBinding& Binding)
{
    if (!KeyBindingWidgetClass || !ScrollBox_KeyBindings)
    {
        return;
    }

    US_UI_KeyBindingWidget* KeyBindWidget = CreateWidget<US_UI_KeyBindingWidget>(this, KeyBindingWidgetClass);
    if (KeyBindWidget)
    {
        KeyBindWidget->ActionName = Binding.ActionName;
        KeyBindWidget->ActionDisplayName = Binding.DisplayName;
        KeyBindWidget->SetKeyBinding(Binding.PrimaryKey);
        KeyBindWidget->OnKeyBindingChanged.AddDynamic(this, &US_UI_ControlsSettingsTab::OnKeyBindingChanged);

        ScrollBox_KeyBindings->AddChild(KeyBindWidget);
        KeyBindingWidgets.Add(KeyBindWidget);
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

void US_UI_ControlsSettingsTab::OnKeyBindingChanged(FName ActionName, FKey NewKey)
{
    // Update the binding in our array
    for (FInputActionBinding& Binding : InputBindings)
    {
        if (Binding.ActionName == ActionName)
        {
            Binding.PrimaryKey = NewKey;
            break;
        }
    }
}

void US_UI_ControlsSettingsTab::OnResetToDefaultsClicked()
{
    // Reset mouse settings
    if (ViewModel.IsValid())
    {
        ViewModel->MouseSensitivity = 1.0f;
        ViewModel->bInvertYAxis = false;
    }

    // Reset key bindings
    LoadInputBindings(); // This reloads defaults
    PopulateKeyBindings();

    // Update UI
    OnViewModelDataChanged();
}

void US_UI_ControlsSettingsTab::ApplySettings()
{
    if (ViewModel.IsValid())
    {
        OriginalMouseSensitivity = ViewModel->MouseSensitivity;
        bOriginalInvertY = ViewModel->bInvertYAxis;
    }

    OriginalInputBindings = InputBindings;
    SaveInputBindings();
}

void US_UI_ControlsSettingsTab::RevertSettings()
{
    if (ViewModel.IsValid())
    {
        ViewModel->MouseSensitivity = OriginalMouseSensitivity;
        ViewModel->bInvertYAxis = bOriginalInvertY;
    }

    InputBindings = OriginalInputBindings;
    PopulateKeyBindings();

    // Update UI
    OnViewModelDataChanged();
}

bool US_UI_ControlsSettingsTab::HasUnsavedChanges() const
{
    if (!ViewModel.IsValid())
    {
        return false;
    }

    // Check basic settings
    if (FMath::Abs(ViewModel->MouseSensitivity - OriginalMouseSensitivity) > KINDA_SMALL_NUMBER ||
        ViewModel->bInvertYAxis != bOriginalInvertY)
    {
        return true;
    }

    // Check key bindings
    if (InputBindings.Num() != OriginalInputBindings.Num())
    {
        return true;
    }

    for (int32 i = 0; i < InputBindings.Num(); ++i)
    {
        if (InputBindings[i].PrimaryKey != OriginalInputBindings[i].PrimaryKey ||
            InputBindings[i].SecondaryKey != OriginalInputBindings[i].SecondaryKey)
        {
            return true;
        }
    }

    return false;
}

void US_UI_ControlsSettingsTab::SaveInputBindings()
{
    // In a real implementation, save to input settings config
    UE_LOG(LogTemp, Log, TEXT("Saving input bindings..."));
}