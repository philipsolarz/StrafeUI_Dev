// Plugins/StrafeUI/Source/StrafeUI/Public/UI/S_UI_CreateGameWidget.h

#pragma once

#include "CoreMinimal.h"
#include "UI/S_UI_BaseScreenWidget.h"
#include "S_UI_CreateGameWidget.generated.h"

class US_UI_VM_CreateGame;
class UEditableTextBox;
class US_UI_StringComboBox;
class UCheckBox;
class USlider;
class UButton;
class US_UI_CollapsibleBox;

UCLASS(Abstract)
class STRAFEUI_API US_UI_CreateGameWidget : public US_UI_BaseScreenWidget
{
    GENERATED_BODY()

public:
    void SetViewModel(US_UI_ViewModelBase* InViewModel);

    virtual US_UI_ViewModelBase* CreateViewModel() override;

protected:
    virtual void NativeOnInitialized() override;

private:
    UFUNCTION()
    void OnViewModelDataChanged();

    UFUNCTION()
    void OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnCreateGameClicked();

    UPROPERTY()
    TWeakObjectPtr<US_UI_VM_CreateGame> ViewModel;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* Txt_GameName;

    UPROPERTY(meta = (BindWidget))
    US_UI_StringComboBox* Cmb_GameMode;

    UPROPERTY(meta = (BindWidget))
    US_UI_StringComboBox* Cmb_Map;

    UPROPERTY(meta = (BindWidget))
    UCheckBox* Chk_IsPrivate;

    UPROPERTY(meta = (BindWidget))
    USlider* Sld_MaxPlayers;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* Txt_Password;

    UPROPERTY(meta = (BindWidget))
    US_UI_CollapsibleBox* Col_GameModeSettings;

    UPROPERTY(meta = (BindWidget))
    US_UI_CollapsibleBox* Col_AdvancedSettings;

    UPROPERTY(meta = (BindWidget))
    UButton* Btn_CreateGame;
};