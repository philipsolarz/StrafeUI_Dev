// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_ServerBrowser.cpp

#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "S_UI_Subsystem.h"
#include "OnlineSubsystem.h"

#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Data/S_UI_ScreenTypes.h"
#include "Online/OnlineSessionNames.h"

// Match the custom session settings keys from CreateGame
#define SETTING_GAMEMODE FName(TEXT("GAMEMODE"))
#define SETTING_MAPNAME FName(TEXT("MAPNAME"))
#define SETTING_GAMENAME FName(TEXT("GAMENAME"))
#define SETTING_SERVERDESC FName(TEXT("SERVERDESC"))
#define SETTING_FRIENDLYFIRE FName(TEXT("FRIENDLYFIRE"))
#define SETTING_SPECTATORS FName(TEXT("SPECTATORS"))
#define SETTING_TIMELIMIT FName(TEXT("TIMELIMIT"))
#define SETTING_SCORELIMIT FName(TEXT("SCORELIMIT"))
#define SETTING_RESPAWNTIME FName(TEXT("RESPAWNTIME"))
// *** FIX: Add a unique tag to filter sessions by, preventing other games on Steam App ID 480 from showing up ***
#define SETTING_GAMETAG FName(TEXT("GAMETAG"))

US_UI_VM_ServerBrowser::~US_UI_VM_ServerBrowser()
{
	// Clean up any pending delegates
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		}
	}
}

void US_UI_VM_ServerBrowser::RequestServerListRefresh()
{
	UE_LOG(LogTemp, Log, TEXT("Refreshing server list..."));

	// Clear existing lists
	ServerList.Empty();
	AllFoundServers.Empty();
	BroadcastDataChanged();

	// Get the Online Subsystem
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("No online subsystem found"));

		// Show error modal
		if (UWorld* World = GetWorld())
		{
			if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
			{
				F_UIModalPayload Payload;
				Payload.Message = FText::FromString(TEXT("Online services are not available. Please check your connection."));
				Payload.ModalType = E_UIModalType::OK;
				UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
			}
		}
		return;
	}

	// Get the Session Interface
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Session interface is invalid"));
		return;
	}

	// Create the search object
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create session search object"));
		return;
	}

	// Configure the search
	SessionSearch->bIsLanQuery = bSearchLAN;
	SessionSearch->MaxSearchResults = 10000;
	//SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	// *** FIX: Add a query filter for our unique game tag ***
	//SessionSearch->QuerySettings.Set(SETTING_GAMETAG, FString("StrafeGame"), EOnlineComparisonOp::Equals);


	// Get the local player
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No world context"));
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetLocalPlayer())
	{
		UE_LOG(LogTemp, Error, TEXT("No local player controller"));
		return;
	}

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();

	// Bind the completion delegate
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &US_UI_VM_ServerBrowser::OnFindSessionsComplete)
	);

	// Start the search
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to start session search"));

		// Clean up the delegate since we won't get a callback
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		// Show error modal
		if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
		{
			F_UIModalPayload Payload;
			Payload.Message = FText::FromString(TEXT("Failed to search for game sessions. Please try again."));
			Payload.ModalType = E_UIModalType::OK;
			UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
		}
	}
}

void US_UI_VM_ServerBrowser::OnFindSessionsComplete(bool bWasSuccessful)
{
	// Clean up the delegate
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		}
	}

	// Clear the lists
	AllFoundServers.Empty();

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("Session search complete. Found %d sessions"), SessionSearch->SearchResults.Num());

		// Process each found session
		for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			US_UI_VM_ServerListEntry* NewEntry = NewObject<US_UI_VM_ServerListEntry>(this);

			// Store the full search result for joining later
			NewEntry->SessionSearchResult = SearchResult;

			// Extract basic info
			F_ServerInfo& ServerInfo = NewEntry->ServerInfo;

			// Get player counts
			ServerInfo.PlayerCount = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
			ServerInfo.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;

			// Get ping
			ServerInfo.Ping = SearchResult.PingInMs;

			// Get basic settings
			ServerInfo.bIsPrivate = !SearchResult.Session.SessionSettings.bShouldAdvertise;
			ServerInfo.bIsLAN = SearchResult.Session.SessionSettings.bIsLANMatch;

			// Get custom session data
			FString GameName;
			if (SearchResult.Session.SessionSettings.Get(SETTING_GAMENAME, GameName))
			{
				ServerInfo.ServerName = FText::FromString(GameName);
			}
			else
			{
				ServerInfo.ServerName = FText::FromString(TEXT("Unknown Server"));
			}

			FString GameMode;
			if (SearchResult.Session.SessionSettings.Get(SETTING_GAMEMODE, GameMode))
			{
				ServerInfo.GameMode = FText::FromString(GameMode);
			}

			FString MapName;
			if (SearchResult.Session.SessionSettings.Get(SETTING_MAPNAME, MapName))
			{
				ServerInfo.CurrentMap = MapName;
			}

			FString Description;
			if (SearchResult.Session.SessionSettings.Get(SETTING_SERVERDESC, Description))
			{
				ServerInfo.Description = FText::FromString(Description);
			}

			AllFoundServers.Add(NewEntry);
		}

		// Apply filters to show results
		UpdateFilteredServerList();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Session search failed or returned no results"));

		// Clear the displayed list
		ServerList.Empty();
		BroadcastDataChanged();

		// Show modal if no servers found
		if (bWasSuccessful && SessionSearch.IsValid() && SessionSearch->SearchResults.Num() == 0)
		{
			if (UWorld* World = GetWorld())
			{
				if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
				{
					F_UIModalPayload Payload;
					Payload.Message = FText::FromString(TEXT("No game sessions found. Try creating your own!"));
					Payload.ModalType = E_UIModalType::OK;
					UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
				}
			}
		}
	}
}

void US_UI_VM_ServerBrowser::JoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	// Get the Online Subsystem
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("No online subsystem found"));
		return;
	}

	// Get the Session Interface
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Session interface is invalid"));
		return;
	}

	// Get the local player
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No world context"));
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetLocalPlayer())
	{
		UE_LOG(LogTemp, Error, TEXT("No local player controller"));
		return;
	}

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();

	// Bind the completion delegate
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &US_UI_VM_ServerBrowser::OnJoinSessionComplete)
	);

	// Attempt to join the session
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSearchResult))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to join session"));

		// Clean up the delegate since we won't get a callback
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		// Show error modal
		if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
		{
			F_UIModalPayload Payload;
			Payload.Message = FText::FromString(TEXT("Failed to join the selected game session."));
			Payload.ModalType = E_UIModalType::OK;
			UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
		}
	}
}

void US_UI_VM_ServerBrowser::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// Get the session interface
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	// Clean up the delegate
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	// Check if join was successful
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully joined session"));

		// Get the connect string
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			UE_LOG(LogTemp, Log, TEXT("Traveling to server: %s"), *ConnectString);

			// Travel to the server
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get connection string"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to join session. Result: %d"), (int32)Result);

		// Show error modal
		FString ErrorMessage;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ErrorMessage = TEXT("The session is full.");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ErrorMessage = TEXT("The session no longer exists.");
			break;
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			ErrorMessage = TEXT("You are already in a session.");
			break;
		default:
			ErrorMessage = TEXT("Failed to join the session. Please try again.");
			break;
		}

		if (UWorld* World = GetWorld())
		{
			if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
			{
				F_UIModalPayload Payload;
				Payload.Message = FText::FromString(ErrorMessage);
				Payload.ModalType = E_UIModalType::OK;
				UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
			}
		}
	}
}

void US_UI_VM_ServerBrowser::ApplyFilters()
{
	UpdateFilteredServerList();
}

void US_UI_VM_ServerBrowser::UpdateFilteredServerList()
{
	ServerList.Empty();

	// Apply filters to the full list
	for (const TObjectPtr<US_UI_VM_ServerListEntry>& Entry : AllFoundServers)
	{
		if (PassesFilters(Entry))
		{
			ServerList.Add(Entry->ServerInfo);
		}
	}

	// Notify UI of changes
	BroadcastDataChanged();
}

bool US_UI_VM_ServerBrowser::PassesFilters(const TObjectPtr<US_UI_VM_ServerListEntry>& Entry) const
{
	if (!Entry)
	{
		return false;
	}

	const F_ServerInfo& ServerInfo = Entry->ServerInfo;

	// Filter by server name
	if (!FilterServerName.IsEmpty())
	{
		if (!ServerInfo.ServerName.ToString().Contains(FilterServerName, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	// Filter by game mode
	if (!FilterGameMode.IsEmpty())
	{
		if (!ServerInfo.GameMode.ToString().Contains(FilterGameMode, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	// Filter by ping
	if (ServerInfo.Ping > FilterMaxPing)
	{
		return false;
	}

	// Filter full servers
	if (bFilterHideFullServers && ServerInfo.PlayerCount >= ServerInfo.MaxPlayers)
	{
		return false;
	}

	// Filter empty servers
	if (bFilterHideEmptyServers && ServerInfo.PlayerCount == 0)
	{
		return false;
	}

	// Filter private servers
	if (bFilterHidePrivateServers && ServerInfo.bIsPrivate)
	{
		return false;
	}

	return true;
}