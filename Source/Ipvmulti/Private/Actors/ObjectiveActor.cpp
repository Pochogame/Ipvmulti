// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ObjectiveActor.h"

#include "AssetTypeActions/AssetDefinition_SoundBase.h"
#include "Components/SphereComponent.h"
#include "Ipvmulti/IpvmultiCharacter.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AObjectiveActor::AObjectiveActor()
{
	bReplicates = true;
	StaticMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent=StaticMesh;

	Sphere=CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	Sphere->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AObjectiveActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AObjectiveActor::PlayEffect()
{
	UGameplayStatics::SpawnEmitterAtLocation(this,PickupUVF,GetActorLocation());
	
}

 void AObjectiveActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	PlayEffect();

	AIpvmultiCharacter*myChar=Cast<AIpvmultiCharacter>(OtherActor);

	if (myChar)
	{
		myChar->bIsCarryingObjetive=true;
		Destroy();
	}
	
}

