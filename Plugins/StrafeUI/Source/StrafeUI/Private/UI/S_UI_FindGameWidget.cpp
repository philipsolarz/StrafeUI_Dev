// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_FindGameWidget.cpp

#include "UI/S_UI_FindGameWidget.h"
#include "Components/ListView.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"

void US_UI_FindGameWidget::SetViewModel(US_UI_VM_ServerBrowser* InViewModel)
{
    if (InViewModel)
    {
        ViewModel = InViewModel;

        // Bind to the ViewModel's OnDataChanged delegate to be notified of updates.
        ViewModel->OnDataChanged.AddUniqueDynamic(this, &US_UI_FindGameWidget::OnServerListUpdated);

        // Trigger an initial data refresh.
        OnServerListUpdated();
    }
}

void US_UI_FindGameWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Refresh)
    {
        Btn_Refresh->OnClicked().AddUObject(this, &US_UI_FindGameWidget::HandleRefreshClicked);
    }
    if (Btn_Join)
    {
        // TODO: Implement join logic, likely using List_Servers->GetSelectedItem()
        Btn_Join->OnClicked().AddUObject(this, &US_UI_FindGameWidget::HandleJoinClicked);
    }
    if (Btn_Back)
    {
        Btn_Back->OnClicked().AddUObject(this, &US_UI_FindGameWidget::HandleBackClicked);
    }
}

void US_UI_FindGameWidget::OnServerListUpdated()
{
    if (ViewModel.IsValid() && List_Servers)
    {
        List_Servers->ClearListItems();

        // Populate the list view with data from the ViewModel.
        for (const F_ServerInfo& ServerInfo : ViewModel->ServerList)
        {
            // You would typically use a dedicated UObject for list view items.
            // For simplicity, we are just logging here.
            // Example: UMyServerEntryObject* Entry = NewObject<UMyServerEntryObject>();
            // Entry->ServerInfo = ServerInfo;
            // List_Servers->AddItem(Entry);
            UE_LOG(LogTemp, Log, TEXT("Displaying server: %s"), *ServerInfo.ServerName.ToString());
        }
    }
}

void US_UI_FindGameWidget::HandleRefreshClicked()
{
    if (ViewModel.IsValid())
    {
        ViewModel->RequestServerListRefresh();
    }
}

void US_UI_FindGameWidget::HandleJoinClicked()
{
    // Join logic would go here.
    UObject* SelectedItem = List_Servers ? List_Servers->GetSelectedItem<UObject>() : nullptr;
    if (SelectedItem)
    {
        UE_LOG(LogTemp, Log, TEXT("Attempting to join selected server..."));
    }
}

void US_UI_FindGameWidget::HandleBackClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        UISubsystem->PopScreen();
    }
}