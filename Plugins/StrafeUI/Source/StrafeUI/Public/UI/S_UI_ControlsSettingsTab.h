// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_ControlsSettingsTab.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "S_UI_ControlsSettingsTab.generated.h"

class USlider;
class UCheckBox;
class UTextBlock;

/**
 * Controls settings tab content.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_ControlsSettingsTab : public US_UI_SettingsTabBase
{
    GENERATED_BODY()

public:
    virtual void ApplySettings() override;
    virtual void RevertSettings() override;
    virtual bool HasUnsavedChanges() const override;

protected:
    virtual void NativeOnInitialized() override;
    virtual void OnViewModelDataChanged() override;

private:
    UFUNCTION()
    void OnMouseSensitivityChanged(float Value);

    UFUNCTION()
    void OnInvertYChanged(bool bIsChecked);

    // UI Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_MouseSensitivity;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_MouseSensitivityValue;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> Chk_InvertY;

    // Cached original values for revert functionality
    float OriginalMouseSensitivity = 1.0f;
    bool bOriginalInvertY = false;
};

