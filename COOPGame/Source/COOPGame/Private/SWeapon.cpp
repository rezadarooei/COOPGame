// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "COOPGame.h"

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
	//محل در آمدن پارتیکل افکت
	MuzzleScoketName = "MuzzleScoket";
	TracerTargetName = "Target";
}
//شلیک با تفنگ
void ASWeapon::Fire()
{
	//trace the world from pawn eye to crosshair location
	AActor* MyOwner = GetOwner();
	if (MyOwner) {
		//هیت برای نگهداری اطلاعات است
		FHitResult Hit;	//Hold Data,It is struct
		//نقطه ابتدایی تریس
		FVector EyeLocation;
		FRotator EyeRotation;
		//مسیر تیر
		FVector ShotDirection = EyeRotation.Vector();
		//پایان trace
		FVector TraceEnd = EyeLocation + (ShotDirection) * 10000;
		//فاصله از چشم که تغییر داده شد
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		//پارامترهای کلایدر می باشد در تریس
		FCollisionQueryParams QueryPrams;
		//حذف کلایدر فرد یا در نظر نگرفتن ان
		QueryPrams.AddIgnoredActor(MyOwner);

		QueryPrams.AddIgnoredActor(this);
		//به صورت دقیق تر تریس انجام گیرد
		QueryPrams.bTraceComplex = true;

		QueryPrams.bReturnPhysicalMaterial = true;
		//particle system parameter
		FVector TraceEndPoint = TraceEnd;
		//بولین می باشد اگر باشد تریس صورت میگیرد
		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryPrams)) {
			//Blocking Hit Process damage
			//موجودی را که مورد اصابت ضربه قرار می گیرد را نشان می دهد. 
			AActor* HitActor = Hit.GetActor();
			//اسیب دیدگی را نمایش می دهد
			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
			//برای ساخت پارتیکل ایفکت تیر خوردن
			//physics parameter
			EPhysicalSurface SurfaceType=UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

			UParticleSystem* SelectedEffect = nullptr;
			switch (SurfaceType)
			{
			case Surface_FleshDefault:

			case Surface_FleshVulnarble:
				SelectedEffect = FleshImpactEffect;		
				break;
			default:
				SelectedEffect = DefaultImpactEffect;
				break;
			}
			if (SelectedEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
			TraceEndPoint = Hit.ImpactPoint;

		}
		if (DebugWeaponDrawing>0) {
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, true, 1.0f, 0, 1.f);

		}
		PlayFireEffect(TraceEndPoint);
		
	}

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
	APawn* MyyOwner = Cast<APawn>(GetOwner());
	if (MyyOwner) {
		APlayerController* PC = Cast<APlayerController>(MyyOwner->GetController());
		if (PC) {
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

