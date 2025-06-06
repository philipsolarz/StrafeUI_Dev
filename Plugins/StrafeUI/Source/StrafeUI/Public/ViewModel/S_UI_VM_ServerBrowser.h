// Plugins/StrafeUI/Source/StrafeUI/Public/ViewModel/S_UI_VM_ServerBrowser.h

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/S_UI_ViewModelBase.h"
#include "S_UI_VM_ServerBrowser.generated.h"

/**
 * @struct F_ServerInfo
 * @brief Contains information about a single game server.
 */
USTRUCT(BlueprintType)
struct F_ServerInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	FText ServerName;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 PlayerCount;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly, Category = "Server Info")
	int32 Ping;
};

/**
 * @class US_UI_VM_ServerBrowser
 * @brief ViewModel for the Server Browser screen.
 *
 * Manages the list of servers and handles refresh requests.
 */
UCLASS(BlueprintType)
class STRAFEUI_API US_UI_VM_ServerBrowser : public US_UI_ViewModelBase
{
	GENERATED_BODY()

public:
	/**
	 * The list of servers to be displayed in the UI.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Server Browser")
	TArray<F_ServerInfo> ServerList;

	/**
	 * Sends a request to refresh the server list.
	 * This would typically trigger an async call to a backend or online subsystem.
	 */
	UFUNCTION(BlueprintCallable, Category = "Server Browser")
	void RequestServerListRefresh();
};