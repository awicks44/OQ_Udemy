// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

UCLASS()
class OQ_UDEMY_AE_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private: 
	bool FindTeleportDestination(FVector &outLocation);
	void UpdateDestinationMarker();
	void MoveForward(float throttle);
	void MoveRight(float throttle);
	void BeginTeleport();
	void FinishTeleport();
	void UpdateBlinkers();
	FVector2D GetBlinkerCenter();


	void StartFade(float fromAlpha, float toAlpha);

private: 
	//
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent *Camera;
	
	UPROPERTY(VisibleAnywhere)
	class UMotionControllerComponent *LeftController;	
	
	UPROPERTY(VisibleAnywhere)
	class UMotionControllerComponent *RightController;	

	UPROPERTY(VisibleAnywhere)
	class USceneComponent *VRRoot;
	
	// once we add this property, we made a blueprint class based on this class because we were goign to reference an asset
	// easier to reference assets using blueprints. 
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent *DestinationMarker;

	UPROPERTY()
	class UPostProcessComponent *PostProcessComponent;

	UPROPERTY()
	class UMaterialInstanceDynamic * BlinkerMaterialInst;

private: 
	UPROPERTY(EditAnywhere)
	float MaxTeleportDistance = 1000; // 100cm is 1 metter. 1000cm is 10 meters
	
	UPROPERTY(EditAnywhere)
	float TeleportFadeTime = 1; // 100cm is 1 metter. 1000cm is 10 meters

	UPROPERTY(EditAnywhere)
	FVector TeleportProjectionExtent = FVector(100, 100, 100); // 1 meter square

	UPROPERTY(EditAnywhere)
	class UMaterialInterface * BlinkerMaterialBase;

	UPROPERTY(EditAnywhere)
	class UCurveFloat * RadiusVsVelocity; // speed vs time || y vs x || accleration vs time 
};
