// Plugins/StrafeUI/Source/StrafeUI/Private/ViewModel/S_UI_VM_CreateGame.cpp

#include "ViewModel/S_UI_VM_CreateGame.h"
#include "S_UI_Settings.h"
#include "Kismet/GameplayStatics.h"

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
	if (!UISettings.IsValid()) return;

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

	UClass* GameModeClass = SelectedGameModeInfo->GameModeClass.LoadSynchronous();
	if (!GameModeClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateGame failed: Could not load GameModeClass asset"));
		return;
	}

	// Construct the options string for OpenLevel
	FString Options = FString::Printf(TEXT("?listen -game=%s"), *GameModeClass->GetPathName());

	UE_LOG(LogTemp, Log, TEXT("Opening level %s with options: %s"), *SelectedMapAsset->ToString(), *Options);
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, *SelectedMapAsset, true, Options);
}

void US_UI_VM_CreateGame::OnGameModeChanged(FString InSelectedGameModeName)
{
	SelectedGameModeName = InSelectedGameModeName;
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