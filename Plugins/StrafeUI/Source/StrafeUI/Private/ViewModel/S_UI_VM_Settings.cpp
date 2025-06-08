// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_Settings.cpp

#include "ViewModel/S_UI_VM_Settings.h"
#include "GameFramework/GameUserSettings.h"
#include "Engine/Engine.h"

void US_UI_VM_Settings::ApplySettings()
{
    // Apply audio settings
    if (UAudioSettings* AudioSettings = GetMutableDefault<UAudioSettings>())
    {
        // In a real implementation, apply audio settings to the audio system
        UE_LOG(LogTemp, Log, TEXT("Applying audio settings: Master=%f, Music=%f, SFX=%f, Voice=%f"),
            MasterVolume, MusicVolume, SFXVolume, VoiceVolume);
    }

    // Apply video settings
    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        UserSettings->SetVSyncEnabled(bUseVSync);
        UserSettings->SetShadowQuality(ShadowQuality);
        UserSettings->SetTextureQuality(TextureQuality);
        UserSettings->SetAntiAliasingQuality(AntiAliasingMode);

        UserSettings->ApplySettings(false);
    }

    // Apply controls settings
    // In a real implementation, save to input config

    // Apply gameplay settings
    // In a real implementation, apply FOV and other gameplay settings

    // Cache the applied settings
    CacheCurrentSettings();

    UE_LOG(LogTemp, Log, TEXT("Settings applied successfully"));
    BroadcastDataChanged();
}

void US_UI_VM_Settings::RevertChanges()
{
    RestoreFromCache();
    BroadcastDataChanged();
    UE_LOG(LogTemp, Log, TEXT("Settings reverted"));
}

void US_UI_VM_Settings::RestoreDefaults()
{
    // Audio defaults
    MasterVolume = 1.0f;
    MusicVolume = 1.0f;
    SFXVolume = 1.0f;
    VoiceVolume = 1.0f;

    // Video defaults
    bUseVSync = true;
    ShadowQuality = 2;
    TextureQuality = 2;
    AntiAliasingMode = 2;
    ResolutionIndex = 0;
    WindowMode = 0;

    // Controls defaults
    MouseSensitivity = 1.0f;
    bInvertYAxis = false;

    // Gameplay defaults
    FieldOfView = 90.0f;
    bShowFPSCounter = false;

    BroadcastDataChanged();
    UE_LOG(LogTemp, Log, TEXT("Settings restored to defaults"));
}

void US_UI_VM_Settings::LoadSettings()
{
    // Load from game user settings
    if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
    {
        bUseVSync = UserSettings->IsVSyncEnabled();
        ShadowQuality = UserSettings->GetShadowQuality();
        TextureQuality = UserSettings->GetTextureQuality();
        AntiAliasingMode = UserSettings->GetAntiAliasingQuality();

        // Load other settings from config files
        // In a real implementation, load all settings from appropriate sources
    }

    CacheCurrentSettings();
    BroadcastDataChanged();
}

void US_UI_VM_Settings::CacheCurrentSettings()
{
    // Audio
    CachedSettings.MasterVolume = MasterVolume;
    CachedSettings.MusicVolume = MusicVolume;
    CachedSettings.SFXVolume = SFXVolume;
    CachedSettings.VoiceVolume = VoiceVolume;

    // Video
    CachedSettings.bUseVSync = bUseVSync;
    CachedSettings.ShadowQuality = ShadowQuality;
    CachedSettings.TextureQuality = TextureQuality;
    CachedSettings.AntiAliasingMode = AntiAliasingMode;
    CachedSettings.ResolutionIndex = ResolutionIndex;
    CachedSettings.WindowMode = WindowMode;

    // Controls
    CachedSettings.MouseSensitivity = MouseSensitivity;
    CachedSettings.bInvertYAxis = bInvertYAxis;

    // Gameplay
    CachedSettings.FieldOfView = FieldOfView;
    CachedSettings.bShowFPSCounter = bShowFPSCounter;
}

void US_UI_VM_Settings::RestoreFromCache()
{
    // Audio
    MasterVolume = CachedSettings.MasterVolume;
    MusicVolume = CachedSettings.MusicVolume;
    SFXVolume = CachedSettings.SFXVolume;
    VoiceVolume = CachedSettings.VoiceVolume;

    // Video
    bUseVSync = CachedSettings.bUseVSync;
    ShadowQuality = CachedSettings.ShadowQuality;
    TextureQuality = CachedSettings.TextureQuality;
    AntiAliasingMode = CachedSettings.AntiAliasingMode;
    ResolutionIndex = CachedSettings.ResolutionIndex;
    WindowMode = CachedSettings.WindowMode;

    // Controls
    MouseSensitivity = CachedSettings.MouseSensitivity;
    bInvertYAxis = CachedSettings.bInvertYAxis;

    // Gameplay
    FieldOfView = CachedSettings.FieldOfView;
    bShowFPSCounter = CachedSettings.bShowFPSCounter;
}