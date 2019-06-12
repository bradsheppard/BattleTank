// Fill out your copyright notice in the Description page of Project Settings.

#include "TankPlayerController.h"
#include "TankAimingComponent.h"
#include "Engine/World.h"
#include "Tank.h"

void ATankPlayerController::BeginPlay() {
	Super::BeginPlay();
	if (!GetPawn()) return;
	auto AimingComponent = GetPawn()->FindComponentByClass<UTankAimingComponent>();

	if (ensure(AimingComponent))
		FoundAimingComponent(AimingComponent);
}

void ATankPlayerController::SetPawn(APawn* InPawn) {
        Super::SetPawn(InPawn);
        if (InPawn) {
                auto PossessedTank = Cast<ATank>(InPawn);
                if (!ensure(PossessedTank)) { return; }

                PossessedTank->OnDeath.AddUniqueDynamic(this, &ATankPlayerController::OnPossedTankDeath);
        }
}

void ATankPlayerController::OnPossedTankDeath() {
        StartSpectatingOnly();
};

void ATankPlayerController::AimTowardsCrosshair() {
	if (!GetPawn()) { return; }
	auto AimingComponent = GetPawn()->FindComponentByClass<UTankAimingComponent>();
	if (!ensure(AimingComponent)) { return; }

	FVector HitLocation;
	if (GetSightRayHitLocation(HitLocation)) {
		AimingComponent->AimAt(HitLocation);
	}
}

bool ATankPlayerController::GetSightRayHitLocation(FVector& HitLocation) const {
	int32 ViewportSizeX, ViewportSizeY;
	GetViewportSize(ViewportSizeX, ViewportSizeY);

	auto ScreenLocation = FVector2D(ViewportSizeX * CrosshairXLocation, ViewportSizeY * CrosshairYLocation);

	FVector LookDirection;
	if (GetLookDirection(ScreenLocation, LookDirection)) {
		return GetLookVectorHitLocation(LookDirection, HitLocation);
	}
	return false;
}

bool ATankPlayerController::GetLookVectorHitLocation(FVector LookDirection, FVector& HitLocation) const {
	FHitResult HitResult;
	auto StartLocation = PlayerCameraManager->GetCameraLocation();
	auto EndLocation = StartLocation + LookDirection * LineTraceRange;

	if (GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECollisionChannel::ECC_Camera
	)) {
		HitLocation = HitResult.Location;
		return true;
	}

	HitLocation = FVector(0);
	return false;
}

bool ATankPlayerController::GetLookDirection(FVector2D ScreenLocation, FVector& LookDirection) const {
	FVector CameraWorldLocation;
	return DeprojectScreenPositionToWorld(ScreenLocation.X, ScreenLocation.Y, CameraWorldLocation, LookDirection);
}

void ATankPlayerController::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	AimTowardsCrosshair();
}
