// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoUI.generated.h"


class UTextBlock;

UCLASS()
class IPVMULTI_API UAmmoUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	
	 * @param CurrentAmmo 
	 * @param MaxAmmo 
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateAmmoText(int32 CurrentAmmo, int32 MaxAmmo);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;
};