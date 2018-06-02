// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "Components/CapsuleComponent.h"
#include "COOPGame.h"
#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"
// Sets default values
ASCharacter::ASCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//برای نگهدارنده دوربین.
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(RootComponent);
	//چرخش اسپرینگ آرم
	SpringArm->bUsePawnControlRotation = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(CollisionWeapon, ECR_Ignore);

	//ایا کاراکتر توانایی نشستن دارد از ai تعریف میشود
	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	//تعریف دوربین
	CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComp->SetupAttachment(SpringArm);

	HealthComp = CreateDefaultSubobject<USHealthComponent>("HealthComponent");
	//میزان زوم هنگام فشار دادن کلیک راست
	ZoomedFov = 65.f;
	InterpSpeed = 20.f;
	 WeaponScoketAttachName="WeaponScoket";
	 
}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultFov = CameraComp->FieldOfView;
	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHealthChanged);
	//only runs in server Side
	if (Role == ROLE_Authority) {
		///Spawn Default Weapon(برای ظاهر شدن تفنگ در دست بازیکن)

		//پارامتر ظاهر شدن
		FActorSpawnParameters SpawnParams;
		//همواره ظاهر می شود.
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		//ساخت تفنگ باید به این نکته توجه کرد در ساخت حتما از <tsubclass of> استفده میشود
		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(StarttWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (CurrentWeapon) {
			CurrentWeapon->SetOwner(this);
			//اتصال به جز اصلی 
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponScoketAttachName);
		}
	}
	
	
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//اگر درست باشد اولی را بر می دارد غلط باشد دومی
	float CurrentFov = bIsWantsZoom ? ZoomedFov : DefaultFov;
	//درونیابی برای نرم تر شدن زوم
	float TargetZoom = FMath::FInterpTo(CameraComp->FieldOfView, CurrentFov, DeltaTime, InterpSpeed);
	CameraComp->SetFieldOfView(TargetZoom);
}
	



// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//کنترل جابجایی به جلو
	PlayerInputComponent->BindAxis("MoveFoword",this, &ASCharacter::MoveForword);
	PlayerInputComponent->BindAxis("MoveRight", this,&ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);
	//اکشن با فشردن دکمه صورت می گیرد
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::StartCruch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::StartZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::Startfire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::Stopfire);

}
//تغییر فاصله چشم تا دوربین
 FVector ASCharacter::GetPawnViewLocation() const
 {
  	if (CameraComp) {
  		return CameraComp->GetComponentLocation();
  	}
 	return Super::GetPawnViewLocation();
 }

void ASCharacter::MoveForword(float value)
{
	AddMovementInput(GetActorForwardVector(), value);
}

void ASCharacter::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector(), value);
}

void ASCharacter::StartCruch() {
	Crouch();
	
}
void ASCharacter::EndCrouch() {
	UnCrouch();
}

void ASCharacter::StartZoom()
{
	bIsWantsZoom = true;
}

void ASCharacter::EndZoom()
{
	bIsWantsZoom = false;
}

void ASCharacter::Startfire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::Stopfire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StopFire();
}

}

void ASCharacter::OnHealthChanged(USHealthComponent* OwningHealthComponent, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	
		if (Health <= 0 && !bDied) {
			//Died
			bDied = true;
			GetMovementComponent()->StopMovementImmediately();
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			DetachFromControllerPendingDestroy();
			UE_LOG(LogTemp, Warning, TEXT("you died"));
			SetLifeSpan(10.0f);
		}
	
}
//this function shows how we want replicate something
void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASCharacter, CurrentWeapon);
}