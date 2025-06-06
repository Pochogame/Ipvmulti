// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoUI.generated.h"

// Forward declaration para evitar includes circulares
class UTextBlock;

UCLASS()
class IPVMULTI_API UAmmoUI : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Actualiza el texto de munición en el widget.
	 * @param CurrentAmmo La munición actual a mostrar.
	 * @param MaxAmmo La munición máxima a mostrar.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateAmmoText(int32 CurrentAmmo, int32 MaxAmmo);

protected:
	/**
	 * El componente de texto que muestra la munición.
	 * DEBE llamarse "AmmoText" en el Blueprint del Widget y estar marcado como variable.
	 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoText;
};