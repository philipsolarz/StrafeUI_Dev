// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_Settings.cpp

#include "ViewModel/S_UI_VM_Settings.h"

void US_UI_VM_Settings::ApplySettings()
{
	// In a real implementation, you would save these properties
	// to a config file (e.g., UGameUserSettings).
	UE_LOG(LogTemp, Log, TEXT("Applying settings: MasterVolume=%f, VSync=%d, ShadowQuality=%d"),
		MasterVolume, bUseVSync, ShadowQuality);

	// After applying, broadcast that data has changed in case any other system needs to know.
	// For settings, this is less common but good practice.
	BroadcastDataChanged();
}

void US_UI_VM_Settings::RevertChanges()
{
	// In a real implementation, you would reload the settings from the config file,
	// overwriting any changes made in the UI that haven't been applied yet.
	UE_LOG(LogTemp, Log, TEXT("Reverting settings changes."));

	// For this example, we'll just restore to defaults as a stand-in.
	RestoreDefaults();
}

void US_UI_VM_Settings::RestoreDefaults()
{
	UE_LOG(LogTemp, Log, TEXT("Restoring default settings."));

	// Set properties to their default values.
	MasterVolume = 1.0f;
	bUseVSync = true;
	ShadowQuality = 2;

	// Broadcast the change so the UI updates to show the default values.
	BroadcastDataChanged();
}