// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_TabControl.cpp

#include "UI/S_UI_TabControl.h"
#include "UI/S_UI_TabButton.h"
#include "S_UI_Settings.h"
#include "CommonTabListWidgetBase.h"
#include "CommonActivatableWidgetSwitcher.h"
#include "Blueprint/WidgetTree.h"

void US_UI_TabControl::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (TabList)
    {
        TabList->OnTabSelected.AddDynamic(this, &US_UI_TabControl::HandleTabSelected);
    }
}

void US_UI_TabControl::NativeDestruct()
{
    if (TabList)
    {
        TabList->OnTabSelected.RemoveDynamic(this, &US_UI_TabControl::HandleTabSelected);
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

    // Clear existing tabs
    TabList->RemoveAllTabs();
    ContentSwitcher->ClearChildren(); // Use ClearChildren instead of Reset
    TabDefinitions = InTabDefinitions;
    TabIndexMap.Empty();
    TabIdCounter = 0;

    // Get the tab button class from settings
    const US_UI_Settings* Settings = GetDefault<US_UI_Settings>();
    TSubclassOf<UCommonButtonBase> TabButtonClass = nullptr;
    if (Settings && Settings->TabButtonClass.IsValid())
    {
        TabButtonClass = Settings->TabButtonClass.LoadSynchronous();
    }

    if (!TabButtonClass)
    {
        UE_LOG(LogTemp, Error, TEXT("TabControl: No tab button class specified!"));
        return;
    }

    // Create tabs and content
    for (int32 i = 0; i < TabDefinitions.Num(); ++i)
    {
        const FTabDefinition& TabDef = TabDefinitions[i];

        // Generate a unique tab ID
        FName TabId = FName(*FString::Printf(TEXT("Tab_%d"), TabIdCounter++));
        TabIndexMap.Add(TabId, i);

        // Load and add the content widget
        UCommonActivatableWidget* ContentWidget = nullptr;
        if (TabDef.ContentWidgetClass.IsValid())
        {
            UClass* WidgetClass = TabDef.ContentWidgetClass.LoadSynchronous();
            if (WidgetClass)
            {
                ContentWidget = CreateWidget<UCommonActivatableWidget>(this, WidgetClass);
                if (ContentWidget)
                {
                    // CommonActivatableWidgetSwitcher expects activatable widgets
                    ContentSwitcher->AddChild(ContentWidget);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("TabControl: Failed to load widget class for tab %s"),
                    *TabDef.TabName.ToString());
            }
        }

        // Register the tab
        if (ContentWidget)
        {
            // RegisterTab expects a UWidget*, so we can pass our ContentWidget directly
            TabList->RegisterTab(TabId, TabButtonClass, ContentWidget);

            // Set the tab text
            if (US_UI_TabButton* TabButton = Cast<US_UI_TabButton>(TabList->GetTabButtonBaseByID(TabId)))
            {
                TabButton->SetTabLabelText(TabDef.TabName);
            }
        }
    }

    // Select the default tab
    if (TabDefinitions.IsValidIndex(DefaultTabIndex))
    {
        SelectTabByIndex(DefaultTabIndex);
    }

    // After all tabs are registered, log the count
    UE_LOG(LogTemp, Warning, TEXT("TabControl: Registered %d tabs"), TabList->GetTabCount());

    // Also log the visibility state
    UE_LOG(LogTemp, Warning, TEXT("TabList Visibility: %s"),
        *UEnum::GetValueAsString(TabList->GetVisibility()));
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

    // GetActiveWidget returns UWidget*, so we need to cast it
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
        // Switch to the corresponding content
        if (ContentSwitcher)
        {
            ContentSwitcher->SetActiveWidgetIndex(*Index);
        }

        // Fire the event
        FName TabTag = TabDefinitions.IsValidIndex(*Index) ? TabDefinitions[*Index].TabTag : NAME_None;
        OnTabSelected.Broadcast(*Index, TabTag);
    }
}