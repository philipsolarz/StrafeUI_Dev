// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_Subsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/S_UI_ScreenTypes.h"
#include "S_UI_Subsystem.generated.h"

// Forward declarations to reduce header dependencies
class UCommonActivatableWidget;
class US_UI_InputController;
class US_UI_ModalStack;
class US_UI_RootWidget;
class AS_UI_PlayerController;
struct FStreamableHandle;

/**
 * @class US_UI_Subsystem
 * @brief The central orchestrator for all UI operations within the StrafeUI plugin.
 *
 * This subsystem acts as the single point of entry for managing UI screens, modals,
 * and input routing. It is responsible for loading UI assets, instantiating widgets,
 * and maintaining the overall UI state.
 */
UCLASS() // No longer Blueprintable
class STRAFEUI_API US_UI_Subsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	/** Initializes the root widget and input bindings for a specific player. */
	void InitializeUIForPlayer(AS_UI_PlayerController* PlayerController);

	/**
	 * Switches the active content screen. This clears any previous content screen.
	 * @param ScreenId The unique identifier of the content screen to display.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Subsystem")
	void SwitchContentScreen(const E_UIScreenId ScreenId);

	/**
	 * Pops the current screen from the content stack, if any are stacked.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI Subsystem")
	void PopContentScreen();

	/**
	 * Requests to display a modal dialog with a specified payload and callback.
	 * @param Payload The data defining the modal's content and button layout.
	 * @param OnDismissedCallback The delegate to execute when the modal is dismissed.
	 */
	void RequestModal(const F_UIModalPayload& Payload, const FOnModalDismissedSignature& OnDismissedCallback);

	/** Returns the root UI widget. */
	UFUNCTION(BlueprintPure, Category = "UI Subsystem")
	US_UI_RootWidget* GetRootWidget() const { return UIRootWidget; }

private:
	/**
	 * A cache of screen widget classes, populated from the ScreenMapDataAsset.
	 * This avoids repeatedly resolving the soft pointer once a class is loaded.
	 */
	UPROPERTY()
	TMap<E_UIScreenId, TSubclassOf<UCommonActivatableWidget>> ScreenWidgetClassCache;

	/** Pointer to the UI input controller, responsible for managing UI-specific input actions. */
	UPROPERTY()
	TObjectPtr<US_UI_InputController> InputController;

	/** Pointer to the modal stack, which manages the lifecycle of modal dialogs. */
	UPROPERTY()
	TObjectPtr<US_UI_ModalStack> ModalStack;

	/** The root widget for the UI, which contains the main layout and screen host. */
	UPROPERTY()
	TObjectPtr<US_UI_RootWidget> UIRootWidget;

	bool bAssetsLoaded = false;

	/** Flag to prevent starting the load multiple times. */
	bool bAreAssetsLoading = false;

	/** If a screen switch is requested before assets are loaded, it's stored here. */
	E_UIScreenId PendingScreenRequest = E_UIScreenId::None;

	/** The player controller that initiated the UI creation, cached for use in async callbacks. */
	UPROPERTY()
	TWeakObjectPtr<AS_UI_PlayerController> InitializingPlayer;

	/** Handle for managing the asynchronous loading of all core UI assets. */
	TSharedPtr<FStreamableHandle> AllAssetsHandle;

	/**
	 * Starts the asynchronous asset loading process.
	 */
	void StartAssetsLoading();

	/**
	 * Callback function that is executed after the core DataAsset is loaded.
	 * It then queues up the loading of all widgets defined in the DataAsset.
	 * @param ScreenDataAssetPath The path of the loaded Screen Map Data Asset.
	 */
	void OnScreenMapDataAssetLoaded(FSoftObjectPath ScreenDataAssetPath);

	/**
	 * Final callback executed after all UI assets (screens, modals, etc.) are loaded.
	 * This function finalizes the UI setup.
	 */
	void OnAllAssetsLoaded();
};