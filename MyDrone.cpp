// Fill out your copyright notice in the Description page of Project Settings.

#include "DroneFly.h"
#include "MyDrone.h"

// Sets default values
AMyDrone::AMyDrone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// inyit cvs table handlers
	UCurveTable* expTable;
	static ConstructorHelpers::FObjectFinder<UCurveTable>
		expTable_BP(TEXT("CurveTable'/Game/Data/testtable.testtable'"));
	expTable = expTable_BP.Object;

	// not sure what this is, but it seems necessary
	static const FString ContextString(TEXT("GENERAL"));
	
	// here we link curves to experimental position CSV data
	curveX = expTable->FindCurve( *FString::Printf(TEXT("X")), ContextString );
	curveY = expTable->FindCurve( *FString::Printf(TEXT("Y")), ContextString );
	curveZ = expTable->FindCurve( *FString::Printf(TEXT("Z")), ContextString );

	// here we link curves to experimental quaternion CSV data
	curveQ0 = expTable->FindCurve(*FString::Printf(TEXT("Q0")), ContextString);
	curveQ1 = expTable->FindCurve(*FString::Printf(TEXT("Q1")), ContextString);
	curveQ2 = expTable->FindCurve(*FString::Printf(TEXT("Q2")), ContextString);
	curveQ3 = expTable->FindCurve(*FString::Printf(TEXT("Q3")), ContextString);

}

// Called when the game starts or when spawned
void AMyDrone::BeginPlay()
{
	Super::BeginPlay();

	// initialize position
	InitialLocation = GetActorLocation();

	// initialize rotation quaternion
	NewRotation.W = 1;
	NewRotation.X = 0;
	NewRotation.Y = 0;
	NewRotation.Z = 0;
	
}

// Called every frame
void AMyDrone::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	FVector NewLocation;

	// we update position of the actor (with [m] to [cm] transform!)
	float x = 100 * curveX->Eval(RunningTime);
	float y = 100 * curveY->Eval(RunningTime);
	float z = 100 * curveZ->Eval(RunningTime);
	NewLocation.X = InitialLocation.X + x;
	NewLocation.Y = InitialLocation.Y + y;
	NewLocation.Z = InitialLocation.Z + z;
	SetActorLocation(NewLocation);

	// we update attitude of the actor
	// notice that the quaternion convention of UE is inverted!
	// in my favorite description: q = ( W -X -Y -Z )
	float q0 = curveQ0->Eval(RunningTime);
	float q1 = curveQ1->Eval(RunningTime);
	float q2 = curveQ2->Eval(RunningTime);
	float q3 = curveQ3->Eval(RunningTime);
	NewRotation.W = q0;
	NewRotation.X = -q1;
	NewRotation.Y = -q2;
	NewRotation.Z = q3;
	//float theta = 3.14159 / 4;
	//NewRotation.W = FMath::Cos(theta/2);
	//NewRotation.X = -FMath::Sin(theta/2)*0;
	//NewRotation.Y = -FMath::Sin(theta/2)*FMath::Cos(theta / 2);
	//NewRotation.Z = -FMath::Sin(theta/2)*FMath::Sin(theta / 2);
	SetActorRotation(NewRotation);

	// here we update running time of actor
	RunningTime += DeltaTime;

}

