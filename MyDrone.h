// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MyDrone.generated.h"

UCLASS()
class DRONEFLY_API AMyDrone : public AActor
{
	GENERATED_BODY()

	// here we stock each position component of the experiment
	FRichCurve* curveX;
	FRichCurve* curveY;
	FRichCurve* curveZ;

	// here we stock each quaternion component of the experiment
	// those are in my notation/conventions!!
	FRichCurve* curveQ0;
	FRichCurve* curveQ1;
	FRichCurve* curveQ2;
	FRichCurve* curveQ3;

	// running time of the actor
	float RunningTime;

	// initial location of the actor
	FVector InitialLocation;

	// quaternion of actual attitude of the actor
	FQuat NewRotation;

public:	
	// Sets default values for this actor's properties
	AMyDrone();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	
};
