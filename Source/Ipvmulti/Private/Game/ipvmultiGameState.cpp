// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ipvmultiGameState.h"

#include "EngineUtils.h"

void AipvmultiGameState::MultiCastOnMissionCompleted_Implementation(APawn* instigatorPawn, bool bMissionSuccess)
{
	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		if (APawn* Pawn = PC->GetPawn())
			Pawn->DisableInput(nullptr);
}
