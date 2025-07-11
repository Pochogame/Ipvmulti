

#include "Actors/ObjectiveZone.h"

#include "Components/BoxComponent.h"
#include "Components/DecalComponent.h"
#include "Ipvmulti/IpvmultiCharacter.h"
#include "Ipvmulti/IpvmultiGameMode.h"


// Sets default values
AObjectiveZone::AObjectiveZone()
{
	OverlapComponent=CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapComponent"));
	OverlapComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapComponent->SetCollisionResponseToChannels(ECR_Ignore);
	OverlapComponent->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	RootComponent=OverlapComponent;
	OverlapComponent->SetHiddenInGame(false);

	DecalComp = CreateDefaultSubobject<UDecalComponent>(TEXT("DecalComp"));
	DecalComp-> DecalSize=FVector(200);
	DecalComp->SetupAttachment(RootComponent);
	
	
	
}


void AObjectiveZone::BeginPlay()
{
	Super::BeginPlay();
	
}

void AObjectiveZone::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	OverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleOverlap);
}

void AObjectiveZone::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			10.f,
			FColor::Green,
			FString::Printf(TEXT("Overlapped"))
		);
	}
	AIpvmultiCharacter*MyPawn = Cast<AIpvmultiCharacter>(OtherActor);
	if (MyPawn==nullptr) return;
	if (MyPawn->bIsCarryingObjetive)
	{
		AIpvmultiGameMode* GM=Cast<AIpvmultiGameMode>(GetWorld()->GetAuthGameMode());
		if (GM!=nullptr)
		{
			GM->CompleteMission(MyPawn);
		}
	}
}

