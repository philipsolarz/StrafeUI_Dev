// Plugins/StrafeUI/Source/StrafeUI/Public/S_UI_Settings.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "S_UI_InputController.h"
#include "S_UI_ModalStack.h"
#include "Data/S_UI_ScreenDataAsset.h"
#include "UI/S_UI_RootWidget.h"
#include "UI/S_UI_ModalWidget.h"
#include "InputAction.h" // Required for UInputAction
#include "S_UI_Settings.generated.h"

/**
 * Defines all configurable settings for the StrafeUI plugin.
 * This allows for easy configuration via the Project Settings window,
 * removing the need for Blueprint subclasses of core systems.
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Strafe UI"))
class STRAFEUI_API US_UI_Settings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** The widget class to use for the root of the UI. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
	TSoftClassPtr<US_UI_RootWidget> RootWidgetClass;

	/** Data asset containing the mapping of screen IDs to widget classes. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
	TSoftObjectPtr<US_UI_ScreenDataAsset> ScreenMapDataAsset;

	/** The widget class to use for creating modals. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
	TSoftClassPtr<US_UI_ModalWidget> ModalWidgetClass;

	/** The class to use for the UI Input Controller. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
	TSoftClassPtr<US_UI_InputController> InputControllerClass;

	/** The class to use for the UI Modal Stack. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Core")
	TSoftClassPtr<US_UI_ModalStack> ModalStackClass;

	/** Input Action for UI Navigation (e.g., Gamepad D-pad, WASD). */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> NavigateAction;

	/** Input Action for confirming a selection. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> AcceptAction;

	/** Input Action for going back or canceling. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Input")
	TSoftObjectPtr<UInputAction> BackAction;
};