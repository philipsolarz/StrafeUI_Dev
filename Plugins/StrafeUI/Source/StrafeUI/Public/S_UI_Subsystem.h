#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_Subsystem.generated.h"

class US_UI_AssetManager;
class US_UI_Navigator;
class US_UI_InputController;
class US_UI_ModalStack;
class US_UI_RootWidget;
class AS_UI_PlayerController;
class US_UI_OnlineSessionManager;

/**
 * Container for per-player UI state
 */
USTRUCT()
struct FPlayerUIState
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<US_UI_RootWidget> UIRootWidget = nullptr;

    UPROPERTY()
    TObjectPtr<US_UI_Navigator> Navigator = nullptr;

    UPROPERTY()
    TObjectPtr<US_UI_InputController> InputController = nullptr;

    UPROPERTY()
    TObjectPtr<US_UI_ModalStack> ModalStack = nullptr;

    UPROPERTY()
    TWeakObjectPtr<AS_UI_PlayerController> PlayerController = nullptr;

    bool bIsInitialized = false;
};

/**
 * The central orchestrator for the StrafeUI plugin.
 * Now supports multiple local players with separate UI instances.
 */
UCLASS()
class STRAFEUI_API US_UI_Subsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Kicks off the full UI initialization for a given player. */
    void InitializeUIForPlayer(AS_UI_PlayerController* PlayerController);

    /** Requests a modal dialog to be displayed for a specific player. */
    void RequestModal(AS_UI_PlayerController* PlayerController, const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback);

    /** Gets the root UI widget for a specific player. */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    US_UI_RootWidget* GetRootWidget(AS_UI_PlayerController* PlayerController) const;

    /** Gets the screen navigation manager for a specific player. */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    US_UI_Navigator* GetNavigator(AS_UI_PlayerController* PlayerController) const;

    /** Gets the UI asset loading manager (shared). */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    US_UI_AssetManager* GetAssetManager() const { return AssetManager; }

    /** Gets the online session manager (shared). */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    US_UI_OnlineSessionManager* GetSessionManager() const { return SessionManager; }

    /** Gets the local player index for a player controller. */
    UFUNCTION(BlueprintPure, Category = "UI Subsystem")
    int32 GetLocalPlayerIndex(AS_UI_PlayerController* PlayerController) const;

    /** Cleans up UI for a specific player. */
    void CleanupPlayerUI(AS_UI_PlayerController* PlayerController);

private:
    /** Finalizes UI setup after all assets have been loaded for a specific player. */
    void FinalizeUIInitialization(AS_UI_PlayerController* PlayerController);

    /** Gets or creates the UI state for a player. */
    FPlayerUIState* GetOrCreatePlayerUIState(AS_UI_PlayerController* PlayerController);

    /** Gets the UI state for a player. */
    FPlayerUIState* GetPlayerUIState(AS_UI_PlayerController* PlayerController) const;

    /** Manager for loading UI assets (shared across all players). */
    UPROPERTY()
    TObjectPtr<US_UI_AssetManager> AssetManager;

    /** Manager for online sessions (shared across all players). */
    UPROPERTY()
    TObjectPtr<US_UI_OnlineSessionManager> SessionManager;

    /** Map of player controllers to their UI state. */
    UPROPERTY()
    TMap<TObjectPtr<AS_UI_PlayerController>, FPlayerUIState> PlayerUIStates;

    /** Tracks which players are currently initializing. */
    UPROPERTY()
    TArray<TWeakObjectPtr<AS_UI_PlayerController>> InitializingPlayers;
};