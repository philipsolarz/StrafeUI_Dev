// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_TabControl.cpp

#include "UI/S_UI_TabControl.h"
#include "UI/S_UI_TabButton.h"
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

    // Clear existing tabs
    TabList->RemoveAllTabs();
    ContentSwitcher->ClearChildren();
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
                    // Don't add to switcher yet - RegisterTab will handle it
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("TabControl: Failed to load widget class for tab %s"),
                    *TabDef.TabName.ToString());
            }
        }

        // Register the tab using the correct API
        if (ContentWidget && TabButtonClass)
        {
            // RegisterTab(FName TabNameID, TSubclassOf<UCommonButtonBase> ButtonWidgetType, UWidget* ContentWidget, const int32 TabIndex = -1)
            if (!TabList->RegisterTab(TabId, TabButtonClass, ContentWidget))
            {
                UE_LOG(LogTemp, Error, TEXT("TabControl: Failed to register tab %s"), *TabDef.TabName.ToString());
            }
            else
            {
                // RegisterTab should add the content widget to the linked switcher automatically
                // But let's verify it was added
                if (!ContentSwitcher->HasChild(ContentWidget))
                {
                    UE_LOG(LogTemp, Warning, TEXT("TabControl: Content widget was not added to switcher by RegisterTab, adding manually"));
                    ContentSwitcher->AddChild(ContentWidget);
                }
            }
        }
    }

    // Select the default tab
    if (TabDefinitions.IsValidIndex(DefaultTabIndex))
    {
        SelectTabByIndex(DefaultTabIndex);
    }
    else if (TabList->GetTabCount() > 0)
    {
        // If no valid default index, select the first tab
        SelectTabByIndex(0);
    }

    // Log final state
    UE_LOG(LogTemp, Warning, TEXT("TabControl: Initialized with %d tabs, TabList has %d registered tabs"),
        TabDefinitions.Num(), TabList->GetTabCount());

    // Log visibility for debugging
    UE_LOG(LogTemp, Warning, TEXT("TabList Visibility: %s"),
        *UEnum::GetValueAsString(TabList->GetVisibility()));

    // Force TabList to be visible if it's not
    if (TabList->GetVisibility() != ESlateVisibility::Visible &&
        TabList->GetVisibility() != ESlateVisibility::HitTestInvisible)
    {
        UE_LOG(LogTemp, Warning, TEXT("TabControl: Forcing TabList to be Visible"));
        TabList->SetVisibility(ESlateVisibility::Visible);
    }

    // Log content switcher state
    UE_LOG(LogTemp, Warning, TEXT("TabControl: ContentSwitcher has %d children"),
        ContentSwitcher->GetNumWidgets());
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