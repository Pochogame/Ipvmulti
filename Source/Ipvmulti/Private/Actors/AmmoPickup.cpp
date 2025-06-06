#include "Actors/AmmoPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Ipvmulti/IpvmultiCharacter.h"  
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AAmmoPickup::AAmmoPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    // Habilitamos que este actor se replique
    bReplicates = true;

    // 1) Creamos el componente de colisión (Esfera)
    SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    SphereComp->InitSphereRadius(50.f);
    SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
    RootComponent = SphereComp;

    // 2) Creamos el mesh estático (para que se vea)
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 3) Restauramos “RestoreAmount” (por defecto 5)
    RestoreAmount = 5;

    // Vinculamos el evento de superposición
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
    // Solo el servidor debe procesar el pickup
    if (!HasAuthority())
        return;

    if (OtherActor == nullptr)
        return;

    // ¿Es nuestro Character?
    AIpvmultiCharacter* PlayerChar = Cast<AIpvmultiCharacter>(OtherActor);
    if (PlayerChar)
    {
        // Llamamos al RPC que restaura la munición en el Character (servidor)
        PlayerChar->ServerRestoreAmmo();

        // (Opcional) Mostrar feedback en servidor/cliente local
        if (PlayerChar->IsLocallyControlled())
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Munición restaurada a 5"));
        }

        // Destruimos este pickup (solo en servidor; se replicará a clientes)
        Destroy();
    }
}
