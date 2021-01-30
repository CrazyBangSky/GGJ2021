// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "CinematicCamera/Public/CineCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BalloonPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBalloonAirDrained);

UCLASS()
class BALLOON_API ABalloonPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABalloonPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void OnLeftStickX(float Rate);

	void OnLeftStickY(float Rate);

	void OnThrottleStart();

	void OnThrottleStop();

	//Apply heat effect on balloon(Increase or decrease volume)
	UFUNCTION(BlueprintCallable)
	void ApplyHeat(float HeatValue);

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite)
	UStaticMeshComponent* BalloonMesh;

	/** Collision component*/
	UPROPERTY(EditDefaultsOnly)
	UCapsuleComponent* CollisionBody;

	/** Side View Camera*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCineCameraComponent* SideViewCameraComponent;

	/** Camera boom positioning the camera beside the balloon*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Used to determine current orientation of the balloon*/
	UArrowComponent* ArrowComponent;

	/** The absolute down direction of the balloon*/
	FRotator CurrentRotation;

	/** The default value of buoyancy on the balloon*/
	UPROPERTY(EditAnywhere)
	FVector DefaultBuoyancy;

	/** Max angle the balloon can tilts by player input*/
	UPROPERTY(EditAnywhere)
	float MaxAngle;

	/** The min volume of the balloon*/
	UPROPERTY(EditAnywhere)
	float MinVolume;

	/** The max volume of the balloon*/
	UPROPERTY(EditAnywhere)
	float MaxVolume;

	UPROPERTY(EditAnywhere)
	float InputDeadZone;

	/** How much fast the balloon will expand when taking heat*/
	UPROPERTY(EditAnyWhere)
	float HeatExpansionMultiplier;

	/** How mush fast the balloon will shrink when in contact with cold air*/
	UPROPERTY(EditAnywhere)
	float ColdContractionMultiplier;

	/** How much fast the balloon will shrink when releasing air*/
	UPROPERTY(EditAnywhere)
	float AirDrainingMultiplier;

	/** How much fast the balloon will rotate to match the input*/
	UPROPERTY(EditAnywhere)
	float RotationSpeedMultiplier;

	/** How much throttle force on the balloon*/
	UPROPERTY(EditAnywhere)
	float ThrottleForceMultiplier;

	/** Initial impulse after player press throttle*/
	UPROPERTY(EditAnywhere)
	float InitImpulse;

	/** The cool down time*/
	UPROPERTY(EditAnywhere)
	float CoolDownTime;

private:
	FVector2D InputValue;
	bool bIsInThrottle;
	float BalloonVolume;
	bool bHasAvailableAir;

	/** Base on player input The supposed down direction of the balloon*/
	FRotator TargetRotation;

	/** The actual buoyancy on the balloon*/
	FVector Buoyancy;

	/** Whether the impulse cool down is completed*/
	bool bIsCoolDownCompleted;

	/** Cool down timer*/
	float CoolDownTimer;


	void UpdateTargetRotation(float DeltaSeconds);

	void UpdateThrottle(float DeltaSeconds);

	//Apply the impulse at the first frame after player start throttle
	void ApplyInitImpulse();

};
