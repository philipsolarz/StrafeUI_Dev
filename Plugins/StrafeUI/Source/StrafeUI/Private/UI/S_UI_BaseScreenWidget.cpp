// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_BaseScreenWidget.cpp

#include "UI/S_UI_BaseScreenWidget.h"
#include "S_UI_Subsystem.h"
#include "GameFramework/PlayerController.h"
#include "S_UI_PlayerController.h"

US_UI_Subsystem* US_UI_BaseScreenWidget::GetUISubsystem() const
{
    if (AS_UI_PlayerController* PC = Cast<AS_UI_PlayerController>(GetOwningPlayer()))
    {
        return PC->GetGameInstance()->GetSubsystem<US_UI_Subsystem>();
    }
    return nullptr;
}