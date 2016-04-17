// Fill out your copyright notice in the Description page of Project Settings.

#include "DroneRoom.h"
#include "MyDrone.h"
#include "Engine/CurveTable.h"

// Sets default values
AMyDrone::AMyDrone()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Our root component will be a sphere that reacts to physics
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(20.0f);
	SphereComponent->BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->BodyInstance.SetResponseToAllChannels(ECR_Overlap);
	OnActorBeginOverlap.AddDynamic(this, &AMyDrone::OnOverlapBegin);

	// Create and position a mesh component to fuselage
	UStaticMeshComponent* fuselageMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	fuselageMesh->AttachTo(RootComponent);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> FuselageVisualAsset(TEXT("/Game/unreal_fuselage.unreal_fuselage"));
	if (FuselageVisualAsset.Succeeded())
	{
		fuselageMesh->SetStaticMesh(FuselageVisualAsset.Object);
		fuselageMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); // no translation for now
		fuselageMesh->SetWorldScale3D(FVector(1.0f)); // no scaling for now
	}

	// find elevon in assets library
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ElevonVisualAsset(TEXT("/Game/elevon.elevon"));

	// Create and position a mesh component to elevon 1
	elevon1Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Elevon1Mesh"));
	elevon1Mesh->AttachTo(RootComponent);
	if (ElevonVisualAsset.Succeeded())
	{
		elevon1Mesh->SetStaticMesh(ElevonVisualAsset.Object);
		elevon1Mesh->SetRelativeLocation(FVector(-12.0f, 12.0f, 0.0f)); // no translation for now
		elevon1Mesh->SetWorldScale3D(FVector(1.0f)); // no scaling for now
	}

	// Create and position a mesh component to elevon 2
	elevon2Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Elevon2Mesh"));
	elevon2Mesh->AttachTo(RootComponent);
	if (ElevonVisualAsset.Succeeded())
	{
		elevon2Mesh->SetStaticMesh(ElevonVisualAsset.Object);
		elevon2Mesh->SetRelativeLocation(FVector(-12.0f, -12.0f, 0.0f)); // no translation for now
		elevon2Mesh->SetWorldScale3D(FVector(1.0f)); // no scaling for now
	}

	// Create a particle system that we can activate or deactivate
	DroneParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("MovementParticles"));
	DroneParticleSystem->AttachTo(fuselageMesh);
	DroneParticleSystem->bAutoActivate = false;
	DroneParticleSystem->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ParticleAsset(TEXT("/Game/StarterContent/Particles/P_Fire.P_Fire"));
	if (ParticleAsset.Succeeded())
	{
		DroneParticleSystem->SetTemplate(ParticleAsset.Object);
	}

	// inyit cvs table handlers
	UCurveTable* expTable;
	static ConstructorHelpers::FObjectFinder<UCurveTable>
		expTable_BP(TEXT("CurveTable'/Game/Data/testtable.testtable'"));
	expTable = expTable_BP.Object;

	// not sure what this is, but it seems necessary
	static const FString ContextString(TEXT("GENERAL"));

	// here we link curves to experimental position CSV data
	curveX = expTable->FindCurve(*FString::Printf(TEXT("X")), ContextString);
	curveY = expTable->FindCurve(*FString::Printf(TEXT("Y")), ContextString);
	curveZ = expTable->FindCurve(*FString::Printf(TEXT("Z")), ContextString);

	// here we link curves to experimental quaternion CSV data
	curveQ0 = expTable->FindCurve(*FString::Printf(TEXT("Q0")), ContextString);
	curveQ1 = expTable->FindCurve(*FString::Printf(TEXT("Q1")), ContextString);
	curveQ2 = expTable->FindCurve(*FString::Printf(TEXT("Q2")), ContextString);
	curveQ3 = expTable->FindCurve(*FString::Printf(TEXT("Q3")), ContextString);

	// here we link curves to experimental elevon CSV data
	curveD1 = expTable->FindCurve(*FString::Printf(TEXT("D1")), ContextString);
	curveD2 = expTable->FindCurve(*FString::Printf(TEXT("D2")), ContextString);
	
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
	Super::Tick(DeltaTime);

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

	// update elevon poses
	float d1 = curveD1->Eval(RunningTime);
	float d2 = curveD2->Eval(RunningTime);
	Elevon1Rotation.Pitch = -d1; // sign is opposite to model standards
	Elevon2Rotation.Pitch = -d2;
	elevon1Mesh->SetRelativeRotation(Elevon1Rotation);
	elevon2Mesh->SetRelativeRotation(Elevon2Rotation);

	// here we update running time of actor
	RunningTime += DeltaTime;

}

void AMyDrone::toggleFire()
{
	if (DroneParticleSystem && DroneParticleSystem->Template && !DroneParticleSystem->IsActive() ){
		DroneParticleSystem->ToggleActive();
	}
}

void AMyDrone::OnOverlapBegin(class AActor* OtherActor)
{
	// When we detect that we are overlapping with another actor we create a new component  
	// above us, this is then removed when an actor leaves.  

	// Other Actor is the actor that triggered the event. Check that is not ourself.  
	if (OtherActor && (OtherActor != this))
	{
		toggleFire();
	}
}

