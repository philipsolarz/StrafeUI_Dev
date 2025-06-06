// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_BaseScreenWidget.h

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "S_UI_BaseScreenWidget.generated.h"

class US_UI_Subsystem;

/**
 * @class S_UI_BaseScreenWidget
 * @brief A base class for all primary UI screens in the game.
 * Provides common functionality, such as easy access to the UI Subsystem.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_BaseScreenWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    /**
     * @brief Gets the UI Subsystem.
     * @return A pointer to the US_UI_Subsystem.
     */
    UFUNCTION(BlueprintCallable, Category = "UI")
    US_UI_Subsystem* GetUISubsystem() const;
};