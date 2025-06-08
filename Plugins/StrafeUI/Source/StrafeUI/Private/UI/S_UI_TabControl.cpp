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

    // Asynchronously load the tab button class to avoid blocking the game thread.
    FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
    FSoftObjectPath TabButtonPath = Settings->TabButtonClass.ToSoftObjectPath();

    // Use a weak pointer to this widget to prevent use-after-free if the widget is destroyed mid-load.
    TWeakObjectPtr<US_UI_TabControl> WeakThis = this;

    TabButtonClassHandle = StreamableManager.RequestAsyncLoad(TabButtonPath,
        [WeakThis, InTabDefinitions, DefaultTabIndex, TabButtonPath]()
        {
            if (!WeakThis.IsValid())
            {
                return; // The tab control was destroyed.
            }

            US_UI_TabControl* StrongThis = WeakThis.Get();
            UObject* LoadedAsset = TabButtonPath.ResolveObject();
            TSubclassOf<UCommonButtonBase> TabButtonClass = Cast<UClass>(LoadedAsset);

            if (!TabButtonClass)
            {
                UE_LOG(LogTemp, Error, TEXT("TabControl: Failed to asynchronously load TabButtonClass!"));
                return;
            }

            // --- The original logic from InitializeTabs now runs in this callback ---
            StrongThis->TabList->RemoveAllTabs();
            StrongThis->ContentSwitcher->ClearChildren();
            StrongThis->TabDefinitions = InTabDefinitions;
            StrongThis->TabIndexMap.Empty();
            StrongThis->TabIdCounter = 0;

            for (int32 i = 0; i < StrongThis->TabDefinitions.Num(); ++i)
            {
                const FTabDefinition& TabDef = StrongThis->TabDefinitions[i];
                FName TabId = FName(*FString::Printf(TEXT("Tab_%d"), StrongThis->TabIdCounter++));
                StrongThis->TabIndexMap.Add(TabId, i);

                // Now that the content widget class is also a soft pointer, we should load it async too.
                // For this refactor, we assume they were preloaded by the subsystem, but a more robust
                // implementation might load them here on-demand.
                UCommonActivatableWidget* ContentWidget = nullptr;
                if (UClass* WidgetClass = TabDef.ContentWidgetClass.Get()) // Use .Get() assuming it's preloaded
                {
                    ContentWidget = CreateWidget<UCommonActivatableWidget>(StrongThis, WidgetClass);
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("TabControl: Content widget class for tab %s was not preloaded!"), *TabDef.TabName.ToString());
                }

                if (ContentWidget && TabButtonClass)
                {
                    if (!StrongThis->TabList->RegisterTab(TabId, TabButtonClass, ContentWidget))
                    {
                        UE_LOG(LogTemp, Error, TEXT("TabControl: Failed to register tab %s"), *TabDef.TabName.ToString());
                    }
                }
            }

            if (StrongThis->TabDefinitions.IsValidIndex(DefaultTabIndex))
            {
                StrongThis->SelectTabByIndex(DefaultTabIndex);
            }
            else if (StrongThis->TabList->GetTabCount() > 0)
            {
                StrongThis->SelectTabByIndex(0);
            }
            // --- End of original logic ---
        });

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