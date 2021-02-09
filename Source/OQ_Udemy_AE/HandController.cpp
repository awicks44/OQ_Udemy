// Fill out your copyright notice in the Description page of Project Settings.


#include "HandController.h"
#include "MotionControllerComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AHandController::AHandController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	
	SetRootComponent(MotionController);
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
		UE_LOG(LogTemp, Warning, TEXT("Can Climb"));

		APlayerController *controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);		

		// alterate way of getting the player controller from an actor
		/*AActor *pawn = Cast<APawn>(GetAttachParentActor()); // we know that the parent of this object is a Character (which is a pawn)
		if (pawn != nullptr)
		{
			APlayerController * controller = Cast<APlayerController>(pawn->GetInstigatorController());
			if (controller != nullptr)
			{
				controller->PlayHapticEffect(HFERumble, MotionController->GetTrackingSource());
			}
		}*/
		
		controller->PlayHapticEffect(HFERumble, MotionController->GetTrackingSource());			
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

