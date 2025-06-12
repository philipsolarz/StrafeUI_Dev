// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_ServerBrowser.cpp

#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "S_UI_Subsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Data/S_UI_ScreenTypes.h"
#include "Online/OnlineSessionNames.h"
#include "FindSessionsCallbackProxyAdvanced.h"
#include "AdvancedSessionsLibrary.h"

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
#define SETTING_GAMETAG FName(TEXT("GAMETAG"))

US_UI_VM_ServerBrowser::~US_UI_VM_ServerBrowser()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		}
	}
}

void US_UI_VM_ServerBrowser::RequestServerListRefresh()
{
	UE_LOG(LogTemp, Log, TEXT("Refreshing server list using Advanced Sessions..."));

	ServerList.Empty();
	AllFoundServers.Empty();
	BroadcastDataChanged();

	TArray<FSessionsSearchSetting> SearchSettings;
	FSessionPropertyKeyPair GameTagSetting = UAdvancedSessionsLibrary::MakeLiteralSessionPropertyString(SETTING_GAMETAG, FString("StrafeGame"));
	FSessionsSearchSetting GameTagFilter = UAdvancedSessionsLibrary::MakeLiteralSessionSearchProperty(GameTagSetting, EOnlineComparisonOpRedux::Equals);
	SearchSettings.Add(GameTagFilter);

	auto* Proxy = UFindSessionsCallbackProxyAdvanced::FindSessionsAdvanced(
		this,
		GetWorld()->GetFirstPlayerController(),
		100,
		bSearchLAN,
		EBPServerPresenceSearchType::AllServers,
		SearchSettings,
		false,
		false,
		true,
		0
	);

	if (Proxy)
	{
		Proxy->OnSuccess.AddDynamic(this, &US_UI_VM_ServerBrowser::OnFindSessionsSuccess);
		Proxy->OnFailure.AddDynamic(this, &US_UI_VM_ServerBrowser::OnFindSessionsFailure);
		Proxy->Activate();
	}
}

void US_UI_VM_ServerBrowser::OnFindSessionsSuccess(const TArray<FBlueprintSessionResult>& SearchResults)
{
	UE_LOG(LogTemp, Log, TEXT("Advanced session search complete. Found %d sessions"), SearchResults.Num());

	AllFoundServers.Empty();

	for (const FBlueprintSessionResult& Result : SearchResults)
	{
		if (!Result.OnlineResult.IsValid()) continue;

		US_UI_VM_ServerListEntry* NewEntry = NewObject<US_UI_VM_ServerListEntry>(this);

		NewEntry->SessionSearchResult = Result.OnlineResult;

		F_ServerInfo& ServerInfo = NewEntry->ServerInfo;
		const FOnlineSessionSearchResult& SearchResult = Result.OnlineResult;

		ServerInfo.PlayerCount = SearchResult.Session.SessionSettings.NumPublicConnections - SearchResult.Session.NumOpenPublicConnections;
		ServerInfo.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
		ServerInfo.Ping = SearchResult.PingInMs;
		ServerInfo.bIsPrivate = !SearchResult.Session.SessionSettings.bShouldAdvertise;
		ServerInfo.bIsLAN = SearchResult.Session.SessionSettings.bIsLANMatch;

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

	UpdateFilteredServerList();

	if (SearchResults.Num() == 0)
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

void US_UI_VM_ServerBrowser::OnFindSessionsFailure(const TArray<FBlueprintSessionResult>& SearchResults)
{
	UE_LOG(LogTemp, Warning, TEXT("Advanced session search failed."));

	ServerList.Empty();
	AllFoundServers.Empty();
	BroadcastDataChanged();
}

void US_UI_VM_ServerBrowser::JoinSession(const FOnlineSessionSearchResult& SessionSearchResult)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem) return;

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetLocalPlayer()) return;

	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &US_UI_VM_ServerBrowser::OnJoinSessionComplete)
	);

	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
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
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem) return;

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString ConnectString;
		if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
		{
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	else
	{
		FString ErrorMessage;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull: ErrorMessage = TEXT("The session is full."); break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist: ErrorMessage = TEXT("The session no longer exists."); break;
		default: ErrorMessage = TEXT("Failed to join the session. Please try again."); break;
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

	for (const TObjectPtr<US_UI_VM_ServerListEntry>& Entry : AllFoundServers)
	{
		if (PassesFilters(Entry))
		{
			ServerList.Add(Entry->ServerInfo);
		}
	}

	BroadcastDataChanged();
}

bool US_UI_VM_ServerBrowser::PassesFilters(const TObjectPtr<US_UI_VM_ServerListEntry>& Entry) const
{
	if (!Entry) return false;

	const F_ServerInfo& ServerInfo = Entry->ServerInfo;

	if (!FilterServerName.IsEmpty() && !ServerInfo.ServerName.ToString().Contains(FilterServerName, ESearchCase::IgnoreCase))
	{
		return false;
	}

	if (!FilterGameMode.IsEmpty() && !ServerInfo.GameMode.ToString().Contains(FilterGameMode, ESearchCase::IgnoreCase))
	{
		return false;
	}

	if (ServerInfo.Ping > FilterMaxPing) return false;

	if (bFilterHideFullServers && ServerInfo.PlayerCount >= ServerInfo.MaxPlayers) return false;

	if (bFilterHideEmptyServers && ServerInfo.PlayerCount == 0) return false;

	if (bFilterHidePrivateServers && ServerInfo.bIsPrivate) return false;

	return true;
}