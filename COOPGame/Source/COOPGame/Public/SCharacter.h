// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"
class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
UCLASS()
class COOPGAME_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void MoveForword(float value);
	void MoveRight(float value);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraComponent")
	UCameraComponent* CameraComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CameraComponent")
	USpringArmComponent* SpringArm;

	void StartCruch();
	void EndCrouch();
	//Default Fov Set in Begin Play
	float DefaultFov;
	
		UPROPERTY(EditDefaultsOnly, Category = "Player")
		float InterpSpeed;
		UPROPERTY(EditDefaultsOnly, Category = "Player",meta=(ClampMin=0.0,ClampMax=100))
	float ZoomedFov;
	bool bIsWantsZoom;
	void StartZoom();
	void EndZoom();
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarttWeapon;

	
	ASWeapon* CurrentWeapon;
	void fire();
	UPROPERTY(EditDefaultsOnly, Category = "Player")
	FName WeaponScoketAttachName;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual FVector GetPawnViewLocation() const override;
	
	
};
