// Fill out your copyright notice in the Description page of Project Settings.


#include "HandController.h"
#include "MotionControllerComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	if (bIsClimbing)
	{
		// we have the current position of the hand controller and the current position of the hand controller 
		// the difference between those 2 will give us how far the actor has moved
		// gives us the vector that goes from the climb start location to the get actor's location
		FVector handControllerDelta = GetActorLocation() - ClimbingStartLocation;
		// we watn to move the pawn in the opposite direction to negate that motion and bring the the hand controller back to the climbStartLocation
		// we don't need to cast because we can move the transform of whatever the actor is
		GetAttachParentActor()->AddActorWorldOffset(-handControllerDelta);
	}	
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

void AHandController::Grip()
{
	if (!bCanClimb)
		return;

	if (!bIsClimbing)
	{
		
		OtherController->bIsClimbing = false;
		
		bIsClimbing = true;
		// this locaiton will be somwhere off in the distance
		ClimbingStartLocation = GetActorLocation();

		ACharacter *character = Cast<ACharacter>(GetAttachParentActor());

		if (character != nullptr)
		{
			character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		}
		
	}

	UE_LOG(LogTemp, Warning, TEXT("Pressing"));
}

void AHandController::Release()
{
	if (bIsClimbing)
	{
		bIsClimbing = false;
		
		ACharacter *character = Cast<ACharacter>(GetAttachParentActor());

		if (character != nullptr)
		{
			character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
	

	UE_LOG(LogTemp, Warning, TEXT("Releasing"));
}

void AHandController::PairController(AHandController* controller)
{
	OtherController = controller;
	// our other controller's, 'other' controller will be us
	OtherController->OtherController = this;
}

