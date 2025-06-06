// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_ServerBrowser.cpp

#include "ViewModel/S_UI_VM_ServerBrowser.h"

void US_UI_VM_ServerBrowser::RequestServerListRefresh()
{
	// In a real implementation, this would trigger an asynchronous query
	// to an online subsystem (e.g., Steam, EOS) to get the server list.
	// For this example, we'll populate it with dummy data to demonstrate.

	UE_LOG(LogTemp, Log, TEXT("Refreshing server list..."));

	// Clear existing list
	ServerList.Empty();

	// Add dummy data
	ServerList.Add({ FText::FromString(TEXT("Alpha Server")), 8, 16, 50 });
	ServerList.Add({ FText::FromString(TEXT("Bravo Server")), 12, 16, 75 });
	ServerList.Add({ FText::FromString(TEXT("Charlie Server")), 4, 8, 30 });

	// Notify any listening UI widgets that the ServerList has been updated.
	BroadcastDataChanged();
}