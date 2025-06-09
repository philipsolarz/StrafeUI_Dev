// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_TabControl.h

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CommonTabListWidgetBase.h"
#include "Engine/StreamableManager.h"
#include "CommonActivatableWidgetSwitcher.h"
#include "S_UI_TabControl.generated.h"

/**
 * Data structure for defining a tab and its associated content.
 */
USTRUCT(BlueprintType)
struct FTabDefinition
{
    GENERATED_BODY()

    /** The display name for this tab. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText TabName;

    /** The widget class to display when this tab is selected. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftClassPtr<UCommonActivatableWidget> ContentWidgetClass;

    /** Optional tag to identify this tab programmatically. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName TabTag;

    FTabDefinition()
    {
        TabName = FText::GetEmpty();
        TabTag = NAME_None;
    }
};

/**
 * A reusable tab control widget that manages a tab list and content switcher.
 * This widget can be used anywhere tabs are needed in the UI.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_TabControl : public UCommonUserWidget
{
    GENERATED_BODY()

public:
    /**
     * Initializes the tab control with a list of tab definitions.
     * @param InTabDefinitions Array of tabs to create
     * @param DefaultTabIndex The index of the tab to select by default
     */
    UFUNCTION(BlueprintCallable, Category = "Tab Control")
    void InitializeTabs(const TArray<FTabDefinition>& InTabDefinitions, int32 DefaultTabIndex = 0);

    /**
     * Selects a tab by its index.
     * @param TabIndex The index of the tab to select
     */
    UFUNCTION(BlueprintCallable, Category = "Tab Control")
    void SelectTabByIndex(int32 TabIndex);

    /**
     * Selects a tab by its tag.
     * @param TabTag The tag of the tab to select
     */
    UFUNCTION(BlueprintCallable, Category = "Tab Control")
    void SelectTabByTag(FName TabTag);

    /**
     * Gets the currently active tab's content widget.
     * @return The active content widget, or nullptr if none
     */
    UFUNCTION(BlueprintCallable, Category = "Tab Control")
    UCommonActivatableWidget* GetActiveTabContent() const;

    /**
     * Gets the currently selected tab index.
     * @return The index of the selected tab, or -1 if none
     */
    UFUNCTION(BlueprintCallable, Category = "Tab Control")
    int32 GetSelectedTabIndex() const;

    /**
     * Gets the content switcher widget (for advanced usage).
     * @return The content switcher widget
     */
    UCommonActivatableWidgetSwitcher* GetContentSwitcher() const { return ContentSwitcher; }

    /** Event fired when a tab is selected. */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabSelected, int32, TabIndex, FName, TabTag);
    UPROPERTY(BlueprintAssignable, Category = "Tab Control")
    FOnTabSelected OnTabSelected;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** The tab list widget that displays the tab buttons. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonTabListWidgetBase> TabList;

    /** The widget switcher that displays tab content. */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetSwitcher> ContentSwitcher;

private:
    /** Called when a tab is selected in the tab list. */
    UFUNCTION()
    void HandleTabSelected(FName TabId);

    /** Called when a tab button is created. */
    UFUNCTION()
    void HandleTabButtonCreation(FName TabId, UCommonButtonBase* TabButton);

    /** Called when all tab assets have been loaded. */
    void OnAllTabAssetsLoaded();

    /** Cached tab definitions. */
    UPROPERTY()
    TArray<FTabDefinition> TabDefinitions;

    /** Map of tab IDs to their indices. */
    TMap<FName, int32> TabIndexMap;

    /** Counter for generating unique tab IDs. */
    int32 TabIdCounter = 0;

    /** Handle for the asynchronous loading of all tab assets. */
    TSharedPtr<FStreamableHandle> AllAssetsHandle;

    /** The pending default tab index to select after loading. */
    int32 PendingDefaultTabIndex = 0;
};