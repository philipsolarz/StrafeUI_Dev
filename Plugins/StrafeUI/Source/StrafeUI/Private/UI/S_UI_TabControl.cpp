// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_TabControl.cpp

#include "UI/S_UI_TabControl.h"
#include "UI/S_UI_TabButton.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "S_UI_Settings.h"
#include "CommonTabListWidgetBase.h"
#include "CommonActivatableWidgetSwitcher.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"

void US_UI_TabControl::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (TabList && ContentSwitcher)
    {
        TabList->OnTabSelected.AddDynamic(this, &US_UI_TabControl::HandleTabSelected);
        TabList->OnTabButtonCreation.AddDynamic(this, &US_UI_TabControl::HandleTabButtonCreation);

        // Set the linked switcher
        TabList->SetLinkedSwitcher(ContentSwitcher);
    }
}

void US_UI_TabControl::NativeConstruct()
{
    Super::NativeConstruct();

    // Force a layout refresh after construction
    if (TabList)
    {
        TabList->ForceLayoutPrepass();

        // Ensure TabList is visible
        if (TabList->GetVisibility() != ESlateVisibility::Visible)
        {
            UE_LOG(LogTemp, Warning, TEXT("TabControl: Setting TabList to Visible in NativeConstruct"));
            TabList->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void US_UI_TabControl::NativeDestruct()
{
    if (TabList)
    {
        TabList->OnTabSelected.RemoveDynamic(this, &US_UI_TabControl::HandleTabSelected);
        TabList->OnTabButtonCreation.RemoveDynamic(this, &US_UI_TabControl::HandleTabButtonCreation);
    }

    Super::NativeDestruct();
}

void US_UI_TabControl::InitializeTabs(const TArray<FTabDefinition>& InTabDefinitions, int32 DefaultTabIndex)
{
    if (!TabList || !ContentSwitcher)
    {
        UE_LOG(LogTemp, Error, TEXT("TabControl: TabList or ContentSwitcher is null!"));
        return;
    }

    // Get the tab button class from settings
    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
    if (!Settings || Settings->TabButtonClass.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("TabControl: No tab button class specified in settings!"));
        return;
    }

    // Store the definitions and default index for later
    TabDefinitions = InTabDefinitions;
    PendingDefaultTabIndex = DefaultTabIndex;

    // Collect all assets that need to be loaded
    TArray<FSoftObjectPath> AssetsToLoad;

    // Add the tab button class
    AssetsToLoad.Add(Settings->TabButtonClass.ToSoftObjectPath());

    // Add all content widget classes
    for (const FTabDefinition& TabDef : TabDefinitions)
    {
        if (!TabDef.ContentWidgetClass.IsNull())
        {
            AssetsToLoad.Add(TabDef.ContentWidgetClass.ToSoftObjectPath());
        }
    }

    // Load all assets at once
    FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
    TWeakObjectPtr<US_UI_TabControl> WeakThis = this;

    AllAssetsHandle = StreamableManager.RequestAsyncLoad(AssetsToLoad,
        [WeakThis]()
        {
            if (!WeakThis.IsValid())
            {
                return; // The tab control was destroyed.
            }

            US_UI_TabControl* StrongThis = WeakThis.Get();
            StrongThis->OnAllTabAssetsLoaded();
        });
}

void US_UI_TabControl::OnAllTabAssetsLoaded()
{
    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
    if (!Settings)
    {
        UE_LOG(LogTemp, Error, TEXT("TabControl: Settings became invalid during load"));
        return;
    }

    TSubclassOf<UCommonButtonBase> TabButtonClass = Settings->TabButtonClass.Get();
    if (!TabButtonClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TabControl: Failed to load TabButtonClass!"));
        return;
    }

    // Clear existing tabs
    TabList->RemoveAllTabs();
    ContentSwitcher->ClearChildren();
    TabIndexMap.Empty();
    TabIdCounter = 0;

    // Create tabs with loaded content
    for (int32 i = 0; i < TabDefinitions.Num(); ++i)
    {
        const FTabDefinition& TabDef = TabDefinitions[i];
        FName TabId = FName(*FString::Printf(TEXT("Tab_%d"), TabIdCounter++));
        TabIndexMap.Add(TabId, i);

        // Create the content widget
        UCommonActivatableWidget* ContentWidget = nullptr;
        if (UClass* WidgetClass = TabDef.ContentWidgetClass.Get())
        {
            ContentWidget = CreateWidget<UCommonActivatableWidget>(this, WidgetClass);
            if (!ContentWidget)
            {
                UE_LOG(LogTemp, Error, TEXT("TabControl: Failed to create content widget for tab %s"),
                    *TabDef.TabName.ToString());
                continue;
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("TabControl: Content widget class for tab %s was not loaded!"),
                *TabDef.TabName.ToString());
            continue;
        }

        // Register the tab
        if (!TabList->RegisterTab(TabId, TabButtonClass, ContentWidget))
        {
            UE_LOG(LogTemp, Error, TEXT("TabControl: Failed to register tab %s"),
                *TabDef.TabName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("TabControl: Successfully registered tab %s with %d content children"),
                *TabDef.TabName.ToString(), ContentSwitcher->GetNumWidgets());
        }
    }

    // Select the default tab
    if (TabDefinitions.IsValidIndex(PendingDefaultTabIndex))
    {
        SelectTabByIndex(PendingDefaultTabIndex);
    }
    else if (TabList->GetTabCount() > 0)
    {
        SelectTabByIndex(0);
    }

    // Log final state
    UE_LOG(LogTemp, Warning, TEXT("TabControl: Initialization complete. ContentSwitcher has %d children"),
        ContentSwitcher->GetNumWidgets());

    // Force visibility
    if (TabList->GetVisibility() != ESlateVisibility::Visible)
    {
        TabList->SetVisibility(ESlateVisibility::Visible);
    }
}

void US_UI_TabControl::SelectTabByIndex(int32 TabIndex)
{
    if (!TabDefinitions.IsValidIndex(TabIndex))
    {
        return;
    }

    // Find the tab ID for this index
    for (const auto& Pair : TabIndexMap)
    {
        if (Pair.Value == TabIndex)
        {
            TabList->SelectTabByID(Pair.Key);
            break;
        }
    }
}

void US_UI_TabControl::SelectTabByTag(FName TabTag)
{
    for (int32 i = 0; i < TabDefinitions.Num(); ++i)
    {
        if (TabDefinitions[i].TabTag == TabTag)
        {
            SelectTabByIndex(i);
            break;
        }
    }
}

UCommonActivatableWidget* US_UI_TabControl::GetActiveTabContent() const
{
    if (!ContentSwitcher)
    {
        return nullptr;
    }

    UWidget* ActiveWidget = ContentSwitcher->GetActiveWidget();
    return Cast<UCommonActivatableWidget>(ActiveWidget);
}

int32 US_UI_TabControl::GetSelectedTabIndex() const
{
    if (!TabList)
    {
        return -1;
    }

    FName SelectedTabId = TabList->GetSelectedTabId();
    if (const int32* Index = TabIndexMap.Find(SelectedTabId))
    {
        return *Index;
    }

    return -1;
}

void US_UI_TabControl::HandleTabSelected(FName TabId)
{
    if (const int32* Index = TabIndexMap.Find(TabId))
    {
        // Since we've linked the switcher, it should switch automatically
        // But we'll ensure the active widget index is correct
        if (ContentSwitcher && ContentSwitcher->GetActiveWidgetIndex() != *Index)
        {
            ContentSwitcher->SetActiveWidgetIndex(*Index);
        }

        // Fire the event
        FName TabTag = TabDefinitions.IsValidIndex(*Index) ? TabDefinitions[*Index].TabTag : NAME_None;
        OnTabSelected.Broadcast(*Index, TabTag);
    }
}

void US_UI_TabControl::HandleTabButtonCreation(FName TabId, UCommonButtonBase* TabButton)
{
    UE_LOG(LogTemp, Warning, TEXT("TabControl: Tab button created for ID %s"), *TabId.ToString());

    if (TabButton)
    {
        // Force the tab button to be visible
        TabButton->SetVisibility(ESlateVisibility::Visible);

        // Set the tab text if it's our custom tab button
        if (US_UI_TabButton* CustomTabButton = Cast<US_UI_TabButton>(TabButton))
        {
            // Find the tab definition for this ID
            if (const int32* Index = TabIndexMap.Find(TabId))
            {
                if (TabDefinitions.IsValidIndex(*Index))
                {
                    CustomTabButton->SetTabLabelText(TabDefinitions[*Index].TabName);
                }
            }
        }

        // Log button visibility
        UE_LOG(LogTemp, Warning, TEXT("Tab Button Visibility after fix: %s"),
            *UEnum::GetValueAsString(TabButton->GetVisibility()));
    }
}