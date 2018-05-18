// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"
class USkeletalMeshComponent;
UCLASS()
class COOPGAME_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Component")
	USkeletalMeshComponent* MeshComp;
	UFUNCTION(BlueprintCallable,Category="Weapon")
	void Fire();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
