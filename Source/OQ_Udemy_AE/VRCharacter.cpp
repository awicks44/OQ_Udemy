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


	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	// doesn't matter if we attach this to the root because we will update the position every frame anyways
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	// doesn't matter where we attach it
	PostProcessComponent->SetupAttachment(GetRootComponent());		

	//BlinkerMaterialBase->CreateDefaultSubobject<UMaterialInstanceDynamic>(TEXT("BlinkerMaterialInst"));


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

bool AVRCharacter::FindTeleportDestination(FVector &outLocation)
{
	FVector start = RightController->GetComponentLocation(); // where eyes will be 
	FVector look = RightController->GetForwardVector();
	// rotate aroudn a certain axis by a given angle. Right axis of the right controller
	look = look.RotateAngleAxis(30, RightController->GetRightVector());
	FVector end = start + look * MaxTeleportDistance; // current locatio of camera + direction we are looking at * by how far we want to ray trace

	FHitResult hitResult;
	// if we hit something
	bool bHit = GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility);

	if (!bHit) return false;

	FNavLocation navLocation;
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(hitResult.Location, navLocation, TeleportProjectionExtent);

	if (!bOnNavMesh) return false;

	outLocation = navLocation.Location;

	return true;
}

void AVRCharacter::UpdateDestinationMarker()
{
	FVector location;
	bool bHasDestination = FindTeleportDestination(location);
	// if we hit something && are on the navMesh
	if (bHasDestination)
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarker->SetWorldLocation(location); // used to pass in the hitResult.Location before checking to hit the NavMesh
	}
	else
	{
		DestinationMarker->SetVisibility(false);
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
	float actorHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	
	SetActorLocation(DestinationMarker->GetComponentLocation() + actorHeight);
	
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



