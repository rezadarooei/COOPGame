// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("Component");
	RootComponent = MeshComp;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASWeapon::Fire()
{
	//trace the world from pawn eye to crosshair location
	//Hold Data,It is struct
	AActor* MyOwner = GetOwner();
	if (MyOwner) {
		FHitResult Hit;
		FVector EyeLocation;
		FRotator EyeRotation;
		FVector ShotDirection = EyeRotation.Vector;
		FVector TraceEnd = EyeLocation + (ShotDirection) * 10000;
		MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
		FCollisionQueryParams QueryPrams;
		QueryPrams.AddIgnoredActor(MyOwner);
		QueryPrams.AddIgnoredActor(this);
		QueryPrams.bTraceComplex = true;
		if (GetWorld()->LineTraceSingleByChannel(Hit,EyeLocation,TraceEnd,ECollisionChannel::ECC_Visibility)) {
			//Blocking Hit Process damage
			AActor* HitActor = Hit.GetActor();
			UGameplayStatics::ApplyPointDamage(HitActor, 20.f, ShotDirection, Hit, MyOwner->GetInstigator(),this,DamageType );
		}
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, FColor::White, false, 1.0f, 0, 1.f);
	}

}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

