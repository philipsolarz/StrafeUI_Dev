// Fill out your copyright notice in the Description page of Project Settings.


#include "S_UI_Settings.h"

void US_UI_Settings::PostInitProperties()
{
    Super::PostInitProperties();

    // Ensure settings are loaded from config files
    LoadConfig();
}

#if WITH_EDITOR
void US_UI_Settings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // Persist the updated setting immediately
    SaveConfig();
}
#endif


