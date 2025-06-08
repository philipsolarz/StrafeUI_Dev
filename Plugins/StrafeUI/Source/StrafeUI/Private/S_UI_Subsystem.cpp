// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_Subsystem.cpp

#include "S_UI_Subsystem.h"
#include "S_UI_Settings.h" // Include the new settings header

// Required engine headers
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "CommonButtonBase.h"

// Required project headers
#include "Data/S_UI_ScreenDataAsset.h"
#include "S_UI_InputController.h"
#include "S_UI_ModalStack.h"
#include "S_UI_PlayerController.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_MainMenuWidget.h"
#include "UI/S_UI_CreateGameWidget.h"
#include "UI/S_UI_FindGameWidget.h"
#include "UI/S_UI_SettingsWidget.h"
#include "UI/S_UI_SettingsTabBase.h"
#include "ViewModel/S_UI_VM_CreateGame.h"
#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "ViewModel/S_UI_VM_Settings.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Components/NamedSlot.h"


void US_UI_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Initializing..."));
	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Initialized"));
}

void US_UI_Subsystem::InitializeUIForPlayer(AS_UI_PlayerController* PlayerController)
{
	if (!PlayerController || UIRootWidget)
	{
		return;
	}

	// Store the player controller for use in the async callback.
	InitializingPlayer = PlayerController;

	// Start the async loading process. The rest of the UI initialization
	// will be handled in the OnAllAssetsLoaded callback.
	StartAssetsLoading();
}

void US_UI_Subsystem::StartAssetsLoading()
{
	// Ensure we only start loading once.
	if (bAssetsLoaded || bAreAssetsLoading)
	{
		return;
	}
	bAreAssetsLoading = true;

	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem: Starting asynchronous asset loading..."));

	const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: Cannot find StrafeUISettings!"));
		bAreAssetsLoading = false;
		return;
	}

	// Phase 1: Load the Screen Map Data Asset first, as it defines what other assets we need.
	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	FSoftObjectPath ScreenDataAssetPath = Settings->ScreenMapDataAsset.ToSoftObjectPath();

	// Use a weak ptr to self for safety in the async callback.
	TWeakObjectPtr<US_UI_Subsystem> WeakThis = this;

	StreamableManager.RequestAsyncLoad(ScreenDataAssetPath,
		[WeakThis, ScreenDataAssetPath]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnScreenMapDataAssetLoaded(ScreenDataAssetPath);
			}
		});
}

void US_UI_Subsystem::OnScreenMapDataAssetLoaded(FSoftObjectPath ScreenDataAssetPath)
{
	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem: Screen map data asset loaded."));

	const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
	US_UI_ScreenDataAsset* ScreenData = Cast<US_UI_ScreenDataAsset>(Settings->ScreenMapDataAsset.Get());

	if (!Settings || !ScreenData)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ScreenMapDataAsset or Settings after async load!"));
		bAreAssetsLoading = false;
		return;
	}

	// Phase 2: Assemble a list of ALL UI assets that need to be loaded.
	TArray<FSoftObjectPath> AssetsToLoad;

	// Core Classes
	AssetsToLoad.Add(Settings->RootWidgetClass.ToSoftObjectPath());
	AssetsToLoad.Add(Settings->MainMenuWidgetClass.ToSoftObjectPath());
	AssetsToLoad.Add(Settings->ModalStackClass.ToSoftObjectPath());
	AssetsToLoad.Add(Settings->ModalWidgetClass.ToSoftObjectPath());
	AssetsToLoad.Add(Settings->InputControllerClass.ToSoftObjectPath());
	AssetsToLoad.Add(Settings->TabButtonClass.ToSoftObjectPath());

	// Settings Tab Classes
	AssetsToLoad.Add(Settings->AudioSettingsTabClass.ToSoftObjectPath());
	AssetsToLoad.Add(Settings->VideoSettingsTabClass.ToSoftObjectPath());
	AssetsToLoad.Add(Settings->ControlsSettingsTabClass.ToSoftObjectPath());
	AssetsToLoad.Add(Settings->GameplaySettingsTabClass.ToSoftObjectPath());

	// Screen Widget Classes from the Data Asset
	for (const F_UIScreenDefinition& Definition : ScreenData->ScreenDefinitions)
	{
		if (!Definition.WidgetClass.IsNull())
		{
			AssetsToLoad.Add(Definition.WidgetClass.ToSoftObjectPath());
		}
	}

	// Now, request the async load for the entire batch.
	FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
	TWeakObjectPtr<US_UI_Subsystem> WeakThis = this;
	AllAssetsHandle = StreamableManager.RequestAsyncLoad(AssetsToLoad,
		[WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->OnAllAssetsLoaded();
			}
		});
}

void US_UI_Subsystem::OnAllAssetsLoaded()
{
	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem: All UI assets finished loading."));

	const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("Settings are null in OnAllAssetsLoaded. Aborting UI setup."));
		bAreAssetsLoading = false;
		return;
	}

	// --- Populate Screen Widget Cache ---
	const US_UI_ScreenDataAsset* ScreenData = Cast<US_UI_ScreenDataAsset>(Settings->ScreenMapDataAsset.Get());
	if (ScreenData)
	{
		for (const F_UIScreenDefinition& Definition : ScreenData->ScreenDefinitions)
		{
			if (UClass* LoadedClass = Definition.WidgetClass.Get())
			{
				ScreenWidgetClassCache.Add(Definition.ScreenId, LoadedClass);
				UE_LOG(LogTemp, Log, TEXT("Cached screen %s -> %s"), *UEnum::GetValueAsString(Definition.ScreenId), *LoadedClass->GetName());
			}
		}
	}

	// --- Initialize Modal Stack ---
	if (!ModalStack)
	{
		if (UClass* LoadedModalStackClass = Settings->ModalStackClass.Get())
		{
			ModalStack = NewObject<US_UI_ModalStack>(this, LoadedModalStackClass);
			if (UClass* LoadedModalWidgetClass = Settings->ModalWidgetClass.Get())
			{
				ModalStack->Initialize(this, TSoftClassPtr<US_UI_ModalWidget>(LoadedModalWidgetClass));
			}
		}
	}

	// --- Initialize UI for the Player (original logic from InitializeUIForPlayer) ---
	if (InitializingPlayer.IsValid())
	{
		AS_UI_PlayerController* PlayerController = InitializingPlayer.Get();

		// 1. Initialize the Input Controller
		if (TSubclassOf<US_UI_InputController> LoadedInputControllerClass = TSubclassOf<US_UI_InputController>(Settings->InputControllerClass.Get()))
		{
			InputController = NewObject<US_UI_InputController>(this, LoadedInputControllerClass);
			if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
			{
				InputController->Initialize(this, EIC, Settings);
			}
		}

		// 2. Create and add the Root UI Widget
		if (const TSubclassOf<US_UI_RootWidget> RootClass = TSubclassOf<US_UI_RootWidget>(Settings->RootWidgetClass.Get()))
		{
			UIRootWidget = CreateWidget<US_UI_RootWidget>(PlayerController, RootClass);
			if (UIRootWidget)
			{
				UIRootWidget->AddToViewport();

				// 3. Create and add the persistent Main Menu widget
				if (const TSubclassOf<US_UI_MainMenuWidget> MainMenuClass = TSubclassOf<US_UI_MainMenuWidget>(Settings->MainMenuWidgetClass.Get()))
				{
					US_UI_MainMenuWidget* MainMenuWidget = CreateWidget<US_UI_MainMenuWidget>(PlayerController, MainMenuClass);
					if (MainMenuWidget && UIRootWidget->GetMainMenuSlot())
					{
						UIRootWidget->GetMainMenuSlot()->AddChild(MainMenuWidget);
					}
				}
			}
		}
	}

	// All assets are loaded and core UI is built.
	bAssetsLoaded = true;
	bAreAssetsLoading = false;

	// If a screen switch was requested while we were loading, execute it now.
	if (PendingScreenRequest != E_UIScreenId::None)
	{
		UE_LOG(LogTemp, Log, TEXT("Processing pending screen request: %s"), *UEnum::GetValueAsString(PendingScreenRequest));
		SwitchContentScreen(PendingScreenRequest);
		PendingScreenRequest = E_UIScreenId::None;
	}
}

void US_UI_Subsystem::Deinitialize()
{
	if (UIRootWidget)
	{
		UIRootWidget->RemoveFromParent();
		UIRootWidget = nullptr;
	}
	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Deinitialized"));
	Super::Deinitialize();
}


void US_UI_Subsystem::SwitchContentScreen(const E_UIScreenId ScreenId)
{
	// If assets are still loading, queue the request and handle it once loading is complete.
	if (!bAssetsLoaded)
	{
		if (bAreAssetsLoading)
		{
			UE_LOG(LogTemp, Warning, TEXT("SwitchContentScreen called for %s while assets are still loading. Request is queued."), *UEnum::GetValueAsString(ScreenId));
			PendingScreenRequest = ScreenId;
		}
		else
		{
			// This case can happen if UI is requested before InitializeUIForPlayer is called.
			// We should start the loading process.
			UE_LOG(LogTemp, Warning, TEXT("SwitchContentScreen called before asset loading has started. Initiating now."));
			PendingScreenRequest = ScreenId;
			StartAssetsLoading();
		}
		return;
	}

	if (ScreenId == E_UIScreenId::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("SwitchContentScreen failed: Invalid ScreenId 'None' provided."));
		return;
	}

	if (!UIRootWidget || !UIRootWidget->GetContentStack())
	{
		UE_LOG(LogTemp, Error, TEXT("SwitchContentScreen failed: UIRootWidget or its ContentStack is null."));
		return;
	}

	// For tab-like behavior, clear any existing widgets in the content stack
	UIRootWidget->GetContentStack()->ClearWidgets();

	// Find the widget class from our cache
	if (const TSubclassOf<UCommonActivatableWidget>* FoundWidgetClass = ScreenWidgetClassCache.Find(ScreenId))
	{
		// Add the new widget to the root's content stack. This returns the created instance.
		UCommonActivatableWidget* PushedWidget = UIRootWidget->GetContentStack()->AddWidget<UCommonActivatableWidget>(*FoundWidgetClass);
		UE_LOG(LogTemp, Verbose, TEXT("Switched content screen to: %s"), *UEnum::GetValueAsString(ScreenId));

		// --- ViewModel Injection using the Factory Pattern ---
		if (IViewModelProvider* ViewModelProvider = Cast<IViewModelProvider>(PushedWidget))
		{
			US_UI_ViewModelBase* ViewModel = ViewModelProvider->CreateViewModel();

			// The widget is responsible for casting and setting its own viewmodel.
			// We need a way to pass the created viewmodel to the widget.
			// Let's assume a SetViewModel function on the base widget for now.
			if (US_UI_BaseScreenWidget* BaseScreenWidget = Cast<US_UI_BaseScreenWidget>(PushedWidget))
			{
				// This is a bit of a hack, we need a common SetViewModel on the base class
				// or another interface. For now, we'll cast to each type.
				if (US_UI_FindGameWidget* FindGameWidget = Cast<US_UI_FindGameWidget>(BaseScreenWidget))
				{
					FindGameWidget->SetViewModel(ViewModel);
				}
				else if (US_UI_SettingsWidget* SettingsWidget = Cast<US_UI_SettingsWidget>(BaseScreenWidget))
				{
					SettingsWidget->SetViewModel(ViewModel);
				}
				else if (US_UI_CreateGameWidget* CreateGameWidget = Cast<US_UI_CreateGameWidget>(BaseScreenWidget))
				{
					CreateGameWidget->SetViewModel(ViewModel);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SwitchContentScreen failed: No widget class found for ScreenId %s."), *UEnum::GetValueAsString(ScreenId));
	}
}

void US_UI_Subsystem::PopContentScreen()
{
	if (UIRootWidget && UIRootWidget->GetContentStack())
	{
		if (UCommonActivatableWidget* ActiveWidget = UIRootWidget->GetContentStack()->GetActiveWidget())
		{
			ActiveWidget->DeactivateWidget();
			UE_LOG(LogTemp, Verbose, TEXT("Popping current content screen."));
		}
	}
}

void US_UI_Subsystem::RequestModal(const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback)
{
	// This function might be called before assets are loaded.
	// The modal stack itself is created during the async loading process.
	if (!bAssetsLoaded)
	{
		// A robust solution would queue the modal request.
		// For this refactor, we'll log a warning and drop it. In a real project, queuing would be better.
		UE_LOG(LogTemp, Warning, TEXT("RequestModal called before core assets are loaded. The request will be ignored."));
		if (!bAreAssetsLoading) { StartAssetsLoading(); } // Try to start loading if not already in progress.
		return;
	}

	if (ModalStack)
	{
		ModalStack->QueueModal(Payload, OnDismissedCallback);
		UE_LOG(LogTemp, Verbose, TEXT("Modal requested with message: %s"), *Payload.Message.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("RequestModal failed: ModalStack is not initialized!"));
	}
}