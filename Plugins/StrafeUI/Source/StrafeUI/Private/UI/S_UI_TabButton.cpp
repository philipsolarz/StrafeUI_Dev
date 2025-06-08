// Plugins/StrafeUI/Source/StrafeUI/Private/UI/S_UI_TabButton.cpp

#include "UI/S_UI_TabButton.h"
#include "CommonTextBlock.h"

void US_UI_TabButton::SetTabLabelText(const FText& InText)
{
    if (TabLabel)
    {
        TabLabel->SetText(InText);
    }
}