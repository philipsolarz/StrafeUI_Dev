// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_Subsystem.cpp

#include "S_UI_Subsystem.h"
#include "S_UI_Settings.h" // Include the new settings header

// Required engine headers
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"

// Required project headers
#include "Data/S_UI_ScreenDataAsset.h"
#include "S_UI_InputController.h"
#include "S_UI_ModalStack.h"
#include "S_UI_PlayerController.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_FindGameWidget.h"
#include "UI/S_UI_SettingsWidget.h"
#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "ViewModel/S_UI_VM_Settings.h"
#include "Widgets/CommonActivatableWidgetContainer.h"


void US_UI_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Initializing..."));

	const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("S_UI_Subsystem: Cannot find StrafeUISettings!"));
		return;
	}

	// Pre-load the screen definitions
	if (const US_UI_ScreenDataAsset* ScreenData = Settings->ScreenMapDataAsset.LoadSynchronous())
	{
		for (const F_UIScreenDefinition& Definition : ScreenData->ScreenDefinitions)
		{
			if (Definition.WidgetClass)
			{
				ScreenWidgetClassCache.Add(Definition.ScreenId, Definition.WidgetClass.LoadSynchronous());
			}
		}
	}

	// Initialize the modal stack, as it doesn't depend on a player controller
	if (TSubclassOf<US_UI_ModalStack> LoadedModalStackClass = Settings->ModalStackClass.LoadSynchronous())
	{
		ModalStack = NewObject<US_UI_ModalStack>(this, LoadedModalStackClass);
		ModalStack->Initialize(this, Settings->ModalWidgetClass);
	}

	// NOTE: We no longer create the UI Root or Input Controller here.

	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Initialized"));
}

void US_UI_Subsystem::InitializeUIForPlayer(AS_UI_PlayerController* PlayerController)
{
	if (!PlayerController || UIRootWidget)
	{
		// Prevent initialization if the player is invalid or if UI has already been created.
		return;
	}

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
			// 3. Push the initial screen
			PushScreen(E_UIScreenId::MainMenu);
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


void US_UI_Subsystem::PushScreen(const E_UIScreenId ScreenId)
{
	if (ScreenId == E_UIScreenId::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("PushScreen failed: Invalid ScreenId 'None' provided."));
		return;
	}

	if (!UIRootWidget || !UIRootWidget->GetMainStack())
	{
		UE_LOG(LogTemp, Error, TEXT("PushScreen failed: UIRootWidget or its MainStack is null."));
		return;
	}

	// Find the widget class from our cache
	if (const TSubclassOf<UCommonActivatableWidget>* FoundWidgetClass = ScreenWidgetClassCache.Find(ScreenId))
	{
		// --- CORRECTED FUNCTION CALL ---
		// Add the widget to the root's main stack. This returns the created instance.
		UCommonActivatableWidget* PushedWidget = UIRootWidget->GetMainStack()->AddWidget<UCommonActivatableWidget>(*FoundWidgetClass);
		UE_LOG(LogTemp, Verbose, TEXT("Pushed screen: %s"), *UEnum::GetValueAsString(ScreenId));

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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PushScreen failed: No widget class found for ScreenId %s."), *UEnum::GetValueAsString(ScreenId));
	}
}

void US_UI_Subsystem::PopScreen()
{
	if (UIRootWidget && UIRootWidget->GetMainStack())
	{
		// --- CORRECTED LOGIC ---
		// Get the currently active widget and deactivate it. The stack will handle the rest.
		if (UCommonActivatableWidget* ActiveWidget = UIRootWidget->GetMainStack()->GetActiveWidget())
		{
			ActiveWidget->DeactivateWidget();
			UE_LOG(LogTemp, Verbose, TEXT("Popping current screen."));
		}
	}
}

void US_UI_Subsystem::RequestModal(const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback)
{
	if (ModalStack)
	{
		ModalStack->QueueModal(Payload, OnDismissedCallback);
	}
	UE_LOG(LogTemp, Verbose, TEXT("Modal requested with message: %s"), *Payload.Message.ToString());
}