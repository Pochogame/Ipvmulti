// Copyright Epic Games, Inc. All Rights Reserved.

#include "IpvmultiCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "AmmoUI.h"
#include "Blueprint/UserWidget.h"
// Asegúrate de incluir Projectile.h si no está ya en tu PCH
#include "Projectile.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AIpvmultiCharacter::AIpvmultiCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
       
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;  
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;  
    CameraBoom->bUsePawnControlRotation = true; 

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // --- INICIALIZACIÓN DE VARIABLES ---
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;

    ProjectileClass = AProjectile::StaticClass();
    FireRate = 0.25f;
    bIsFiringWeapon = false;

    // ✨ CORRECCIÓN: La munición máxima ahora es 5.
    MaxAmmo = 5; 
    CurrentAmmo = MaxAmmo;
}

void AIpvmultiCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocallyControlled())
    {
        if (AmmoWidgetClass)
        {
            APlayerController* FPC = Cast<APlayerController>(GetController());
            if (FPC)
            {
                 AmmoWidgetInstance = CreateWidget<UAmmoUI>(FPC, AmmoWidgetClass);
                 if (AmmoWidgetInstance)
                 {
                    AmmoWidgetInstance->AddToViewport();
                    AmmoWidgetInstance->UpdateAmmoText(GetCurrentAmmo(), GetMaxAmmo());
                 }
            }
        }
    }
}

void AIpvmultiCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AIpvmultiCharacter, CurrentHealth);
    DOREPLIFETIME(AIpvmultiCharacter, CurrentAmmo);
}

// ... (El resto de funciones como NotifyControllerChanged, SetupPlayerInputComponent, Move, Look, etc., permanecen igual) ...
// ... (Solo necesitas reemplazar las que se muestran a continuación) ...

// ====================================================================================
//                             SECCIÓN DE VIDA Y DAÑO
// ====================================================================================

void AIpvmultiCharacter::OnRep_CurrentHealth()
{
    OnHealthUpdate();
}

void AIpvmultiCharacter::OnHealthUpdate_Implementation()
{
    // Esta lógica de mensajes está bien, la dejamos como está.
    if (IsLocallyControlled())
    {
       FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
       GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);

       if (CurrentHealth <= 0)
       {
          FString deathMessage = FString::Printf(TEXT("You have been killed."));
          GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
          // Aquí podrías añadir lógica para deshabilitar al jugador, etc.
       }
    }

    if (GetLocalRole() == ROLE_Authority)
    {
       FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
       GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, healthMessage);
    }
}

void AIpvmultiCharacter::SetCurrentHealth(float healthValue)
{
    if (GetLocalRole() == ROLE_Authority)
    {
       CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
       OnHealthUpdate();
    }
}

// ✨ CORRECCIÓN IMPORTANTE: Lógica de TakeDamage arreglada.
float AIpvmultiCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Llama a la función base primero.
    float ActualDamage = Super::TakeDamage(DamageTaken, DamageEvent, EventInstigator, DamageCauser);
    
    // Solo aplica el daño si es mayor que cero.
    if (ActualDamage > 0.0f)
    {
        // La vida solo debe cambiar en el servidor.
        if (GetLocalRole() == ROLE_Authority)
        {
            SetCurrentHealth(CurrentHealth - ActualDamage);
        }
    }

    // Devuelve el daño que realmente se aplicó.
    return ActualDamage;
}


// ====================================================================================
//                         SECCIÓN DE MUNICIÓN Y DISPARO
// ====================================================================================

void AIpvmultiCharacter::OnRep_Ammo()
{
    if (AmmoWidgetInstance)
    {
        AmmoWidgetInstance->UpdateAmmoText(CurrentAmmo, MaxAmmo);
    }
}

void AIpvmultiCharacter::ServerRestoreAmmo_Implementation()
{
    CurrentAmmo = MaxAmmo;
    OnRep_Ammo(); // Llama al RepNotify en el servidor para actualizar su UI si la tuviera.
}

void AIpvmultiCharacter::TryFire()
{
    // Chequeo en el cliente para respuesta inmediata y evitar enviar RPCs innecesarios.
    if (!bIsFiringWeapon && CurrentAmmo > 0)
    {
        // Llama a la función del servidor para que se encargue de la lógica real.
        HandleFire();
    }
}

void AIpvmultiCharacter::StopFire()
{
    bIsFiringWeapon = false;
}

// ✨ CORRECCIÓN: Lógica de disparo y munición unificada y simplificada.
void AIpvmultiCharacter::HandleFire_Implementation()
{
    // Doble chequeo autoritativo en el servidor.
    if (!bIsFiringWeapon && CurrentAmmo > 0)
    {
       bIsFiringWeapon = true;

       // 1. Consumir munición directamente aquí. No se necesita otra función RPC.
       CurrentAmmo--;
       // Llama al OnRep manualmente en el servidor para que se actualice de inmediato.
       OnRep_Ammo(); 

       // 2. Generar proyectil.
       FVector spawnLocation = GetActorLocation() + ( GetActorRotation().Vector() * 100.0f ) + (GetActorUpVector() * 50.0f);
       FRotator spawnRotation = GetActorRotation();
       FActorSpawnParameters spawnParameters;
       spawnParameters.Instigator = GetInstigator();
       spawnParameters.Owner = this;
       
       if (ProjectileClass)
       {
          GetWorld()->SpawnActor<AProjectile>(ProjectileClass, spawnLocation, spawnRotation, spawnParameters);
       }

       // 3. Iniciar el temporizador para poder volver a disparar.
       GetWorld()->GetTimerManager().SetTimer(FiringTimer, this, &AIpvmultiCharacter::StopFire, FireRate, false);
    }
}