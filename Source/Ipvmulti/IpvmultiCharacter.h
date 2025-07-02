// IpvmultiCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IpvmultiCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class UAmmoUI;         
class UUserWidget;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class IPVMULTI_API AIpvmultiCharacter : public ACharacter
{
    GENERATED_BODY()

    /** Cámara */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    /** Input */
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
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** RPC para restaurar munición */
    UFUNCTION(Server, Reliable)
    void ServerRestoreAmmo();

protected:
    // Movimiento / mirada
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);

    // Proyectil
    UPROPERTY(EditDefaultsOnly, Category="Gameplay|Projectile")
    TSubclassOf<class AProjectile> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, Category="Gameplay")
    float FireRate;

    bool bIsFiringWeapon;

    /** Disparo (cliente llama esto) */
    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void TryFire();

    /** Detener ráfaga */
    UFUNCTION()
    void StopFire();

    /** Servidor: instancia proyectil y consume muni */
    UFUNCTION(Server, Reliable)
    void HandleFire();

    FTimerHandle FiringTimer;

    // Munición
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    int32 MaxAmmo;

    UPROPERTY(ReplicatedUsing = OnRep_Ammo, VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    int32 CurrentAmmo;

    // Widget UI
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> AmmoWidgetClass;

    UPROPERTY()
    UAmmoUI* AmmoWidgetInstance;

    // Game Over widget
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TSubclassOf<UUserWidget> GameOverWidgetClass;

    UPROPERTY()
    UUserWidget* GameOverWidgetInstance;

    void HandleDeath();


    UFUNCTION()
    void OnRep_Ammo();

    // Funciones UE overrides
    virtual void NotifyControllerChanged() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void BeginPlay() override;

public:
    // Getters
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE UCameraComponent*   GetFollowCamera() const { return FollowCamera; }

    UFUNCTION(BlueprintPure, Category="Combat")
    FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }

    UFUNCTION(BlueprintPure, Category="Combat")
    FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }

    // Salud
    UPROPERTY(EditDefaultsOnly, Category = "Health")
    float MaxHealth;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
    float CurrentHealth;

    UFUNCTION()
    void OnRep_CurrentHealth();

    UFUNCTION(BlueprintNativeEvent, Category="Health")
    void OnHealthUpdate();

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetCurrentHealth(float healthValue);

    virtual float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, 
                             AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintPure, Category="Health")
    FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category="Health")
    FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable)
    void OpenLobby();

    UFUNCTION(BlueprintCallable)
    void CallOpenLevel(const FString& IPAdress);

    void CallClientTravel(const FString& IPAdress);

    UPROPERTY(BlueprintReadOnly, Category="Gameplay")
    bool bIsCarryingObjetive;
};
