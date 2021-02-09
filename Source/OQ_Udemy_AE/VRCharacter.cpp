// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MotionControllerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
//#include "Camera/PlayerCameraManager.h"
//#include "GameFramework/PlayerController.h"
//#include "DrawDebugHelpers.h"
//#include "Math/Color.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"


// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;	
	
	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetupAttachment(VRRoot);
	LeftController->bDisplayDeviceModel = true;
	LeftController->SetTrackingSource(EControllerHand::Left);	

	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	RightController->SetupAttachment(VRRoot);
	RightController->bDisplayDeviceModel = true;
	RightController->SetTrackingSource(EControllerHand::Right);

	TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPath"));
	TeleportPath->SetupAttachment(RightController);
	
	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));	
	DestinationMarker->SetupAttachment(GetRootComponent()); // doesn't matter if we attach this to the root because we will update the position every frame anyways

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));	
	PostProcessComponent->SetupAttachment(GetRootComponent());		// doesn't matter where we attach it
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);

	// just in case designer forgets to set the base material
	if (BlinkerMaterialBase != nullptr)
	{
		BlinkerMaterialInst = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);

		//apply the material to the post process conmponent on the actor
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInst);	
	}
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
	
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("FREE MOUTHGUARD -> URL: %s"), *(Request->GetURL())));

	// determine how much camera moves [new/current location minus old locatoin]
	// camera moves before the character so the character is the old location
	FVector newCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	
	// make sure capsule componnet doens't move up or down
	newCameraOffset.Z = 0;
	
	// move the actor that much
	AddActorWorldOffset(newCameraOffset);
	
	// move the VRRoot so that camera doesn't move
	VRRoot->AddWorldOffset(-newCameraOffset);

	UpdateDestinationMarker();

	UpdateBlinkers();

	
	
}

bool AVRCharacter::FindTeleportDestination(TArray<FVector> &outPath, FVector &outLocation)
{
	FVector start = RightController->GetComponentLocation(); // where eyes will be 
	FVector look = RightController->GetForwardVector();

	// no longer need to calculate our end anymore
	

	FPredictProjectilePathParams pathParams(
		TeleportProjectileRadius,
		start,
		look * TeleportProjectileSpeed,
		TeleportSimulationTime,
		ECollisionChannel::ECC_Visibility,
		this);

	//pathParams.DrawDebugType = EDrawDebugTrace::ForOneFrame;
	pathParams.bTraceComplex = true;

	FPredictProjectilePathResult pathResult;

	bool bHit = UGameplayStatics::PredictProjectilePath(this, pathParams, pathResult);	

	if (!bHit) return false;

	for (FPredictProjectilePathPointData pointOnPath : pathResult.PathData)
	{
		FVector position;
		outPath.Add(pointOnPath.Location);
	}

	FNavLocation navLocation;
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(pathResult.HitResult.Location, navLocation, TeleportProjectionExtent);

	if (!bOnNavMesh) return false;

	outLocation = navLocation.Location;

	return true;
}

void AVRCharacter::UpdateDestinationMarker()
{
	FVector location;
	TArray<FVector> path;
	bool bHasDestination = FindTeleportDestination(path, location);
	// if we hit something && are on the navMesh
	if (bHasDestination)
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(location); // used to pass in the hitResult.Location before checking to hit the NavMesh

		DrawTeleportPath(path);
	}
	else
	{
		DestinationMarker->SetVisibility(false);

		// if we don't have a destination, we shouldn't draw anything to the screen. Don't show a path.
		TArray<FVector> emptyPath;
		DrawTeleportPath(emptyPath);
	}
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Released, this, &AVRCharacter::BeginTeleport);

}

void AVRCharacter::MoveForward(float throttle)
{
	AddMovementInput(throttle * Camera->GetForwardVector());
}

void AVRCharacter::MoveRight(float throttle)
{
	AddMovementInput(throttle * Camera->GetRightVector());
}

void AVRCharacter::BeginTeleport()
{
	
	StartFade(0, 1);
	
	FTimerHandle handle;
	
	GetWorldTimerManager().SetTimer(handle, this, &AVRCharacter::FinishTeleport, TeleportFadeTime);	
}

void AVRCharacter::FinishTeleport()
{
	FVector destination = DestinationMarker->GetComponentLocation();
	destination += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();	
	SetActorLocation(destination);
	
	StartFade(1, 0);
}

void AVRCharacter::UpdateBlinkers()
{
	if (RadiusVsVelocity == nullptr) return;

	float speed = GetVelocity().Size();
	float radius = RadiusVsVelocity->GetFloatValue(speed);

	BlinkerMaterialInst->SetScalarParameterValue(TEXT("Radius"), radius);

	FVector2D center = GetBlinkerCenter();
	BlinkerMaterialInst->SetVectorParameterValue(TEXT("Center"), FLinearColor(center.X, center.Y, 0));
}

void AVRCharacter::DrawTeleportPath(const TArray<FVector>& path)
{
	UpdateSpline(path);	

	for (USplineMeshComponent * splineMesh : TeleportPathMeshPool)
	{
		splineMesh->SetVisibility(false);
	}

	int32 numOfSegments = path.Num() - 1;
	for (int32 i = 0; i < numOfSegments; i++)
	{
		// if the number of objects inm the pool is still less than the number of points in the the path [points in the path will change dynamically the farther this user projects out]
		// if we don't have enough meshes in the pool to match the number of points, we create a new one
		if (TeleportPathMeshPool.Num() <= i) 
		{
			USplineMeshComponent * SplineMesh = NewObject<USplineMeshComponent>(this);			
			SplineMesh->SetMobility(EComponentMobility::Movable);
			// previously attaching to VR root. Will not attached to VRRoot since we are getting positions that are in local space to the TeleportPath
			// we want the local space to be the same for the spline mesh so we can do that easily by attaching the child spline mesh to the TeleportPath
			SplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(TeleportArchMesh);			
			SplineMesh->SetMaterial(0, TeleportArchMaterial);						
			SplineMesh->RegisterComponent();

			TeleportPathMeshPool.Add(SplineMesh);
		}

		// then we grab out one of the meshes in the pool and set it to the locatino of the path
		// we are ensured that we have a mesh at 'i' because of the if statement
		USplineMeshComponent *SplineMesh = TeleportPathMeshPool[i];
		SplineMesh->SetVisibility(true);

		FVector startLocation, startTangent;
		FVector endLocation, endTangent;

		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, startLocation, startTangent);
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i + 1, endLocation, endTangent);

		SplineMesh->SetStartAndEnd(startLocation, startTangent, endLocation, endTangent);
		// was previously setting the world location of the spline mesh to match that of the path point locations. Won't do that anymore. In turn, the spline mesh location will stay at (0, 0, 0)
		
	}
}


void AVRCharacter::UpdateSpline(const TArray<FVector>& path)
{

	TeleportPath->ClearSplinePoints(false);
	for (int32 i = 0; i < path.Num(); i++)
	{
		FVector localPosition =  TeleportPath->GetComponentTransform().InverseTransformPosition(path[i]);
		FSplinePoint point = FSplinePoint(i, localPosition, ESplinePointType::Curve);
		TeleportPath->AddPoint(point, false);
	}
	TeleportPath->UpdateSpline();
}

FVector2D AVRCharacter::GetBlinkerCenter()
{
	// get direction of movementy
	FVector movementDirection = GetVelocity().GetSafeNormal();
	// if we aren't moving really, keep the center of the screen in the middle of the viewport
	if (movementDirection.IsNearlyZero())
		return FVector2D(.5f, .5f);

	FVector worldStationaryLocation;
	// if the movement direction is moving in front, project it positive
	if (FVector::DotProduct(Camera->GetForwardVector(), movementDirection) > 0)
	{
		// multiplying by 1000 puts the location at least 10 meters in front of us
		worldStationaryLocation = Camera->GetComponentLocation() + movementDirection * 1000;
	}
	else
	{
		worldStationaryLocation = Camera->GetComponentLocation() - movementDirection * 1000;
	}

	APlayerController* controller = Cast<APlayerController>(GetController());
	
	if (controller == nullptr)
		return FVector2D(.5f, .5f);

	FVector2D screenLocation;
	// returned boolean will be false if the stationaory location is off the screen. We don't mind that
	controller->ProjectWorldLocationToScreen(worldStationaryLocation, screenLocation);
	// need to get the screen size		
	int32 sizeX, sizeY;
	controller->GetViewportSize(sizeX, sizeY);
	// convert our pixels to UVs
	screenLocation.X /= sizeX;
	screenLocation.Y /= sizeY;	
	
	return screenLocation;
}

void AVRCharacter::StartFade(float fromAlpha, float toAlpha)
{
	// get controller - returns the controller for the current actor
	APlayerController * controller = Cast<APlayerController>(GetController());

	if (controller != nullptr)
	{
		controller->PlayerCameraManager->StartCameraFade(fromAlpha, toAlpha, TeleportFadeTime, FLinearColor::Black);
	}
}



