// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MotionControllerComponent.h"
#include "HandController.generated.h"

UCLASS()
class OQ_UDEMY_AE_API AHandController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHandController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetHand(EControllerHand hand) { MotionController->SetTrackingSource(hand); }

private: 
	// set based on the following signature: FActorBeginOverlapSignature, AActor, OnActorBeginOverlap, AActor*, OverlappedActor, AActor*, OtherActor
	UFUNCTION()
	void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
	void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	bool CanClimb() const; // doesn' need to mutate the state of the actor, so we put const on it

private: 

	UPROPERTY(VisibleAnywhere)
	UMotionControllerComponent *MotionController;

	// state
	bool bCanClimb = false;

private: 
	UPROPERTY(EditAnywhere)
	class UHapticFeedbackEffect_Base *HFERumble;


	

};
