// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "COOPGame.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


//برای نشان دادن دیباگ لاین فقط در صورت خاص
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw Debug Lines for Weapons"), 
	ECVF_Cheat);
// Sets default values
ASWeapon::ASWeapon()
{
 	
	//تعریف تفنگ که اسکلت مش هست
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("Component");
	RootComponent = MeshComp;
	//محل در آمدن پارتیکل افکت از جسم
	MuzzleScoketName = "MuzzleScoket";
	TracerTargetName = "Target";

	BaseDamage = 20.f;
	RateOfFire = 600.f;//rate of fire per minutes
	SetReplicates(true);//replicate weapon on the client sideتفنگ را در قسمت کلاینت نیز قرار می دهد
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	TimeBetweenShots = 60 / RateOfFire;
}

//شلیک با تفنگ
void ASWeapon::Fire()
{
	if (Role < ROLE_Authority) {
		ServerFire();
		
	}
	//trace the world from pawn eye to crosshair location
	AActor* MyOwner = GetOwner();
	if (MyOwner) {
		//هیت برای نگهداری اطلاعات است
		FHitResult Hit;	//Hold Data,It is struct
		FVector EyeLocation;		//نقطه ابتدایی تریس

		FRotator EyeRotation;		
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);		//فاصله از چشم که تغییر داده شد

		FVector ShotDirection = EyeRotation.Vector();	//مسیر تیر	

		FVector TraceEnd = EyeLocation + (ShotDirection) * 10000;//پایان trace

		FCollisionQueryParams QueryPrams;		//پارامترهای کلایدر می باشد در تریس

		QueryPrams.AddIgnoredActor(MyOwner);		//حذف کلایدر فرد یا در نظر نگرفتن ان


		QueryPrams.AddIgnoredActor(this);		//به صورت دقیق تر تریس انجام گیرد

		QueryPrams.bTraceComplex = true;

		QueryPrams.bReturnPhysicalMaterial = true;		//particle system parameter

		FVector TraceEndPoint = TraceEnd;		//بولین می باشد اگر باشد تریس صورت میگیرد
		EPhysicalSurface SurfaceType = SurfaceType_Default;
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd , CollisionWeapon, QueryPrams)) {
			//Blocking Hit Process damage
				AActor* HitActor = Hit.GetActor();			//موجودی را که مورد اصابت ضربه قرار می گیرد را نشان می دهد. 

			 SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			float ActualDamage = BaseDamage;
 			if (SurfaceType == Surface_FleshVulnarble)
			{
 				 ActualDamage *= 4;
 			}
 			
			//اسیب دیدگی را نمایش می دهد
			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
			//برای ساخت پارتیکل ایفکت تیر خوردن
			//physics parameter
			PlayImpactEffect(SurfaceType, Hit.ImpactPoint);
 			
			TraceEndPoint = Hit.ImpactPoint;

		}
		if (DebugWeaponDrawing>0) {
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, true, 1.0f, 0, 1.f);

		}
		PlayFireEffect(TraceEndPoint);
		if (Role == ROLE_Authority) {
			HitScanTrace.TraceTo = TraceEndPoint;
			HitScanTrace.SurfaceType = SurfaceType;

		}
		LastTimeFired = GetWorld()->TimeSeconds;
	}

}

void ASWeapon::OnRep_HitScanTrace()
{
	//Play Cosmetic FX
	PlayFireEffect(HitScanTrace.TraceTo);
	PlayImpactEffect(HitScanTrace.SurfaceType, HitScanTrace.TraceTo);
}

//Implementation

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}
//it needs validate because we use validation in server fire

bool ASWeapon::ServerFire_Validate()
{
	return true;
}


void ASWeapon::PlayFireEffect(FVector TraceEnd)
{
	if (MuzzleEffect) {
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleScoketName);
	}
	//برای پارتیکل دود می باشد
	if (TracerEffect) {

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleScoketName);
		UParticleSystemComponent* TraccerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
		if (TraccerComp) {
			//برای نشان دادن مسیر می باشد
			TraccerComp->SetVectorParameter(TracerTargetName, TraceEnd);
		}
	}
	//برای لرزش دوربین حاصل از شلیک
	APawn* MyyOwner = Cast<APawn>(GetOwner());
	if (MyyOwner) {
		APlayerController* PC = Cast<APlayerController>(MyyOwner->GetController());
		if (PC) {
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactEffect(EPhysicalSurface SurfaceType,FVector ImpactPoint)
{
	UParticleSystem* SelectedEffect = nullptr;// متغییر کمکی برای اینکه در هر قسمت چه چیزی رخ می دهد.نشان دهد
	switch (SurfaceType)//برخورد گلوله به هر قسمت چه پارتیک ایفکتی را نشان می دهد.
	{
	case Surface_FleshDefault://اگر به بدن بخورد باید خون بریزد

	case Surface_FleshVulnarble://اگر به سر بخورد باید خون بریزد
		SelectedEffect = FleshImpactEffect;

		break;
	default:
		SelectedEffect = DefaultImpactEffect;//اگر به جای دیگه بخورد چیزی رخ ندهد

		break;
	}
	if (SelectedEffect) {
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleScoketName);
		FVector ShotDirection = ImpactPoint - MuzzleLocation;
		ShotDirection.Normalize();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ShotDirection.Rotation());

	}
}

//this is for automatic shot
void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastTimeFired + TimeBetweenShots - GetWorld()->TimeSeconds,0.f);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots,this,&ASWeapon::Fire, TimeBetweenShots, true,FirstDelay );
	Fire();
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}
//this function shows how we want replicate something

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//before that we told in current weapon in h files we want to replicated
	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace,COND_SkipOwner);
}