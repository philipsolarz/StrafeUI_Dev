// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_FindGameWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "ViewModel/S_UI_VM_ServerBrowser.h"
#include "S_UI_FindGameWidget.generated.h"

class UCommonButtonBase;
class UListView;

/**
 * @class S_UI_FindGameWidget
 * @brief The C++ base for the Find Game screen.
 * Binds to a ViewModel to display a list of servers.
 */
UCLASS(Abstract)
class STRAFEUI_API US_UI_FindGameWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

public:
    /** Sets the ViewModel for this widget and triggers the initial UI update. */
    void SetViewModel(US_UI_ViewModelBase* InViewModel);

    virtual US_UI_ViewModelBase* CreateViewModel() override;

protected:
    virtual void NativeOnInitialized() override;

private:
    /** Called when the ViewModel's data has changed, refreshing the UI. */
    UFUNCTION()
    void OnServerListUpdated();

    //~ Button Click Handlers
    // The declaration for HandleRefreshClicked() is removed from here.
    UFUNCTION()
    void HandleJoinClicked();

    UFUNCTION()
    void HandleBackClicked();

    /** The ViewModel that provides data for this widget. */
    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_ServerBrowser> ViewModel;

    //~ UPROPERTY Bindings
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UListView> List_Servers;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Refresh;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Join;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonButtonBase> Btn_Back;
};