// IpvmultiCharacter.cpp
#include "IpvmultiCharacter.h"
#include "AmmoUI.h"
#include "Blueprint/UserWidget.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Projectile.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AIpvmultiCharacter::AIpvmultiCharacter()
{
    // Colisión y movimiento
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
    bUseControllerRotationPitch = bUseControllerRotationYaw = bUseControllerRotationRoll = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0,500,0);
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

    // Cámara
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.f;
    CameraBoom->bUsePawnControlRotation = true;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
    FollowCamera->SetupAttachment(CameraBoom);

    // Salud
    MaxHealth = 100.f;
    CurrentHealth = MaxHealth;

    // Proyectil y fuego
    ProjectileClass = AProjectile::StaticClass();
    FireRate = 0.25f;
    bIsFiringWeapon = false;

    // Munición
    MaxAmmo = 5;
    CurrentAmmo = MaxAmmo;

    bReplicates = true;
}

void AIpvmultiCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocallyControlled() && AmmoWidgetClass)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            AmmoWidgetInstance = CreateWidget<UAmmoUI>(PC, AmmoWidgetClass);
            if (AmmoWidgetInstance)
            {
                AmmoWidgetInstance->AddToViewport();
                AmmoWidgetInstance->UpdateAmmoText(CurrentAmmo, MaxAmmo);
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

//////////////////////////////////////////////////////////////////////////
// Input

void AIpvmultiCharacter::NotifyControllerChanged()
{
    Super::NotifyControllerChanged();

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AIpvmultiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AIpvmultiCharacter::Move);
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AIpvmultiCharacter::Look);
        EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AIpvmultiCharacter::TryFire);
    }
}

void AIpvmultiCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MV = Value.Get<FVector2D>();
    if (Controller)
    {
        const FRotator YawRot(0, Controller->GetControlRotation().Yaw, 0);
        AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::X), MV.Y);
        AddMovementInput(FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y), MV.X);
    }
}

void AIpvmultiCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LV = Value.Get<FVector2D>();
    if (Controller)
    {
        AddControllerYawInput(LV.X);
        AddControllerPitchInput(LV.Y);
    }
}

//////////////////////////////////////////////////////////////////////////
// Salud

void AIpvmultiCharacter::OnRep_CurrentHealth()
{
    OnHealthUpdate();
}

void AIpvmultiCharacter::OnHealthUpdate_Implementation()
{
    if (IsLocallyControlled())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
            FString::Printf(TEXT("Health: %f"), CurrentHealth));
        if (CurrentHealth <= 0)
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("DEAD"));
    }
    if (HasAuthority())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue,
            FString::Printf(TEXT("%s health: %f"), *GetName(), CurrentHealth));
    }

    if (CurrentHealth <= 0.f)
    {
        HandleDeath();
    }
}

void AIpvmultiCharacter::SetCurrentHealth(float healthValue)
{
    if (HasAuthority())
    {
        CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
        OnHealthUpdate();
    }
}

float AIpvmultiCharacter::TakeDamage(float DamageTaken, FDamageEvent const& DE, 
                                     AController* I, AActor* C)
{
    float Applied = Super::TakeDamage(DamageTaken, DE, I, C);
    if (Applied > 0.f && HasAuthority())
        SetCurrentHealth(CurrentHealth - Applied);
    return Applied;
}

void AIpvmultiCharacter::OpenLobby()
{
    UWorld* World = GetWorld();
    if (!World) return;
    World -> ServerTravel("Game/ThirdPerson/Maps/Lobby.Lobby?listen");
}

void AIpvmultiCharacter::CallOpenLevel(const FString& IPAdress)
{
    UGameplayStatics::OpenLevel(this, *IPAdress);
}

void AIpvmultiCharacter::CallClientTravel(const FString& IPAdress)
{
    APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
    if (PlayerController)
    {
        PlayerController->ClientTravel(IPAdress,TRAVEL_Absolute);
    }
}

//////////////////////////////////////////////////////////////////////////
// Munición y disparo

void AIpvmultiCharacter::OnRep_Ammo()
{
    if (AmmoWidgetInstance)
        AmmoWidgetInstance->UpdateAmmoText(CurrentAmmo, MaxAmmo);
}

void AIpvmultiCharacter::ServerRestoreAmmo_Implementation()
{
    CurrentAmmo = MaxAmmo;
    OnRep_Ammo();
}

void AIpvmultiCharacter::TryFire()
{
    if (!bIsFiringWeapon && CurrentAmmo > 0)
        HandleFire();
}

void AIpvmultiCharacter::StopFire()
{
    bIsFiringWeapon = false;
}



void AIpvmultiCharacter::HandleFire_Implementation()
{
    // Doble chequeo en el servidor para asegurar que todo es válido
    if (!bIsFiringWeapon && CurrentAmmo > 0 && ProjectileClass != nullptr)
    {
        bIsFiringWeapon = true;
        
        CurrentAmmo--;
        OnRep_Ammo();  

        // Spawn proyectil (con el chequeo de ProjectileClass añadido por seguridad)
        FVector Loc = GetActorLocation() + GetActorForwardVector()*100 + GetActorUpVector()*50;
        FRotator Rot = GetControlRotation(); // Usamos la rotación del control para que el proyectil vaya hacia donde miras.
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        GetWorld()->SpawnActor<AProjectile>(ProjectileClass, Loc, Rot, SpawnParams);

        // Iniciar el temporizador para poder volver a disparar
        GetWorld()->GetTimerManager().SetTimer(FiringTimer, this, 
            &AIpvmultiCharacter::StopFire, FireRate, false);
    }
    else
    {
        // Si por alguna razón la lógica falla (ej. sin munición), nos aseguramos de no quedar bloqueados
        StopFire();
    }
}

void AIpvmultiCharacter::HandleDeath()
{
    // 1) Ragdoll
    if (USkeletalMeshComponent* Skel = GetMesh())
    {
        Skel->SetSimulatePhysics(true);
        Skel->SetCollisionProfileName(TEXT("Ragdoll"));
    }

    // 2) Deshabilitar input
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        DisableInput(PC);
    }

    // 3) Ocultar UI existente (munición, healthbar, etc.)
    if (AmmoWidgetInstance && AmmoWidgetInstance->IsInViewport())
    {
        AmmoWidgetInstance->RemoveFromParent();

        // 4) Mostrar Game Over (solo en cliente local)
        if (IsLocallyControlled() && GameOverWidgetClass)
        {
            if (APlayerController* PC = Cast<APlayerController>(GetController()))
            {
                GameOverWidgetInstance = CreateWidget<UUserWidget>(PC, GameOverWidgetClass);
                if (GameOverWidgetInstance)
                {
                    GameOverWidgetInstance->AddToViewport();
                }
            }
        }
    }
}
