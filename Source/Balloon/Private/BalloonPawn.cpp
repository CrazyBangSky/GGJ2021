// Fill out your copyright notice in the Description page of Project Settings.


#include "BalloonPawn.h"
#include "MoreMath.h"

// Sets default values
ABalloonPawn::ABalloonPawn()
	: CurrentRotation(FRotator::ZeroRotator)
	, DefaultBuoyancy(FVector::UpVector)
	, MaxAngle(180.0f)
	, MinVolume(20.0f)
	, MaxVolume(120.0f)
	, InputDeadZone(0.1f)
	, HeatExpansionMultiplier(1.0f)
	, ColdContractionMultiplier(1.0f)
	, AirDrainingMultiplier(1.0f)
	, RotationSpeedMultiplier(1.0f)
	, ThrottleForceMultiplier(1.0f)
	, InitImpulse(0)
	, CoolDownTime(2.0f)
	, InputValue(FVector2D(0,0))
	, bIsInThrottle(false)
	, bHasAvailableAir(true)
	, InitalScale(FVector::ZeroVector)
	, bIsCoolDownCompleted(true)
	, CoolDownTimer(0)
	, TargetRotation(FRotator::ZeroRotator)
{ 

 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Default object

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//Static mesh
	BalloonMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BalloonMesh"));
	BalloonMesh->SetupAttachment(RootComponent);

	//Collsiion body
	CollisionBody = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionBody"));
	CollisionBody->SetupAttachment(BalloonMesh);

	
	// Create a camera boom attached to the root (capsule)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(BalloonMesh);
	CameraBoom->SetUsingAbsoluteRotation(true); // Rotation of the character should not affect rotation of boom
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 75.f);
	CameraBoom->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	CameraBoom->SetRelativeLocation(FVector::ZeroVector);

	//Create a camera and attach to boom
	SideViewCameraComponent = CreateDefaultSubobject<UCineCameraComponent>(TEXT("SideViewCamera"));
	SideViewCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	SideViewCameraComponent->bUsePawnControlRotation = false; // We don't want the camera rotate with balloon

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(BalloonMesh);
	
}

// Called when the game starts or when spawned
void ABalloonPawn::BeginPlay()
{
	Super::BeginPlay();

	//Default Balloon volume
	BalloonVolume = (MaxVolume - MinVolume)*0.8f + MinVolume;

	InitalScale = BalloonMesh->GetComponentScale();


}

// Called every frame
void ABalloonPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Apply buoyancy
	BalloonMesh->AddForce(DefaultBuoyancy);

	UpdateTargetRotation(DeltaTime);

	UpdateThrottle(DeltaTime);

	if (!bIsCoolDownCompleted)
	{
		CoolDownTimer += DeltaTime;
		if (CoolDownTimer > CoolDownTime)
		{
			CoolDownTimer = 0;
			bIsCoolDownCompleted = true;
		}
	}

	//Clamp current rotation
	CurrentRotation = BalloonMesh->GetComponentRotation();
	CurrentRotation.Normalize();
	if (FMath::Abs(CurrentRotation.Roll) > MaxAngle)
	{
		CurrentRotation = UMoreMath::ClampEachAxis(CurrentRotation, -MaxAngle, MaxAngle);
		BalloonMesh->SetWorldRotation(CurrentRotation);
		BalloonMesh->SetPhysicsAngularVelocity(FVector::ZeroVector);
	}
}

// Called to bind functionality to input
void ABalloonPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("LeftStickX"), this, &ABalloonPawn::OnLeftStickX);
	PlayerInputComponent->BindAxis(TEXT("LeftStickY"), this, &ABalloonPawn::OnLeftStickY);
	PlayerInputComponent->BindAction(TEXT("Throttle"), EInputEvent::IE_Pressed, this, &ABalloonPawn::OnThrottleStart);
	PlayerInputComponent->BindAction(TEXT("Throttle"), EInputEvent::IE_Released, this, &ABalloonPawn::OnThrottleStop);

}


void ABalloonPawn::OnLeftStickX(float Rate)
{
	InputValue.X = Rate;
}

void ABalloonPawn::OnLeftStickY(float Rate)
{
	InputValue.Y = Rate;
}

void ABalloonPawn::OnThrottleStart()
{
	if (!FMath::IsNearlyEqual(BalloonVolume, MinVolume))
	{
		//Throttle if still has available air

		bIsInThrottle = true;

		if (bIsCoolDownCompleted)
		{
			ApplyInitImpulse();
			bIsCoolDownCompleted = false;
		}
	}
}

void ABalloonPawn::OnThrottleStop()
{
	bIsInThrottle = false;
}

void ABalloonPawn::ResetRotationAndScale()
{
	BalloonMesh->SetWorldRotation(FRotator::ZeroRotator);

	BalloonVolume = (MaxVolume - MinVolume) * 0.8f + MinVolume;
	this->SetActorScale3D(InitalScale);
}

void ABalloonPawn::ApplyHeat(float HeatValue)
{
	if (HeatValue > 0)
	{
		BalloonVolume += HeatValue * HeatExpansionMultiplier;
		BalloonVolume = FMath::Clamp(BalloonVolume, MinVolume, MaxVolume);
	}
	else if (HeatValue < 0)
	{
		BalloonVolume += HeatValue * ColdContractionMultiplier;
		BalloonVolume = FMath::Clamp(BalloonVolume, MinVolume, MaxVolume);
	}

	//Air drained
	if (FMath::IsNearlyEqual(BalloonVolume, MinVolume))
	{
		bIsInThrottle = false;
		AirDrainedDelegate.Broadcast();
	}

	//Update volume
	float DefaultVolume = (MaxVolume - MinVolume)*0.8f + MinVolume;

	this->SetActorScale3D(InitalScale*(BalloonVolume / DefaultVolume));

}

void ABalloonPawn::UpdateTargetRotation(float DeltaSeconds)
{
// 	CurrentRotation = ArrowComponent->GetComponentRotation();
// 	GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Red, FString::Printf(TEXT("CurrentRot: %s Dir: %s"), *(CurrentRotation.ToString()), *(CurrentRotation.Vector().ToString())));
// 
// 	//Convert input value to target orientation of the balloon
// 	FVector TargetDownVector = FVector(0, -InputValue.X, InputValue.Y);
// 	TargetDownVector.Normalize();
// 	GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, FString::Printf(TEXT("TargetDownVec: %s"), *(TargetDownVector.ToString())));
// 
// 	TargetRotation = TargetDownVector.Rotation() - FVector::DownVector.Rotation();
// 	//TargetRotation = UMoreMath::ClampEachAxis(TargetRotation, 0, MaxAngle);
// 	GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, FString::Printf(TEXT("TargetRot: %s, Dir: %s"), *(TargetRotation.ToString()), *(TargetRotation.Vector().ToString())));
// 
// 	//Rotation needed to be made
// 	FRotator NeededRotation = TargetRotation - CurrentRotation;
// 	NeededRotation.Normalize();
// 	GEngine->AddOnScreenDebugMessage(3, 2.0f, FColor::Red, FString::Printf(TEXT("NeededRot: %s, Dir : %s"), *(NeededRotation.ToString()), *(NeededRotation.Vector().ToString())));
// 
// 	//The Rotation needed to be made in this Tick
// 	FRotator RotationIncrement = FRotator(FMath::Pow(NeededRotation.Pitch, 2.0f), FMath::Pow(NeededRotation.Yaw, 2.0f), FMath::Pow(NeededRotation.Roll, 2.0f)) * (1.0f / FMath::Pow(180.0f, 2.0f));
// 	RotationIncrement *= DeltaSeconds * RotationSpeedMultiplier;
// 	GEngine->AddOnScreenDebugMessage(4, 2.0f, FColor::Red, FString::Printf(TEXT("RotTick: %s"), *(RotationIncrement.ToString()), *(RotationIncrement.Vector().ToString())));
	
	//TODO : I need to learn 3D math when I'm free

	//Calculate the angle between down vec and target vec in player perspective
	//Persume that X axis points to right side, Y points to the up side
	FVector2D DownVec(0, -1);
	FVector2D TargetVec(InputValue.X, InputValue.Y);

	if (FMath::Sqrt(FMath::Pow(InputValue.X, 2.0f)+ FMath::Pow(InputValue.Y, 2.0f))<InputDeadZone)
	{
		return;
	}

	TargetVec.Normalize();
	
	float TargetAngle = FMath::RadiansToDegrees(FMath::Acos(FVector2D::DotProduct(DownVec, TargetVec)));

	GEngine->AddOnScreenDebugMessage(4, 2.0f, FColor::Red, FString::Printf(TEXT("Target : %f"),TargetAngle));
	
	//If Target vec points to right side, rotate clock-wise
	if (InputValue.X < 0.0f)
	{
		TargetAngle = -TargetAngle;
	}
	

	TargetAngle = FMath::Clamp(TargetAngle, -MaxAngle, MaxAngle);

	float CurrentRotationAngle = FRotator::NormalizeAxis(BalloonMesh->GetComponentRotation().Roll);

	GEngine->AddOnScreenDebugMessage(0, 2.0f, FColor::Red, FString::Printf(TEXT("Current Rotation : %f"), CurrentRotationAngle));

	//The angle between target rotation and current rotation
	float NeededRotationAngle = TargetAngle - CurrentRotationAngle;
	NeededRotationAngle = FRotator::NormalizeAxis(NeededRotationAngle);
	
	GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Red, FString::Printf(TEXT("Needed Rotation : %f"), NeededRotationAngle));

	//The angle of the rotation needed to be made in this tick
	float AngleIncrement = FMath::Sign(NeededRotationAngle)* FMath::Pow(NeededRotationAngle, 2.0f) / FMath::Pow(180.0f, 2.0f);
	AngleIncrement *= DeltaSeconds * RotationSpeedMultiplier;
	FRotator RotationIncrement = FRotator(0, 0, AngleIncrement);

	GEngine->AddOnScreenDebugMessage(2, 2.0f, FColor::Red, FString::Printf(TEXT("Increment : %f"), AngleIncrement));
	
	//Update Rotation
	BalloonMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	BalloonMesh->AddWorldRotation(RotationIncrement, true, nullptr, ETeleportType::TeleportPhysics);

	return;

}

void ABalloonPawn::UpdateThrottle(float DeltaSeconds)
{
	if (!bIsInThrottle)
	{
		return;
	}

	//Get the vector points from balloon center to the balloon bottom.
	CurrentRotation = ArrowComponent->GetComponentRotation();
	FVector DownVector = CurrentRotation.RotateVector(FVector::DownVector);

	//The direction of the throttle force should be the oppsite of the down vec
	FVector Force = -DownVector;
	Force.Normalize();

	//The balloon shrinks by losing the air
	BalloonVolume -= DeltaSeconds * AirDrainingMultiplier;
	BalloonVolume = FMath::Clamp(BalloonVolume, MinVolume, MaxVolume);
	if (FMath::IsNearlyEqual(BalloonVolume, MinVolume))
	{
		bIsInThrottle = false;
		AirDrainedDelegate.Broadcast();
	}

	float DefaultVolume = (MaxVolume - MinVolume)*0.8f + MinVolume;

	this->SetActorScale3D(InitalScale*(BalloonVolume / DefaultVolume));

	BalloonMesh->AddForce(Force*ThrottleForceMultiplier);

	return;
}

void ABalloonPawn::ApplyInitImpulse()
{
	//Get the vector points from balloon center to the balloon bottom.
	CurrentRotation = ArrowComponent->GetComponentRotation();
	FVector DownVector = CurrentRotation.RotateVector(FVector::DownVector);

	//The direction of the throttle force should be the oppsite of the down vec
	FVector Impulse = -DownVector;
	Impulse.Normalize();
	Impulse *= InitImpulse;

	//Apply impulse
	BalloonMesh->AddImpulse(Impulse);
}