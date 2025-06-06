// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_SettingsWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "ViewModel/S_UI_VM_Settings.h"
#include "S_UI_SettingsWidget.generated.h"

class UCommonButtonBase;
class USlider;
class UCheckBox;

/**
 * @class S_UI_SettingsWidget
 * @brief The C++ base for the Settings screen.
 * Binds to a ViewModel to display and modify game settings.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_SettingsWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

public:
    /** Sets the ViewModel for this widget and triggers the initial UI update. */
    void SetViewModel(US_UI_VM_Settings* InViewModel);

protected:
    virtual void NativeOnInitialized() override;

private:
    /** Called when the ViewModel's data has changed, refreshing the UI controls. */
    UFUNCTION()
    void OnSettingsUpdated();

    /** Called when the VSync checkbox state changes. */
    UFUNCTION()
    void OnVSyncCheckStateChanged(bool bIsChecked);

    /** Called when the volume slider value changes. */
    UFUNCTION()
    void OnVolumeSliderChanged(float Value);

    //~ Button Click Handlers
    UFUNCTION()
    void HandleApplyClicked();

    UFUNCTION()
    void HandleRevertClicked();

    UFUNCTION()
    void HandleBackClicked();

    /** The ViewModel that provides data for this widget. */
    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_Settings> ViewModel;

    //~ UPROPERTY Bindings for UI elements
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<USlider> Slider_MasterVolume;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCheckBox> CheckBox_VSync;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Apply;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Revert;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;
};