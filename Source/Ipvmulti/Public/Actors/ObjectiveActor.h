// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectiveActor.generated.h"

class USphereComponent;
class UPrimitiveComponent;
UCLASS()
class IPVMULTI_API AObjectiveActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AObjectiveActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere,category="Components")
	UStaticMeshComponent* StaticMesh;
	
	UPROPERTY(VisibleAnywhere,category="Components")
	USphereComponent* Sphere;

	UPROPERTY(EditDefaultsOnly,category="VFX")
	UParticleSystem* PickupUVF;

	void PlayEffect();
	

public:
	

	virtual void NotifyActorBeginOverlap(AActor*OtherActor)override;
};



