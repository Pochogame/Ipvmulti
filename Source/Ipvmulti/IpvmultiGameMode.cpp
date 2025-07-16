// Copyright Epic Games, Inc. All Rights Reserved.

#include "IpvmultiGameMode.h"
#include "IpvmultiCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AIpvmultiGameMode::AIpvmultiGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AIpvmultiGameMode::CompleteMission(APawn* Pawn)
{
	if (Pawn==nullptr) return;
	Pawn->DisableInput(nullptr);
	
	if (SpectatorViewClass)
	{
		TArray<AActor*> ReturnActors;
		
		UGameplayStatics::GetAllActorsOfClass(this,SpectatorViewClass, ReturnActors);
		if (ReturnActors.Num()>0)
		{
			AActor*NewViewTarget=ReturnActors[0];
			APlayerController*PC=Cast<APlayerController>(Pawn->GetController());
			if (PC)
			{
				PC->SetViewTargetWithBlend(NewViewTarget,1.0f,VTBlend_Cubic);
			}
		}


	}
	OnMissionCompleted(Pawn);

	
}
