// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HandController.h"
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
	bool FindTeleportDestination(TArray<FVector> &outPath, FVector &outLocation);
	void UpdateDestinationMarker();
	void MoveForward(float throttle);
	void MoveRight(float throttle);
	void GripLeft() { LController->Grip(); }
	void ReleaseLeft() { LController->Release(); }
	void GripRight() { RController->Grip(); }
	void ReleaseRight() { RController->Release(); }
	void BeginTeleport();
	void FinishTeleport();
	void UpdateBlinkers();
	void DrawTeleportPath(const TArray<FVector> &path);
	void UpdateSpline(const TArray<FVector> &path); // adding const here ensures that we don't make copies of this variable
	FVector2D GetBlinkerCenter();


	void StartFade(float fromAlpha, float toAlpha);

private: 
	//
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent *Camera;
	
	UPROPERTY(VisibleAnywhere)
	AHandController *LController;

	UPROPERTY(VisibleAnywhere)
	AHandController *RController;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent *VRRoot;

	UPROPERTY(VisibleAnywhere) // want to change it's properties in blueprint
	class USplineComponent * TeleportPath;
	
	// once we add this property, we made a blueprint class based on this class because we were goign to reference an asset
	// easier to reference assets using blueprints. 
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent *DestinationMarker;

	UPROPERTY()
	class UPostProcessComponent *PostProcessComponent;

	UPROPERTY()
	class UMaterialInstanceDynamic * BlinkerMaterialInst;

	UPROPERTY()
	TArray<class USplineMeshComponent*>  TeleportPathMeshPool;



private: // configuration parameters
	UPROPERTY(EditAnywhere)
	float TeleportProjectileSpeed = 800; // 100cm is 1 metter. 1000cm is 10 meters

	UPROPERTY(EditAnywhere)
	float TeleportSimulationTime = 1; // affects the length of the trace

	UPROPERTY(EditAnywhere)
	float TeleportProjectileRadius = 10; 
	
	UPROPERTY(EditAnywhere)
	float TeleportFadeTime = 1; // 100cm is 1 metter. 1000cm is 10 meters

	UPROPERTY(EditAnywhere)
	FVector TeleportProjectionExtent = FVector(100, 100, 100); // 1 meter square

	UPROPERTY(EditAnywhere)
	class UMaterialInterface * BlinkerMaterialBase;

	UPROPERTY(EditAnywhere)
	class UCurveFloat * RadiusVsVelocity; // speed vs time || y vs x || accleration vs time 

	UPROPERTY(EditDefaultsOnly) // [EditDefaultsOnly] we only want to edit this in the blueprint. We don't want the leve to overwrite it
	class UStaticMesh *TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly) // [EditDefaultsOnly] we only want to edit this in the blueprint. We don't want the leve to overwrite it
	class UMaterialInterface *TeleportArchMaterial;	

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AHandController> HandControllerClass;
};
