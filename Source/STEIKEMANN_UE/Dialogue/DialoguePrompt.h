// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialoguePrompt.generated.h"

UENUM(BlueprintType)
enum class ECameraLerp : uint8
{
	None,
	First,
	Second
};

/* Forward Declarations */
class UCameraComponent;
class UBoxComponent;
class ASteikemannCharacter;
class UArrowComponent;

UCLASS()
class STEIKEMANN_UE_API ADialoguePrompt : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USceneComponent* Root;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		USceneComponent* Prompt;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UBoxComponent* Volume;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UArrowComponent* PlayerTransform;
public:
	// Sets default values for this actor's properties
	ADialoguePrompt();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void SceneComponentLookAt(USceneComponent* Comp, USceneComponent* Target);
	UFUNCTION(BlueprintCallable)
		float LerpSceneComponentTransformToSceneComponent(USceneComponent* Comp, USceneComponent* TargetComp, float LerpAlpha, float DeltaTime);

public:
	UPROPERTY(BlueprintReadOnly)
		ASteikemannCharacter* m_Player{ nullptr };
	UPROPERTY(BlueprintReadOnly)
		bool bPlayerWithinVolume{};
	UPROPERTY(BlueprintReadOnly)
		UCameraComponent* m_PlayerCamera{ nullptr };

	UPROPERTY(BlueprintReadOnly)
		float CameraLerpSpeed{ 1.f };
	UPROPERTY(BlueprintReadWrite)
		int m_PromptIndex_Internal{0};

public:	// Functions for activating prompt
	UFUNCTION()
		void OnVolumeBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintImplementableEvent)
		void ShowPrompt();
	UFUNCTION(BlueprintImplementableEvent)
		void EndPrompt();

	/* Common changes when going from one prompt index to another */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void PromptChange();
	void PromptChange_Pure();
public:	// Functions called by player
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void GetNextPromptState(ASteikemannCharacter* player, int promptIndex);
	void GetNextPromptState_Pure(ASteikemannCharacter* player, int promptIndex = -1);


	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void ExitPrompt(float ExitTime);

	UFUNCTION(BlueprintCallable)
		FTransform GetPlayerPromptTransform() const;
};

