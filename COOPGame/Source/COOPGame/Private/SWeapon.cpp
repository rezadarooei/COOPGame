// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Particles/ParticleSystem.h"
// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("Component");
	RootComponent = MeshComp;
	MuzzleScoketName = "MuzzleScoket";
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASWeapon::Fire()
{
	//trace the world from pawn eye to crosshair location
	AActor* MyOwner = GetOwner();
	if (MyOwner) {
		FHitResult Hit;	//Hold Data,It is struct

		FVector EyeLocation;
		FRotator EyeRotation;
		FVector ShotDirection = EyeRotation.Vector();
		FVector TraceEnd = EyeLocation + (ShotDirection) * 10000; 

		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		
		FCollisionQueryParams QueryPrams;
		QueryPrams.AddIgnoredActor(MyOwner);
		QueryPrams.AddIgnoredActor(this);
		QueryPrams.bTraceComplex = true;

		if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd,ECC_Visibility,QueryPrams)) {
			//Blocking Hit Process damage
			AActor* HitActor = Hit.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, DamageType);
			UE_LOG(LogTemp, Warning, TEXT("It is work2"));

			if (ImpactEffect) {
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
				UE_LOG(LogTemp, Warning, TEXT("It is work1"));
			}
		}
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::Red, true, 1.0f, 0, 1.f);
		if (MuzzleEffect) {
			UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleScoketName);
		}
	}

}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

