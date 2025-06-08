// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_CreateGameWidget.cpp

#include "UI/S_UI_CreateGameWidget.h"
#include "ViewModel/S_UI_VM_CreateGame.h"
#include "Components/EditableTextBox.h"
#include "UI/S_UI_StringComboBox.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "S_UI_Settings.h"
#include "UI/S_UI_CollapsibleBox.h"

US_UI_ViewModelBase* US_UI_CreateGameWidget::CreateViewModel()
{
	US_UI_VM_CreateGame* VM = NewObject<US_UI_VM_CreateGame>(this);
	VM->Initialize(GetDefault<US_UI_Settings>());
	return VM;
}


void US_UI_CreateGameWidget::SetViewModel(US_UI_ViewModelBase* InViewModel)
{
	if (US_UI_VM_CreateGame* InCreateGameViewModel = Cast<US_UI_VM_CreateGame>(InViewModel))
	{
		if (InCreateGameViewModel)
		{
			ViewModel = InCreateGameViewModel;

			// --- INITIAL SETUP ---
			// The list of game modes is static, so we only need to populate it once.
			if (ViewModel.IsValid() && Cmb_GameMode)
			{
				Cmb_GameMode->ClearOptions();
				for (const FString& GameModeName : ViewModel->GameModeDisplayNames)
				{
					Cmb_GameMode->AddOption(GameModeName);
				}

				// Set the initial selection in the ComboBox from the ViewModel's default.
				if (ViewModel->GameModeDisplayNames.Num() > 0)
				{
					Cmb_GameMode->SetSelectedOption(ViewModel->SelectedGameModeName);
				}
			}

			// Now, bind to data changes for dynamic updates (like the map list).
			ViewModel->OnDataChanged.AddUniqueDynamic(this, &US_UI_CreateGameWidget::OnViewModelDataChanged);

			// Trigger the first update for the map list.
			OnViewModelDataChanged();
		}
	}
}

void US_UI_CreateGameWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_CreateGame)
	{
		Btn_CreateGame->OnClicked.AddDynamic(this, &US_UI_CreateGameWidget::OnCreateGameClicked);
	}

	if (Cmb_GameMode)
	{
		Cmb_GameMode->OnSelectionChanged.AddDynamic(this, &US_UI_CreateGameWidget::OnGameModeSelectionChanged);
	}
}

// This function now ONLY updates what can dynamically change.
void US_UI_CreateGameWidget::OnViewModelDataChanged()
{
	if (ViewModel.IsValid() && Cmb_Map)
	{
		// Repopulate the Maps dropdown, as this list changes based on the Game Mode.
		Cmb_Map->ClearOptions();
		for (const FString& MapName : ViewModel->MapDisplayNames)
		{
			Cmb_Map->AddOption(MapName);
		}

		if (ViewModel->MapDisplayNames.Num() > 0)
		{
			Cmb_Map->SetSelectedOption(ViewModel->SelectedMapName);
		}
	}
}

void US_UI_CreateGameWidget::OnGameModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// Ensure we don't process redundant events, and only when the user interacts directly.
	if (ViewModel.IsValid() && SelectionType == ESelectInfo::OnMouseClick && ViewModel->SelectedGameModeName != SelectedItem)
	{
		ViewModel->OnGameModeChanged(SelectedItem);
	}
}

void US_UI_CreateGameWidget::OnCreateGameClicked()
{
	if (ViewModel.IsValid())
	{
		// Push UI state to the ViewModel before creating the game
		ViewModel->GameName = Txt_GameName->GetText().ToString();
		ViewModel->bIsPrivate = Chk_IsPrivate->IsChecked();
		ViewModel->MaxPlayers = Sld_MaxPlayers->GetValue();
		ViewModel->Password = Txt_Password->GetText().ToString();
		ViewModel->SelectedGameModeName = Cmb_GameMode->GetSelectedOption();
		ViewModel->SelectedMapName = Cmb_Map->GetSelectedOption();

		ViewModel->CreateGame();
	}
}