// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

// --- AÑADIDO PARA MUNICIÓN Y UI ---
#include "AmmoUI.h"
// --- FIN DE LO AÑADIDO ---

#include "Projectile.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "IpvmultiCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AIpvmultiCharacter : public ACharacter
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
    UInputAction* FireAction;

public:
    AIpvmultiCharacter();
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Pide al servidor restaurar la munición. */
    UFUNCTION(Server, Reliable)
    void ServerRestoreAmmo();

protected:
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    // --- SECCIÓN DE DISPARO (MODIFICADA Y UNIFICADA) ---

    UPROPERTY(EditDefaultsOnly, Category="Gameplay|Projectile")
    TSubclassOf<class AProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category="Gameplay")
    float FireRate;

    bool bIsFiringWeapon;

    /** Lógica de disparo que comprueba la munición antes de proceder. */
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void TryFire();

    /** Detiene la ráfaga de disparo. */
    UFUNCTION() // No necesita ser BlueprintCallable si solo lo usa el timer
    void StopFire();

    /** Servidor: consume munición e instancia el proyectil. */
    UFUNCTION(Server, Reliable)
    void HandleFire();

    FTimerHandle FiringTimer;

    // --- FIN DE SECCIÓN DE DISPARO ---


    // --- AÑADIDO PARA MUNICIÓN Y UI ---

    /** Munición máxima */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    int32 MaxAmmo;

    /** Munición actual (replicada a todos los clientes). */
    UPROPERTY(ReplicatedUsing = OnRep_Ammo, VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    int32 CurrentAmmo;

    /** Clase de Widget a usar para la munición. Se asigna en el Editor. */
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> AmmoWidgetClass;

    /** Instancia del widget de munición creado. */
    UPROPERTY()
    UAmmoUI* AmmoWidgetInstance;

    /** Notificador que se llama cuando CurrentAmmo cambia. */
    UFUNCTION()
    void OnRep_Ammo();

    // -- YA NO SE NECESITA ServerConsumeAmmo, la lógica está en HandleFire --

    // --- FIN DE LO AÑADIDO ---


protected:
    virtual void NotifyControllerChanged() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void BeginPlay() override;

public:
    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    // --- SECCIÓN DE VIDA (CÓDIGO ORIGINAL CONSERVADO CON MEJORAS) ---
    UFUNCTION(BlueprintPure, Category="Health")
    FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintPure, Category="Health")
    FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, Category="Health")
    void SetCurrentHealth(float healthValue);

    // ✨ CAMBIO IMPORTANTE: Añadido "override" para mayor seguridad.
    UFUNCTION(BlueprintCallable, Category = "Health")
    float TakeDamage( float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser ) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxHealth;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
    float CurrentHealth;

    UFUNCTION()
    void OnRep_CurrentHealth();

    UFUNCTION(BlueprintNativeEvent, Category="Health")
    void OnHealthUpdate();
    // --- FIN DE SECCIÓN DE VIDA ---


    // --- AÑADIDO PARA MUNICIÓN Y UI ---
    UFUNCTION(BlueprintPure, Category="Combat")
    FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintPure, Category="Combat")
    FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
    // --- FIN DE LO AÑADIDO ---
};