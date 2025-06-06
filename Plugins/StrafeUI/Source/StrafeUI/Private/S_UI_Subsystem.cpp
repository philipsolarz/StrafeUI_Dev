// Plugins/StrafeUI/Source/StrafeUI/Private/S_UI_Subsystem.cpp

#include "S_UI_Subsystem.h"
#include "UObject/PrimaryAssetId.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

// Placeholder includes for forward-declared classes.
// You will need to create and include the actual headers for these.
// #include "S_UI_InputController.h"
// #include "S_UI_ModalStack.h"
// #include "S_UI_RootWidget.h"
// #include "Data/S_UI_ScreenDataAsset.h"

void US_UI_Subsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialization logic will go here, such as:
	// - Creating the root widget
	// - Loading the screen data asset
	// - Initializing the input controller and modal stack
	UE_LOG(LogTemp, Log, TEXT("S_UI_Subsystem Initialized"));
}

void US_UI_Subsystem::Deinitialize()
{
	// Cleanup logic will go here, such as:
	// - Tearing down the root widget
	// - Clearing caches
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

	// Implementation will involve:
	// 1. Check if the widget class is already cached in ScreenWidgetClassCache.
	// 2. If not, asynchronously load it from the ScreenMapDataAsset.
	// 3. Once loaded, use the UIRootWidget to push the screen widget onto its managed stack.
	UE_LOG(LogTemp, Verbose, TEXT("Pushing screen: %s"), *UEnum::GetValueAsString(ScreenId));
}

void US_UI_Subsystem::PopScreen()
{
	// Implementation will call the corresponding Pop function on the UIRootWidget.
	UE_LOG(LogTemp, Verbose, TEXT("Popping current screen."));
}

void US_UI_Subsystem::RequestModal(const F_UIModalPayload& Payload, const FOnUIModalDismissed& OnDismissedCallback)
{
	// Implementation will delegate the request to the US_UI_ModalStack.
	// The ModalStack will then be responsible for creating and displaying the modal.
	UE_LOG(LogTemp, Verbose, TEXT("Modal requested with message: %s"), *Payload.Message.ToString());
}