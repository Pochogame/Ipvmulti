#include "Actors/AmmoPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Ipvmulti/IpvmultiCharacter.h"  
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AAmmoPickup::AAmmoPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    
    bReplicates = true;

    
    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComp->InitSphereRadius(50.f);
    SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
    RootComponent = SphereComp;

    
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    
    RestoreAmount = 5;

    
    SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AAmmoPickup::OnOverlapBegin);
}

void AAmmoPickup::BeginPlay()
{
    Super::BeginPlay();
}

void AAmmoPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
                                 AActor* OtherActor,
                                 UPrimitiveComponent* OtherComp,
                                 int32 OtherBodyIndex,
                                 bool bFromSweep,
                                 const FHitResult& SweepResult)
{
    
    if (!HasAuthority())
        return;

    if (OtherActor == nullptr)
        return;

   
    AIpvmultiCharacter* PlayerChar = Cast<AIpvmultiCharacter>(OtherActor);
    if (PlayerChar)
    {
        
        PlayerChar->ServerRestoreAmmo();

        
        if (PlayerChar->IsLocallyControlled())
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Munición restaurada a 5"));
        }

        
        Destroy();
    }
}
