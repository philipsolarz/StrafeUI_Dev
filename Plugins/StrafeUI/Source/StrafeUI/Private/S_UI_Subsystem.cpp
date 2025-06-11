#include "S_UI_Subsystem.h"
#include "S_UI_Settings.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Components/NamedSlot.h"

#include "S_UI_AssetManager.h"
#include "S_UI_Navigator.h"
#include "S_UI_InputController.h"
#include "S_UI_ModalStack.h"
#include "S_UI_PlayerController.h"
#include "S_UI_OnlineSessionManager.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_MainMenuWidget.h"

void US_UI_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Create the shared manager instances.
    AssetManager = NewObject<US_UI_AssetManager>(this);
    SessionManager = NewObject<US_UI_OnlineSessionManager>(this);

    // Initialize the session manager
    SessionManager->Initialize();

    UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Initialized"));
}

void US_UI_Subsystem::Deinitialize()
{
    // Clean up the session manager first
    if (SessionManager)
    {
        SessionManager->Shutdown();
        SessionManager = nullptr;
    }

    // Clean up all player UI states
    for (auto& PlayerUIStatePair : PlayerUIStates)
    {
        FPlayerUIState& UIState = PlayerUIStatePair.Value;
        if (UIState.UIRootWidget)
        {
            UIState.UIRootWidget->RemoveFromParent();
        }
    }
    PlayerUIStates.Empty();

    UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Deinitialized"));
    Super::Deinitialize();
}

void US_UI_Subsystem::InitializeUIForPlayer(AS_UI_PlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    // Check if this player is already initialized or initializing
    FPlayerUIState* ExistingState = GetPlayerUIState(PlayerController);
    if (ExistingState && ExistingState->bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("S_UI_Subsystem: Player %d already has UI initialized"), GetLocalPlayerIndex(PlayerController));
        return;
    }

    // Check if already initializing
    for (const TWeakObjectPtr<AS_UI_PlayerController>& InitializingPlayer : InitializingPlayers)
    {
        if (InitializingPlayer.IsValid() && InitializingPlayer.Get() == PlayerController)
        {
            UE_LOG(LogTemp, Warning, TEXT("S_UI_Subsystem: Player %d is already initializing"), GetLocalPlayerIndex(PlayerController));
            return;
        }
    }

    // Add to initializing list
    InitializingPlayers.Add(PlayerController);

    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: Cannot find StrafeUISettings!"));
        InitializingPlayers.RemoveSingle(PlayerController);
        return;
    }

    // Initialize asset manager if not already done
    if (!AssetManager->AreAssetsLoaded())
    {
        AssetManager->Initialize(Settings);

        // Bind completion callback for all initializing players
        AssetManager->OnAssetsLoaded.BindLambda([this]()
            {
                // Process all waiting players
                TArray<TWeakObjectPtr<AS_UI_PlayerController>> PlayersToInitialize = InitializingPlayers;
                for (const TWeakObjectPtr<AS_UI_PlayerController>& WeakPC : PlayersToInitialize)
                {
                    if (AS_UI_PlayerController* PC = WeakPC.Get())
                    {
                        FinalizeUIInitialization(PC);
                    }
                }
            });

        AssetManager->StartAssetsLoading();
    }
    else
    {
        // Assets already loaded, finalize immediately
        FinalizeUIInitialization(PlayerController);
    }
}

void US_UI_Subsystem::FinalizeUIInitialization(AS_UI_PlayerController* PlayerController)
{
    UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem: Finalizing UI setup for Player %d..."), GetLocalPlayerIndex(PlayerController));

    if (!PlayerController)
    {
        return;
    }

    // Remove from initializing list
    InitializingPlayers.RemoveSingle(PlayerController);

    // Get or create UI state for this player
    FPlayerUIState* UIState = GetOrCreatePlayerUIState(PlayerController);
    if (!UIState)
    {
        UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: Failed to create UI state for player"));
        return;
    }

    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();

    // --- Initialize Modal Stack ---
    if (!UIState->ModalStack)
    {
        if (TSubclassOf<US_UI_ModalStack> LoadedModalStackClass = TSubclassOf<US_UI_ModalStack>(Settings->ModalStackClass.Get()))
        {
            UIState->ModalStack = NewObject<US_UI_ModalStack>(this, LoadedModalStackClass);
            UIState->ModalStack->Initialize(this, Settings->ModalWidgetClass);
        }
    }

    // --- Initialize Input Controller ---
    if (TSubclassOf<US_UI_InputController> LoadedInputControllerClass = TSubclassOf<US_UI_InputController>(Settings->InputControllerClass.Get()))
    {
        UIState->InputController = NewObject<US_UI_InputController>(this, LoadedInputControllerClass);
        if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
        {
            UIState->InputController->Initialize(this, EIC, Settings);
        }
    }

    // --- Create Root Widget ---
    if (const TSubclassOf<US_UI_RootWidget> RootClass = TSubclassOf<US_UI_RootWidget>(Settings->RootWidgetClass.Get()))
    {
        UIState->UIRootWidget = CreateWidget<US_UI_RootWidget>(PlayerController, RootClass);
        if (UIState->UIRootWidget)
        {
            UIState->UIRootWidget->AddToViewport();

            // Create and add the persistent Main Menu widget
            if (const TSubclassOf<US_UI_MainMenuWidget> MainMenuClass = TSubclassOf<US_UI_MainMenuWidget>(Settings->MainMenuWidgetClass.Get()))
            {
                US_UI_MainMenuWidget* MainMenuWidget = CreateWidget<US_UI_MainMenuWidget>(PlayerController, MainMenuClass);
                if (MainMenuWidget && UIState->UIRootWidget->GetMainMenuSlot())
                {
                    UIState->UIRootWidget->GetMainMenuSlot()->AddChild(MainMenuWidget);
                }
            }
        }
    }

    // --- Initialize Navigator ---
    UIState->Navigator = NewObject<US_UI_Navigator>(this);
    UIState->Navigator->Initialize(UIState->UIRootWidget, AssetManager);

    // Mark as initialized
    UIState->bIsInitialized = true;
    UIState->PlayerController = PlayerController;

    UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem: UI initialized for Player %d"), GetLocalPlayerIndex(PlayerController));
}

void US_UI_Subsystem::RequestModal(AS_UI_PlayerController* PlayerController, const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback)
{
    if (!AssetManager || !AssetManager->AreAssetsLoaded())
    {
        UE_LOG(LogTemp, Warning, TEXT("RequestModal called before core assets are loaded. The request will be ignored."));
        return;
    }

    FPlayerUIState* UIState = GetPlayerUIState(PlayerController);
    if (!UIState || !UIState->ModalStack)
    {
        UE_LOG(LogTemp, Error, TEXT("RequestModal failed: No UI state or ModalStack for player!"));
        return;
    }

    UIState->ModalStack->QueueModal(Payload, OnDismissedCallback);
    UE_LOG(LogTemp, Verbose, TEXT("Modal requested for Player %d with message: %s"),
        GetLocalPlayerIndex(PlayerController), *Payload.Message.ToString());
}

US_UI_RootWidget* US_UI_Subsystem::GetRootWidget(AS_UI_PlayerController* PlayerController) const
{
    const FPlayerUIState* UIState = GetPlayerUIState(PlayerController);
    return UIState ? UIState->UIRootWidget : nullptr;
}

US_UI_Navigator* US_UI_Subsystem::GetNavigator(AS_UI_PlayerController* PlayerController) const
{
    const FPlayerUIState* UIState = GetPlayerUIState(PlayerController);
    return UIState ? UIState->Navigator : nullptr;
}

int32 US_UI_Subsystem::GetLocalPlayerIndex(AS_UI_PlayerController* PlayerController) const
{
    if (!PlayerController)
    {
        return -1;
    }

    if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
    {
        return LocalPlayer->GetControllerId();
    }

    return -1;
}

void US_UI_Subsystem::CleanupPlayerUI(AS_UI_PlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    // Remove from initializing list if present
    InitializingPlayers.RemoveSingle(PlayerController);

    // Clean up UI state
    FPlayerUIState* UIState = GetPlayerUIState(PlayerController);
    if (UIState)
    {
        if (UIState->UIRootWidget)
        {
            UIState->UIRootWidget->RemoveFromParent();
        }
        PlayerUIStates.Remove(PlayerController);
    }

    UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem: Cleaned up UI for Player %d"), GetLocalPlayerIndex(PlayerController));
}

FPlayerUIState* US_UI_Subsystem::GetOrCreatePlayerUIState(AS_UI_PlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return nullptr;
    }

    FPlayerUIState* ExistingState = PlayerUIStates.Find(PlayerController);
    if (ExistingState)
    {
        return ExistingState;
    }

    // Create new state
    FPlayerUIState& NewState = PlayerUIStates.Add(PlayerController);
    NewState.PlayerController = PlayerController;
    return &NewState;
}

FPlayerUIState* US_UI_Subsystem::GetPlayerUIState(AS_UI_PlayerController* PlayerController) const
{
    if (!PlayerController)
    {
        return nullptr;
    }

    return const_cast<TMap<TObjectPtr<AS_UI_PlayerController>, FPlayerUIState>&>(PlayerUIStates).Find(PlayerController);
}