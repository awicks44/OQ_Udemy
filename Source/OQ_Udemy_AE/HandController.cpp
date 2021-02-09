// Fill out your copyright notice in the Description page of Project Settings.


#include "HandController.h"
#include "MotionControllerComponent.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

// Sets default values
AHandController::AHandController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	
	SetRootComponent(MotionController);
	// not sure if I need
	/*Controller->bDisplayDeviceModel = true;	
	Controller->SetTrackingSource(EControllerHand::Left);*/
}

// Called when the game starts or when spawned
void AHandController::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &AHandController::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AHandController::ActorEndOverlap);
	
}

// Called every frame
void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHandController::ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	bCanClimb = CanClimb();
}

void AHandController::ActorBeginOverlap(AActor * OverlappedActor, AActor * OtherActor)
{
	bool bNewCanClimb = CanClimb();
	if (!bCanClimb && bNewCanClimb)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Can climb"));
		UE_LOG(LogTemp, Warning, TEXT("Can Climb"));
	}

	bCanClimb = bNewCanClimb;
}

bool AHandController::CanClimb() const
{
	TArray<AActor*> actors;

	GetOverlappingActors(actors);

	for (AActor * overlappingActor : actors)
	{
		if (overlappingActor->ActorHasTag(TEXT("Climbable")))
		{
			return true;
		}
	}	

	return false;
}

