// Fill out your copyright notice in the Description page of Project Settings.


#include "MoreMath.h"

FRotator UMoreMath::ClampEachAxis(FRotator InRotator, float MinAngle, float MaxAngle)
{
	MinAngle = FMath::Clamp(MinAngle, -360.0f, 360.0f);
	MaxAngle = FMath::Clamp(MaxAngle, MinAngle, 360.0f);

	return FRotator(FMath::ClampAngle(InRotator.Pitch, MinAngle, MaxAngle), FMath::ClampAngle(InRotator.Yaw, MinAngle, MaxAngle), FMath::ClampAngle(InRotator.Roll, MinAngle, MaxAngle));

}