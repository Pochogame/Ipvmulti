// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ipvmultiGameState.generated.h"

/**
 * 
 */
UCLASS()
class IPVMULTI_API AipvmultiGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void MultiCastOnMissionCompleted(APawn* instigatorPawn,bool bMissionSuccess);
};
