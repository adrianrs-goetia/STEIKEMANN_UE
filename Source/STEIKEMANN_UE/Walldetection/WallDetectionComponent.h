// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallDetection_EnS.h"
#include "../STEIKEMANN_UE.h"
#include "WallDetectionComponent.generated.h"

#define ECC_StickyWall ECC_GameTraceChannel4

UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent) )
class STEIKEMANN_UE_API UWallDetectionComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:	
	// Sets default values for this component's properties

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void SetCapsuleSize(float radius, float halfheight) { m_capsule.SetCapsule(radius, halfheight); }
	
	void SetHeight(float minHeight, float playerCapsuleHalfHeight) { m_MinHeight = minHeight, m_OwnerHalfHeight = playerCapsuleHalfHeight; }
	void SetMinLengthToWall(float length) { m_MinLengthToWall = length; }

private:
	//bool bShowDebug{};
	FCollisionShape m_capsule;
	float Angle_UpperLimit{  0.7f };
	float Angle_LowerLimit{ -0.7f };
	float m_MinLengthToWall{ 40.f };

public:	// Wall Detection

	UPROPERTY(EditAnywhere)
		float ActorMiddleOffset{ 10.f };

	bool DetectWall(const AActor* actor, const FVector Location, const FVector ForwardVector, Wall::WallData& walldata, Wall::WallData& WallJumpData);
	bool DetectStickyWall(const AActor* actor, const FVector Location, const FVector Forward, Wall::WallData& walldata, ECollisionChannel TraceChannel);

	/* Checks if the component detects a sticky wall within an angle of a given normal vector */
	bool DetectStickyWallOnNormalWithinAngle(FVector actorlocation, const float dotprodlimit, FVector normal);

private:
	float m_OwnerHalfHeight{};

	float m_MinHeight{ 40.f };	// from root

	bool DetermineValidPoints_IMPL(TArray<FHitResult>& hits, const FVector& Location);
	bool Valid_WallJumpPoints(TArray<FHitResult>& hits, const FVector& Location);
	void GetWallPoint_IMPL(Wall::WallData& data, const TArray<FHitResult>& hits);
	void GetAverageWallLocationAndNormalFromSweep(const TArray<FHitResult>& hits, FVector& location, FVector& normal);

	float GetMinHeight(float z);
	bool ValidLengthToCapsule(FVector Hit, FVector capsuleLocation, float capsuleHeight);

public:	// Ledge detection
	float Ledge_Anglelimit{ 0.8f };

	bool DetectLedge(Wall::LedgeData& ledge, const AActor* actor, const FVector actorLocation, const FVector actorUp, const Wall::WallData& wall, const float height, const float inwardsLength);
	bool DetectLedge_Sweep(Wall::LedgeData& ledge, const AActor* actor, const FVector actorLocation, const FVector actorUp, const float height, const float inwardsLength);

	UPROPERTY(EditAnywhere, Category = "Ledge detection")
		float LD_height{ 100.f };
	UPROPERTY(EditAnywhere, Category = "Ledge detection")
		float LD_inward{ 100.f };
	bool DetectLedge_New(Wall::LedgeData& ledge);

	
};
