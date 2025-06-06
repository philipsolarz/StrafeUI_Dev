// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_RootWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Components/Border.h"
#include "S_UI_RootWidget.generated.h"

/**
 * @class S_UI_RootWidget
 * @brief The C++ base for WBP_UIRoot, the main container for the game's UI.
 * Manages the primary screen stack and global overlays.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_RootWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    /** Returns the main widget stack where screens are pushed. */
    UCommonActivatableWidgetStack* GetMainStack() const { return Slot_MainStack; }

private:
    /** The main stack where all primary screens (e.g., Main Menu, Settings) are pushed. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetStack> Slot_MainStack;

    /** An overlay that can be used for global UI effects like fade-in/fade-out transitions. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UBorder> Overlay_Transition;
};