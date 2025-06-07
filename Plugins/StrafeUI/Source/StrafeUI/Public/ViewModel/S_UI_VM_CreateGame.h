// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_CreateGame.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "S_UI_VM_CreateGame.generated.h"

class US_UI_Settings;
struct FStrafeGameModeInfo;

UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_CreateGame : public US_UI_ViewModelBase
{
	GENERATED_BODY()

public:
	void Initialize(const US_UI_Settings* InSettings);

	UFUNCTION(BlueprintCallable, Category = "Create Game")
	void CreateGame();

	UFUNCTION(BlueprintCallable, Category = "Create Game")
	void OnGameModeChanged(FString SelectedGameModeName);

	/** List of game mode names to display in the UI. */
	UPROPERTY(BlueprintReadOnly, Category = "Create Game")
	TArray<FString> GameModeDisplayNames;

	/** List of map names to display in the UI for the selected game mode. */
	UPROPERTY(BlueprintReadOnly, Category = "Create Game")
	TArray<FString> MapDisplayNames;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString GameName = "My Awesome Game";

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	bool bIsPrivate = false;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	int32 MaxPlayers = 8;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString Password;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString SelectedMapName;

	UPROPERTY(BlueprintReadWrite, Category = "Create Game")
	FString SelectedGameModeName;

private:
	UPROPERTY()
	TWeakObjectPtr<const US_UI_Settings> UISettings;
};