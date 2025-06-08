// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_Subsystem.cpp

#include "S_UI_Subsystem.h"
#include "S_UI_Settings.h" // Include the new settings header

// Required engine headers
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

void US_UI_Subsystem::EnsureAssetsLoaded()
{
	if (bAssetsLoaded)
	{
		return;
	}

	const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: Cannot find StrafeUISettings!"));
		return;
	}

	// --- FIX STARTS HERE ---
	// Preemptively load all critical UI classes from Developer Settings.
	// This resolves the soft pointers at a controlled time, preventing race conditions
	// that can occur during editor startup or gameplay initialization. By ensuring these
	// assets are in memory before any UI is created, we make the system more robust.

	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem: Pre-loading UI assets from settings..."));

	// Core classes required by other systems (like the TabControl)
	if (Settings->TabButtonClass.IsPending())
	{
		Settings->TabButtonClass.LoadSynchronous();
	}

	// Settings Tab classes (these were the problematic ones)
	if (Settings->AudioSettingsTabClass.IsPending())
	{
		Settings->AudioSettingsTabClass.LoadSynchronous();
	}
	if (Settings->VideoSettingsTabClass.IsPending())
	{
		Settings->VideoSettingsTabClass.LoadSynchronous();
	}
	if (Settings->ControlsSettingsTabClass.IsPending())
	{
		Settings->ControlsSettingsTabClass.LoadSynchronous();
	}
	if (Settings->GameplaySettingsTabClass.IsPending())
	{
		Settings->GameplaySettingsTabClass.LoadSynchronous();
	}
	// --- FIX ENDS HERE ---

	// Initialize the modal stack if not already created
	if (!ModalStack)
	{
		// Force load the modal stack class
		UClass* LoadedModalStackClass = Settings->ModalStackClass.LoadSynchronous();
		if (LoadedModalStackClass)
		{
			ModalStack = NewObject<US_UI_ModalStack>(this, LoadedModalStackClass);

			// Force load the modal widget class before initializing
			UClass* LoadedModalWidgetClass = Settings->ModalWidgetClass.LoadSynchronous();
			if (LoadedModalWidgetClass)
			{
				ModalStack->Initialize(this, TSoftClassPtr<US_UI_ModalWidget>(LoadedModalWidgetClass));
				UE_LOG(LogTemp, Log, TEXT("Modal stack initialized with widget class: %s"),
					*LoadedModalWidgetClass->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load ModalWidgetClass!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to load ModalStackClass!"));
		}
	}

	// Force load the screen data asset
	UObject* LoadedAsset = Settings->ScreenMapDataAsset.LoadSynchronous();
	if (!LoadedAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ScreenMapDataAsset!"));
		return;
	}

	if (const US_UI_ScreenDataAsset* ScreenData = Cast<US_UI_ScreenDataAsset>(LoadedAsset))
	{
		for (const F_UIScreenDefinition& Definition : ScreenData->ScreenDefinitions)
		{
			if (Definition.WidgetClass.IsValid() || Definition.WidgetClass.IsPending())
			{
				UClass* LoadedClass = Definition.WidgetClass.LoadSynchronous();
				if (LoadedClass)
				{
					ScreenWidgetClassCache.Add(Definition.ScreenId, LoadedClass);
					UE_LOG(LogTemp, Log, TEXT("Loaded screen %s -> %s"),
						*UEnum::GetValueAsString(Definition.ScreenId),
						*LoadedClass->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Failed to load widget class for %s"),
						*UEnum::GetValueAsString(Definition.ScreenId));
				}
			}
		}
	}

	bAssetsLoaded = true;
}

void US_UI_Subsystem::InitializeUIForPlayer(AS_UI_PlayerController* PlayerController)
{
	if (!PlayerController || UIRootWidget)
	{
		return;
	}

	// Ensure assets are loaded before creating UI
	EnsureAssetsLoaded();

	const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
	if (!Settings)
	{
		return;
	}

	// 1. Initialize the Input Controller now that we have a valid Player Controller
	if (TSubclassOf<US_UI_InputController> LoadedInputControllerClass = Settings->InputControllerClass.LoadSynchronous())
	{
		InputController = NewObject<US_UI_InputController>(this, LoadedInputControllerClass);
		if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			InputController->Initialize(this, EIC, Settings);
		}
	}

	// 2. Create and add the Root UI Widget to the viewport
	if (const TSubclassOf<US_UI_RootWidget> RootClass = Settings->RootWidgetClass.LoadSynchronous())
	{
		UIRootWidget = CreateWidget<US_UI_RootWidget>(PlayerController, RootClass);
		if (UIRootWidget)
		{
			UIRootWidget->AddToViewport();

			// 3. Create and add the persistent Main Menu widget
			if (const TSubclassOf<US_UI_MainMenuWidget> MainMenuClass = Settings->MainMenuWidgetClass.LoadSynchronous())
			{
				US_UI_MainMenuWidget* MainMenuWidget = CreateWidget<US_UI_MainMenuWidget>(PlayerController, MainMenuClass);
				if (MainMenuWidget && UIRootWidget->GetMainMenuSlot())
				{
					UIRootWidget->GetMainMenuSlot()->AddChild(MainMenuWidget);
					UE_LOG(LogTemp, Log, TEXT("Main Menu widget added to root layout."));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: MainMenuWidgetClass is not set in Project Settings -> Strafe UI!"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: RootWidgetClass is not set in Project Settings -> Strafe UI!"));
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

		// --- ViewModel Injection ---
		if (US_UI_FindGameWidget* FindGameWidget = Cast<US_UI_FindGameWidget>(PushedWidget))
		{
			US_UI_VM_ServerBrowser* ServerBrowserVM = NewObject<US_UI_VM_ServerBrowser>(this);
			FindGameWidget->SetViewModel(ServerBrowserVM);
		}
		else if (US_UI_SettingsWidget* SettingsWidget = Cast<US_UI_SettingsWidget>(PushedWidget))
		{
			US_UI_VM_Settings* SettingsVM = NewObject<US_UI_VM_Settings>(this);
			SettingsWidget->SetViewModel(SettingsVM);
		}
		else if (US_UI_CreateGameWidget* CreateGameWidget = Cast<US_UI_CreateGameWidget>(PushedWidget))
		{
			US_UI_VM_CreateGame* CreateGameVM = NewObject<US_UI_VM_CreateGame>(this);
			// Pass the settings object to the new initializer
			CreateGameVM->Initialize(GetDefault<US_UI_Settings>());
			CreateGameWidget->SetViewModel(CreateGameVM);
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
	// Ensure assets are loaded before trying to use modal stack
	EnsureAssetsLoaded();

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