// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_TabButton.h

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "S_UI_TabButton.generated.h"

/**
 * A simple tab button for use with the tab control system.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_TabButton : public UCommonButtonBase
{
    GENERATED_BODY()

public:
    /** Sets the tab's display text. */
    UFUNCTION(BlueprintCallable, Category = "Tab Button")
    void SetTabLabelText(const FText& InText);

protected:
    /** The text block that displays the tab label. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    class UCommonTextBlock* TabLabel;
};