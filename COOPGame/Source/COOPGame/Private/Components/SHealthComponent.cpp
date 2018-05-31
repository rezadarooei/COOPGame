// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"


// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefualtHealth = 100.f;
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner) {
		Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HanldeTakeAnyDamage);
	}
	Health = DefualtHealth;
}

void USHealthComponent::HanldeTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0) {
		return;
	}
	Health = FMath::Clamp(Health-Damage, 0.f, DefualtHealth);
	UE_LOG(LogTemp, Warning, TEXT("Health is : %f"), Health);
}

