// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_Settings.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "S_UI_VM_Settings.generated.h"

/**
 * @class US_UI_VM_Settings
 * @brief ViewModel for the Settings screen.
 *
 * Handles the data and logic for all game settings categories.
 */
UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_Settings : public US_UI_ViewModelBase
{
    GENERATED_BODY()

public:
    //~ Audio Settings
    /** The master volume level (0.0 to 1.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
    float MasterVolume = 1.0f;

    /** Music volume level (0.0 to 1.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
    float MusicVolume = 1.0f;

    /** Sound effects volume level (0.0 to 1.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
    float SFXVolume = 1.0f;

    /** Voice/dialogue volume level (0.0 to 1.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Audio")
    float VoiceVolume = 1.0f;

    //~ Video Settings
    /** Whether VSync is enabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    bool bUseVSync = true;

    /** Shadow quality level (0=Low, 1=Medium, 2=High, 3=Ultra). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 ShadowQuality = 2;

    /** Texture quality level (0=Low, 1=Medium, 2=High, 3=Ultra). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 TextureQuality = 2;

    /** Anti-aliasing mode (0=Off, 1=FXAA, 2=TAA, 3=MSAA). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 AntiAliasingMode = 2;

    /** Display resolution index. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 ResolutionIndex = 0;

    /** Window mode (0=Fullscreen, 1=Windowed, 2=Borderless). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Video")
    int32 WindowMode = 0;

    //~ Controls Settings
    /** Mouse sensitivity (0.1 to 2.0). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Controls")
    float MouseSensitivity = 1.0f;

    /** Whether to invert Y axis. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Controls")
    bool bInvertYAxis = false;

    //~ Gameplay Settings
    /** Field of view (60 to 120). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gameplay")
    float FieldOfView = 90.0f;

    /** Whether to show FPS counter. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Gameplay")
    bool bShowFPSCounter = false;

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

    /**
     * Loads settings from the game's config files.
     */
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void LoadSettings();

private:
    // Cached settings for reverting
    struct FCachedSettings
    {
        // Audio
        float MasterVolume = 1.0f;
        float MusicVolume = 1.0f;
        float SFXVolume = 1.0f;
        float VoiceVolume = 1.0f;

        // Video
        bool bUseVSync = true;
        int32 ShadowQuality = 2;
        int32 TextureQuality = 2;
        int32 AntiAliasingMode = 2;
        int32 ResolutionIndex = 0;
        int32 WindowMode = 0;

        // Controls
        float MouseSensitivity = 1.0f;
        bool bInvertYAxis = false;

        // Gameplay
        float FieldOfView = 90.0f;
        bool bShowFPSCounter = false;
    };

    FCachedSettings CachedSettings;

    void CacheCurrentSettings();
    void RestoreFromCache();
};