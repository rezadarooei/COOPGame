// Fill out your copyright notice in the Description page of Project Settings.

#include "SHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{
	DefualtHealth = 100.f;
	SetIsReplicated(true);
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	//Only Runs when In the server
	if (GetOwnerRole() == ROLE_Authority) {
		AActor* Owner = GetOwner();
		if (Owner) {
			Owner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HanldeTakeAnyDamage);
		}
		
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
	OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);
}
//It shows how it must be replicated and it doesn't need defines in .H file
void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//before that we told in current Health in h files we want to replicated
	DOREPLIFETIME(USHealthComponent,Health);
}