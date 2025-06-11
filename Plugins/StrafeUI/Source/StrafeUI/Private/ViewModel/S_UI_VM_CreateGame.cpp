// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_CreateGame.cpp

#include "ViewModel/S_UI_VM_CreateGame.h"
#include "S_UI_Settings.h"
#include "S_UI_Subsystem.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/World.h"
#include "Data/S_UI_ScreenTypes.h"

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

void US_UI_VM_CreateGame::Initialize(const US_UI_Settings* InSettings)
{
	UISettings = InSettings;
	if (!UISettings.IsValid())
	{
		return;
	}

	// Populate the game mode display names from the settings
	for (const FStrafeGameModeInfo& GameModeInfo : UISettings->AvailableGameModes)
	{
		GameModeDisplayNames.Add(GameModeInfo.DisplayName.ToString());
	}

	// Set the initial selection to the first available game mode
	if (GameModeDisplayNames.Num() > 0)
	{
		OnGameModeChanged(GameModeDisplayNames[0]);
	}
	else
	{
		BroadcastDataChanged(); // Ensure UI is at least cleared if no modes are defined
	}
}

void US_UI_VM_CreateGame::CreateGame()
{
	// Get the Session Interface
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

	// Check if a session already exists
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogTemp, Log, TEXT("Found an existing session. Destroying it before creating a new one."));

		// Bind the completion delegate
		DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &US_UI_VM_CreateGame::OnDestroySessionComplete)
		);

		// Destroy the existing session
		if (!SessionInterface->DestroySession(NAME_GameSession))
		{
			// If the call fails, clear the delegate and log an error
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
			UE_LOG(LogTemp, Error, TEXT("Failed to submit session destroy request."));
		}
	}
	else
	{
		// If no session exists, proceed directly to creation
		CreateNewSession();
	}
}

void US_UI_VM_CreateGame::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	// Clean up the delegate
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully destroyed previous session."));
		// Now that the old session is gone, create the new one.
		CreateNewSession();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to destroy previous session."));
	}
}

void US_UI_VM_CreateGame::CreateNewSession()
{
	if (!UISettings.IsValid()) return;

	// This function now contains the logic that was originally in CreateGame()

	// Find the full GameModeInfo struct from the selected display name
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

	// Find the map asset from the selected map display name
	const TSoftObjectPtr<UWorld>* SelectedMapAsset = SelectedGameModeInfo->CompatibleMaps.FindByPredicate(
		[this](const TSoftObjectPtr<UWorld>& MapAsset)
		{
			// Get the asset name from the soft pointer path
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

	// Get the Online Subsystem
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: No online subsystem found"));
		return;
	}

	// Get the Session Interface
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Session interface is invalid"));
		return;
	}

	// Get the local player
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: No world context"));
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetLocalPlayer())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: No local player controller"));
		return;
	}

	// Create the session settings
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());

	// Basic settings
	SessionSettings->NumPublicConnections = MaxPlayers;
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bIsLANMatch = OnlineSubsystem->GetSubsystemName() == "NULL" ? true : false;
	SessionSettings->bIsDedicated = bIsDedicatedServer;
	SessionSettings->bUsesPresence = !bIsDedicatedServer;
	SessionSettings->bUseLobbiesIfAvailable = !bIsDedicatedServer;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinViaPresence = !bIsDedicatedServer;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings->BuildUniqueId = 1;

	// Custom settings - store all our game-specific data
	SessionSettings->Set(SETTING_MAPNAME, SelectedMapName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(SETTING_GAMENAME, GameName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(SETTING_GAMEMODE, SelectedGameModeName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(SETTING_SERVERDESC, ServerDescription, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(SETTING_FRIENDLYFIRE, bAllowFriendlyFire, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(SETTING_SPECTATORS, bAllowSpectators, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(SETTING_TIMELIMIT, TimeLimit, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(SETTING_SCORELIMIT, ScoreLimit, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(SETTING_RESPAWNTIME, RespawnTime, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// If password protected, store the password (Note: In production, you'd want to handle this more securely)
	if (!Password.IsEmpty())
	{
		SessionSettings->Set(FName(TEXT("PASSWORD")), Password, EOnlineDataAdvertisementType::DontAdvertise);
	}

	// Bind the completion delegate
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &US_UI_VM_CreateGame::OnCreateSessionComplete)
	);

	// Create the session
	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings))
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Failed to create session"));

		// Clean up the delegate since we won't get a callback
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		// Show error modal
		if (UWorld* WorldContext = GetWorld())
		{
			if (US_UI_Subsystem* UISubsystem = WorldContext->GetGameInstance()->GetSubsystem<US_UI_Subsystem>())
			{
				F_UIModalPayload Payload;
				Payload.Message = FText::FromString(TEXT("Failed to create game session. Please try again."));
				Payload.ModalType = E_UIModalType::OK;
				UISubsystem->RequestModal(Payload, FOnModalDismissedSignature());
			}
		}
	}
}

void US_UI_VM_CreateGame::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	// Get the session interface to clean up the delegate
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		}
	}

	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Session created successfully. Starting travel to map..."));

		// Travel to the map as a listen server
		UWorld* World = GetWorld();
		if (World && CachedGameModeClass)
		{
			FString TravelURL = FString::Printf(TEXT("%s?listen?game=%s"), *CachedMapAssetPath, *CachedGameModeClass->GetPathName());
			World->ServerTravel(TravelURL);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create session"));

		// Show error modal
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
}

void US_UI_VM_CreateGame::OnGameModeChanged(FString InSelectedGameModeName)
{
	SelectedGameModeName = InSelectedGameModeName;
	UE_LOG(LogTemp, Warning, TEXT("[CreateGameVM] OnGameModeChanged: ViewModel's game mode is now '%s'. Broadcasting change."), *SelectedGameModeName);
	MapDisplayNames.Empty();

	if (!UISettings.IsValid()) return;

	// Find the game mode info struct that matches the new selection
	const FStrafeGameModeInfo* GameModeInfo = UISettings->AvailableGameModes.FindByPredicate(
		[this](const FStrafeGameModeInfo& Info)
		{
			return Info.DisplayName.ToString() == SelectedGameModeName;
		});

	// If found, populate the map display names from its compatible maps list
	if (GameModeInfo)
	{
		for (const TSoftObjectPtr<UWorld>& MapAsset : GameModeInfo->CompatibleMaps)
		{
			MapDisplayNames.Add(FPaths::GetBaseFilename(MapAsset.ToString()));
		}
	}

	// Default to the first map in the list if available
	if (MapDisplayNames.Num() > 0)
	{
		SelectedMapName = MapDisplayNames[0];
	}

	BroadcastDataChanged();
}