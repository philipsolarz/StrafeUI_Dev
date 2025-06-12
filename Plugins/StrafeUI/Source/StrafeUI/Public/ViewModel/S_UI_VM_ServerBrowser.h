// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_ServerBrowser.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "OnlineSessionSettings.h"
#include "FindSessionsCallbackProxyAdvanced.h" // Use the Advanced Sessions search result struct
#include "S_UI_VM_ServerBrowser.generated.h"

USTRUCT(BlueprintType)
struct F_ServerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FText ServerName;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FText GameMode;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FString CurrentMap;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 PlayerCount;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 Ping;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	bool bIsPrivate;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	bool bIsLAN;
};

UCLASS()
class STRAFEUI_API US_UI_VM_ServerListEntry : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	F_ServerInfo ServerInfo;

	FOnlineSessionSearchResult SessionSearchResult;
};

UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_ServerBrowser : public US_UI_ViewModelBase
{
	GENERATED_BODY()

public:
	virtual ~US_UI_VM_ServerBrowser();

	UPROPERTY(BlueprintReadOnly, Category = "Server Browser")
	TArray<F_ServerInfo> ServerList;

	UFUNCTION(BlueprintCallable, Category = "Server Browser")
	void RequestServerListRefresh();

	void JoinSession(const FOnlineSessionSearchResult& SessionSearchResult);

	// Filter properties
	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	FString FilterServerName;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	FString FilterGameMode;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	bool bFilterHideFullServers = false;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	bool bFilterHideEmptyServers = false;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	bool bFilterHidePrivateServers = false;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	int32 FilterMaxPing = 999;

	UPROPERTY(BlueprintReadWrite, Category = "Server Browser|Filters")
	bool bSearchLAN = true;

	UFUNCTION(BlueprintCallable, Category = "Server Browser")
	void ApplyFilters();

	friend class US_UI_FindGameWidget;

private:
	// Callbacks for Advanced Sessions proxy
	UFUNCTION()
	void OnFindSessionsSuccess(const TArray<FBlueprintSessionResult>& SearchResults);
	UFUNCTION()
	void OnFindSessionsFailure(const TArray<FBlueprintSessionResult>& SearchResults);

	// Callback for when join session completes
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UPROPERTY()
	TArray<TObjectPtr<US_UI_VM_ServerListEntry>> AllFoundServers;

	FDelegateHandle JoinSessionCompleteDelegateHandle;

	void UpdateFilteredServerList();

	bool PassesFilters(const TObjectPtr<US_UI_VM_ServerListEntry>& Entry) const;
};