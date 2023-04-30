// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../STEIKEMANN_UE.h"

#include "BaseStaticActor.generated.h"

UCLASS()
class STEIKEMANN_UE_API ABaseStaticActor : public AActor,
	public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseStaticActor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Root")
		USceneComponent* Root{ nullptr };

	UPROPERTY(BlueprintReadWrite, Category = "GameplayTags")
		FGameplayTagContainer GTagContainer;
	void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GTagContainer; }

	FTimerManager TimerManager;

	/**
	* skriv en funksjon som lar actor sjekke opp mot SteikeWorldStatics::PlayerLocation for � se om den skal gj�re noe n�r spilleren er n�rme nok
	*/


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};
