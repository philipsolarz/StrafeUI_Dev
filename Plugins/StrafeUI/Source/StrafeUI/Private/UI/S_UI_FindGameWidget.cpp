// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_FindGameWidget.cpp

#include "UI/S_UI_FindGameWidget.h"
#include "Components/ListView.h"
#include "CommonButtonBase.h"
#include "S_UI_Subsystem.h"
#include "S_UI_Navigator.h" // <<< Added include
// No need to include the VM header here as it's already in the widget's .h file

US_UI_ViewModelBase* US_UI_FindGameWidget::CreateViewModel()
{
    return NewObject<US_UI_VM_ServerBrowser>(this);
}

void US_UI_FindGameWidget::SetViewModel(US_UI_ViewModelBase* InViewModel)
{
    if (US_UI_VM_ServerBrowser* InServerBrowserViewModel = Cast<US_UI_VM_ServerBrowser>(InViewModel))
    {
        if (InServerBrowserViewModel)
        {
            ViewModel = InServerBrowserViewModel;

            // Bind to the ViewModel's OnDataChanged delegate to be notified of updates.
            ViewModel->OnDataChanged.AddUniqueDynamic(this, &US_UI_FindGameWidget::OnServerListUpdated);

            // Trigger an initial data refresh.
            OnServerListUpdated();
        }
    }
}

void US_UI_FindGameWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (Btn_Refresh && ViewModel.IsValid())
    {
        // Pure MVVM approach: Bind button click directly to the ViewModel's function.
        Btn_Refresh->OnClicked().AddUObject(ViewModel.Get(), &US_UI_VM_ServerBrowser::RequestServerListRefresh);
    }
    if (Btn_Join)
    {
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
            // Create a UObject wrapper for our list entry data
            US_UI_VM_ServerListEntry* Entry = NewObject<US_UI_VM_ServerListEntry>(this);
            Entry->ServerInfo = ServerInfo;

            // Add the data object to the list view. The list view will create a widget for it.
            List_Servers->AddItem(Entry);
        }
    }
}

void US_UI_FindGameWidget::HandleJoinClicked()
{
    // Join logic would go here.
    if (const US_UI_VM_ServerListEntry* SelectedItem = List_Servers ? List_Servers->GetSelectedItem<US_UI_VM_ServerListEntry>() : nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("Attempting to join selected server: %s"), *SelectedItem->ServerInfo.ServerName.ToString());
        // In a real game, you would use this data to initiate a connection.
        // For example: GetWorld()->GetGameInstance()->JoinSession(..., SelectedItem->ServerInfo.SessionData);
    }
}

void US_UI_FindGameWidget::HandleBackClicked()
{
    if (US_UI_Subsystem* UISubsystem = GetUISubsystem())
    {
        // <<< Corrected call to use the Navigator
        UISubsystem->GetNavigator()->PopContentScreen();
    }
}