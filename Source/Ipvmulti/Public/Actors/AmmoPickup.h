#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;


UCLASS()
class IPVMULTI_API AAmmoPickup : public AActor
{
	GENERATED_BODY()
    
public:
	AAmmoPickup();

protected:
	virtual void BeginPlay() override;

	/** Componente de colisión tipo esfera para detectar superposiciones */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	USphereComponent* SphereComp;

	/** Static Mesh para que el ítem sea visible en el mundo */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	UStaticMeshComponent* MeshComp;

	/** Lógica que ocurre cuando otro actor empieza a superponerse con SphereComp */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
						AActor* OtherActor,
						UPrimitiveComponent* OtherComp,
						int32 OtherBodyIndex,
						bool bFromSweep,
						const FHitResult& SweepResult);

	/** Cantidad fija de munición que restauramos (siempre igual a MaxAmmo del Character).
	 *  Lo dejamos en 5 por defecto, pero usaríamos MaxAmmo del Character igualmente. */
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	int32 RestoreAmount;
};
