// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoUI.h"
#include "Components/TextBlock.h"

void UAmmoUI::UpdateAmmoText(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (AmmoText)
	{
		
		FText AmmoDisplayFormat = FText::FromString(TEXT("{0} / {1}"));
		FText NewAmmoText = FText::Format(AmmoDisplayFormat, FText::AsNumber(CurrentAmmo), FText::AsNumber(MaxAmmo));

		
		AmmoText->SetText(NewAmmoText);
	}
}