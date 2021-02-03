// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetMathLibrary.h"
#include "MoreMath.generated.h"

/**
 * 
 */
UCLASS()
class BALLOON_API UMoreMath : public UKismetMathLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category= "Math")
	static FRotator ClampEachAxis(FRotator InRotator, float MinAngle, float MaxAngle);
	
};
