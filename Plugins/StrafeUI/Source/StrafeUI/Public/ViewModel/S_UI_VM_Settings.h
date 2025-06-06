// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_Settings.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "S_UI_VM_Settings.generated.h"

/**
 * @class US_UI_VM_Settings
 * @brief ViewModel for the Settings screen.
 *
 * Handles the data and logic for game settings like audio, video, etc.
 */
UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_Settings : public US_UI_ViewModelBase
{
	GENERATED_BODY()

public:
	/** The master volume level (0.0 to 1.0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
	float MasterVolume = 1.0f;

	/** Whether VSync is enabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
	bool bUseVSync = true;

	/** Shadow quality level (e.g., 0=Low, 1=Medium, 2=High). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
	int32 ShadowQuality = 2;

	/**
	 * Applies the current settings. This would typically save them to a config file.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplySettings();

	/**
	 * Reverts any pending changes back to their last applied state.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RevertChanges();

	/**
	 * Restores all settings to their default values.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RestoreDefaults();

private:
	// Could hold the state of settings when the screen was opened to support reverting.
	// For simplicity, this is not fully implemented here.
};