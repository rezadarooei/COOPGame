﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"


static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw Debug Lines for Weapons"), 
	ECVF_Cheat);
// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//تعریف تفنگ که اسکلت مش هست
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("Component");
	RootComponent = MeshComp;
	//محل در آمدن پارتیکل افکت
	MuzzleScoketName = "MuzzleScoket";
	TracerTargetName = "Target";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
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
			if (ImpactEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
			}
			TraceEndPoint = Hit.ImpactPoint;

		}
		if (DebugWeaponDrawing>0) {
			DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, true, 1.0f, 0, 1.f);

		}
		if (MuzzleEffect) {
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleScoketName);
		}
		//برای پارتیکل دود می باشد
		if (TracerEffect) {

			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleScoketName);
			UParticleSystemComponent* TraccerComp=UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);
			if (TraccerComp) {
				//برای نشان دادن مسیر می باشد
				TraccerComp->SetVectorParameter(TracerTargetName,TraceEndPoint);
			}
		}
	}

}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

