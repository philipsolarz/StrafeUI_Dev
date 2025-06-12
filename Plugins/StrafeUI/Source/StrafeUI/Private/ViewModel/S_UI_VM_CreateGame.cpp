// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_CreateGame.cpp

#include "ViewModel/S_UI_VM_CreateGame.h"
#include "S_UI_Settings.h"
#include "S_UI_Subsystem.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/World.h"
#include "Data/S_UI_ScreenTypes.h"
#include "AdvancedSessionsLibrary.h"
#include "CreateSessionCallbackProxyAdvanced.h"
// No longer including DestroySessionCallbackProxy.h

// Define custom session settings keys
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

void US_UI_VM_CreateGame::Initialize(const US_UI_Settings* InSettings)
{
	UISettings = InSettings;
	if (!UISettings.IsValid())
	{
		return;
	}

	for (const FStrafeGameModeInfo& GameModeInfo : UISettings->AvailableGameModes)
	{
		GameModeDisplayNames.Add(GameModeInfo.DisplayName.ToString());
	}

	if (GameModeDisplayNames.Num() > 0)
	{
		OnGameModeChanged(GameModeDisplayNames[0]);
	}
	else
	{
		BroadcastDataChanged();
	}
}

void US_UI_VM_CreateGame::CreateGame()
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: No online subsystem found"));
		return;
	}

	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Session interface is invalid"));
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogTemp, Log, TEXT("Found an existing session. Destroying it before creating a new one."));

		// Call the interface directly instead of using the proxy
		DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &US_UI_VM_CreateGame::OnDestroySessionComplete));
		if (!SessionInterface->DestroySession(NAME_GameSession))
		{
			// If the call fails, clear the delegate and proceed as if it succeeded to avoid getting stuck.
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
			OnDestroySessionComplete(NAME_GameSession, false);
		}
	}
	else
	{
		CreateNewSession();
	}
}

void US_UI_VM_CreateGame::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Destroy session completed (Success: %s). Proceeding with creation."), bWasSuccessful ? TEXT("true") : TEXT("false"));
	CreateNewSession();
}

void US_UI_VM_CreateGame::CreateNewSession()
{
	if (!UISettings.IsValid()) return;

	const FStrafeGameModeInfo* SelectedGameModeInfo = UISettings->AvailableGameModes.FindByPredicate(
		[this](const FStrafeGameModeInfo& Info)
		{
			return Info.DisplayName.ToString() == SelectedGameModeName;
		});

	if (!SelectedGameModeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Could not find settings for selected game mode '%s'"), *SelectedGameModeName);
		return;
	}

	const TSoftObjectPtr<UWorld>* SelectedMapAsset = SelectedGameModeInfo->CompatibleMaps.FindByPredicate(
		[this](const TSoftObjectPtr<UWorld>& MapAsset)
		{
			return FPaths::GetBaseFilename(MapAsset.ToString()) == SelectedMapName;
		});

	if (!SelectedMapAsset || SelectedMapAsset->IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Could not find asset for selected map '%s'"), *SelectedMapName);
		return;
	}

	CachedGameModeClass = SelectedGameModeInfo->GameModeClass.LoadSynchronous();
	if (!CachedGameModeClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Could not load GameModeClass asset"));
		return;
	}

	CachedMapAssetPath = (*SelectedMapAsset).ToSoftObjectPath().GetLongPackageName();

	TArray<FSessionPropertyKeyPair> ExtraSettings;
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyString(SETTING_MAPNAME, SelectedMapName));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyString(SETTING_GAMENAME, GameName));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyString(SETTING_GAMEMODE, SelectedGameModeName));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyString(SETTING_SERVERDESC, ServerDescription));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyBool(SETTING_FRIENDLYFIRE, bAllowFriendlyFire));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyBool(SETTING_SPECTATORS, bAllowSpectators));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyInt(SETTING_TIMELIMIT, TimeLimit));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyInt(SETTING_SCORELIMIT, ScoreLimit));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyFloat(SETTING_RESPAWNTIME, RespawnTime));
	ExtraSettings.Add(UAdvancedSessionsLibrary::MakeLiteralSessionPropertyString(SETTING_GAMETAG, FString("StrafeGame")));

	UCreateSessionCallbackProxyAdvanced* Proxy = UCreateSessionCallbackProxyAdvanced::CreateAdvancedSession(
		this,
		ExtraSettings,
		GetWorld()->GetFirstPlayerController(),
		MaxPlayers,
		0,
		bIsLANMatch,
		true,
		bIsDedicatedServer,
		true,
		true,
		false,
		false,
		false,
		true,
		true,
		true
	);

	Proxy->OnSuccess.AddDynamic(this, &US_UI_VM_CreateGame::OnCreateSessionSuccess);
	Proxy->OnFailure.AddDynamic(this, &US_UI_VM_CreateGame::OnCreateSessionFailure);
	Proxy->Activate();

	UE_LOG(LogTemp, Log, TEXT("Attempting to create session via Advanced Sessions proxy..."));
}

void US_UI_VM_CreateGame::OnCreateSessionSuccess()
{
	UE_LOG(LogTemp, Log, TEXT("Session created and started successfully via proxy. Traveling to map..."));

	UWorld* World = GetWorld();
	if (World && CachedGameModeClass)
	{
		FString TravelURL = FString::Printf(TEXT("%s?listen?game=%s"), *CachedMapAssetPath, *CachedGameModeClass->GetPathName());
		World->ServerTravel(TravelURL);
	}
}

void US_UI_VM_CreateGame::OnCreateSessionFailure()
{
	UE_LOG(LogTemp, Error, TEXT("Failed to create session via Advanced Sessions proxy."));

	if (UWorld* World = GetWorld())
	{
		if (US_UI_Subsystem* UISubsystem = World->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
		{
			F_UIModalPayload Payload;
			Payload.Message = FText::FromString(TEXT("Failed to create game session. Please check your connection and try again."));
			Payload.ModalType = E_UIModalType::OK;
			UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
		}
	}
}

void US_UI_VM_CreateGame::OnGameModeChanged(FString InSelectedGameModeName)
{
	SelectedGameModeName = InSelectedGameModeName;
	MapDisplayNames.Empty();

	if (!UISettings.IsValid()) return;

	const FStrafeGameModeInfo* GameModeInfo = UISettings->AvailableGameModes.FindByPredicate(
		[this](const FStrafeGameModeInfo& Info)
		{
			return Info.DisplayName.ToString() == SelectedGameModeName;
		});

	if (GameModeInfo)
	{
		for (const TSoftObjectPtr<UWorld>& MapAsset : GameModeInfo->CompatibleMaps)
		{
			MapDisplayNames.Add(FPaths::GetBaseFilename(MapAsset.ToString()));
		}
	}

	if (MapDisplayNames.Num() > 0)
	{
		SelectedMapName = MapDisplayNames[0];
	}

	BroadcastDataChanged();
}