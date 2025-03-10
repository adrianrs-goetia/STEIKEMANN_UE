// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseClasses/AbstractClasses/AbstractCharacter.h"
#include "../Interfaces/AttackInterface.h"
#include "../Interfaces/GrappleTargetInterface.h"
#include "EnemyClasses_Enums.h"
#include "../WallDetection/WallDetection_EnS.h"
#include "SmallEnemy.generated.h"

DECLARE_DELEGATE(FIncapacitatedLandDelegation)
DECLARE_DELEGATE(FStunnedLandDelegation)
DECLARE_DELEGATE(FIncapacitatedCollision)
DECLARE_DELEGATE_OneParam(FLaunchedLand, const FHitResult& LandHit)

class UTimelineComponent;
class AEnemyAIController;
class UWallDetectionComponent;
class USphereComponent;
class UBoxComponent;

UCLASS()
class STEIKEMANN_UE_API ASmallEnemy : public ABaseCharacter,
	public IAttackInterface,
	public IGameplayTagAssetInterface,
	public IGrappleTargetInterface
{
	GENERATED_BODY()
#pragma region Base
public:
	// Sets default values for this character's properties
	ASmallEnemy();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Actor is "deactivated" when not sufficiently close to the player
	UPROPERTY(BlueprintReadWrite)
		bool bIsActive{ true };

public:	// Components
	// Collision
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		USphereComponent* PlayerPogoDetection{ nullptr };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		UBoxComponent* BoxComp_Chomp;

	// Timelines
	UPROPERTY(BlueprintReadOnly)
		UTimelineComponent* TlComp_Smacked{ nullptr };

	// Particles
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles")
		UNiagaraSystem* NS_Trail{ nullptr };
	UNiagaraComponent* NComp_AirTrailing;

	AEnemyAIController* m_AI{ nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialStart")
		bool bStartStuckToWall{};
	FTimerHandle TH_SpecialStartWorldLocation;
	UPROPERTY(BlueprintReadOnly)
		FVector SpecialStartWorldLocation;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
		bool bPrintDebugStatus{};
#pragma endregion //Base

#pragma region Animation
public:	// Variables
	class UEnemyAnimInstance* m_Anim{ nullptr };

public: // Functions
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Attacked();
	void Anim_Attacked_Pure(FVector direction);

	UFUNCTION(BlueprintImplementableEvent)
		void Anim_CHOMP();
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Startled();
#pragma endregion // Animation

#pragma region SpawnRespawn
	TSharedPtr<SpawnPointData> m_SpawnPointData;
	void SetSpawnPointData(TSharedPtr<SpawnPointData> spawn);
	FVector GetRandomLocationNearSpawn();
#pragma endregion // SpawnRespawn
	FTimerHandle TH_DisabledCollision;
	/* Disables collisions for a period. Calling EnableCollisions() on timer end
	 * TimerHandle TH_DisabledCollision */
	void DisableCollisions(float time);

	/**
	* Timer delegate called on DisableCollisions(float time) to check if the current placement
	* of the actor is valid. Does a sweep check to the actor's surroundings and moves the 
	* actor if necessary. 
	*/
	FTimerHandle TH_EnableCollision_PlacementCheck;
	// This function should be placed in the BaseCharacterClass
	// as it would probably be used by any characterclass in the game
	void ActorInvalidPlacement();	// Unfinished function

#pragma region GameplayTags
	UPROPERTY(BlueprintReadOnly, Category = "GameplayTags")
		FGameplayTagContainer GameplayTags;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; return; }
#pragma endregion //GameplayTags

#pragma region AI
public: // Variables
	UPROPERTY(BlueprintReadOnly)
		APawn* m_SensedPawn{ nullptr };
	UPROPERTY(BlueprintReadOnly)
		bool bSensingPawn{};

	enum EDogType m_DogType;
	void SetDogType(enum EDogType type);
	TSharedPtr<struct EDogPack> m_DogPack;

public: // Functions
	/* Returns true if the player is spotted, false if it spots something else */
	FGameplayTag SensingPawn(APawn* pawn);

	void Alert(const APawn& instigator);
	void SleepingBegin();
	void SleepingEnd();

	/* How far aligned with the upvector and the forward vector will the dog jump.
	 * 1.f is perfectly up, -1.f is down and 0.f is directly forward
	 * 0.707f is roughly 45 degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chomp")
		float AttackJumpAngle{ 0.6f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chomp")
		float AttackJumpStrength{ 1200.f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chomp")
		float PostChomp_StunTime{ 1.5f };
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackInterface|Grapplehook")
		float AttackInterface_LeewayPause_Timedilation{ 0.35f };
	FTimerHandle TH_PostChompStun;
	void AttackJump();
	void CHOMP_Pure();
	void Cancel_CHOMP();
	UFUNCTION()
		void ChompCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	virtual void Activate_AttackCollider() override;
	virtual void Deactivate_AttackCollider() override;

	FVector ChompColliderScale{};
	void Chomp_EnableCollision();
	void Chomp_DisableCollision();

#pragma region ChaseState
	void Bark_Pure();
	UFUNCTION(BlueprintImplementableEvent)
		void Bark_BPEvent();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Chase|Bark")
		float Bark_JumpStrenght{ 700.f };

	void CorgiJump_Pure();
	UFUNCTION(BlueprintImplementableEvent)
		void CorgiJump_BPEvent();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Chase|CorgiJump")
		float CorgiJump_MinJumpStrength{ 1200.f };
#pragma endregion //ChaseState
#pragma endregion // AI

#pragma region States
public:	// STATES
	EEnemyState m_State = EEnemyState::STATE_None;
	EGravityState m_GravityState = EGravityState::Default;
	void SetDefaultState();

	virtual void Landed(const FHitResult& Hit) override;

	void Gravity_Tick();
	void EnableGravity();
	void DisableGravity();

	UFUNCTION(BlueprintCallable)
		void EnableGravity_Force();
	UFUNCTION(BlueprintCallable)
		void DisableGravity_Force();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "States|Incapacitated")
		float Incapacitated_LandedStunTime{ 2.f };

public: // Variables for calling AI
	FIncapacitatedCollision IncapacitatedCollisionDelegate;
	FIncapacitatedLandDelegation IncapacitatedLandDelegation;
	FStunnedLandDelegation Delegate_StunnedLand;
	FTimerHandle TH_RedetermineIncapacitate;

public: // Functinos calling AI controller or Functions AI controller calling 
	void Incapacitate(const EAIIncapacitatedType& IncapacitateType, float Time = -1.f);
	void IncapacitateUndeterminedTime(const EAIIncapacitatedType& IncapacitateType, void (ASmallEnemy::* landingfunction)() = nullptr);
	void Post_IncapacitateDetermineState();
	void RedetermineIncapacitateState();
	bool IsIncapacitated() const;

	bool IsTargetWithinSpawn(const FVector& target, const float& radiusmulti = 1.f) const;

	void SpottingPlayer_Begin();
	void SpottingPlayer_End();

	bool CanAttack_Pawn() const;

private: // Functions Capacitate - Used for IncapacitatedLandingDelegate
	void IncapacitatedLand();
	void StunnedLand();
	void PostChompLand();
	void CollisionDelegate();
	void Capacitate_Grappled();
#pragma endregion //States
public:
	void RotateActorYawToVector(FVector AimVector, float DeltaTime = -1.f);
	virtual bool ShroomBounce(FVector direction, float strength) override;
#pragma region Wall Mechanics
public:
	UWallDetectionComponent* WallDetector{ nullptr };
	EWall m_WallState = EWall::WALL_None;
	FTimerHandle TH_LeavingWall;

public:
	void StickToWall();
	virtual bool IsStuck_Pure() override { return m_State == EEnemyState::STATE_OnWall; }
	void LeaveWall();
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection")
		float WDC_LeavingWallTimer{ 0.5f };


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection")
		float WDC_Capsule_Radius{ 40.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection")
		float WDC_Capsule_Halfheight{ 90.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection")
		float WDC_MinHeight{ 20.f };
private:
	Wall::WallData m_WallData;

#pragma endregion	//Wall Mechanics
#pragma region LaunchedCollision
public:
	FTimerHandle TH_FreezeCollisionLaunchCooldown;
	FTimerHandle TH_CollisionLaunchFreeze;
	FVector CollisionLaunchDirection;
	FVector MeshInitialPosition;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Launched Reflected Collision")
		float LaunchedCollision_FreezeCooldown{ 0.1f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Launched Reflected Collision")
		float LaunchedCollision_FreezeBaseTime{ 0.15f };
	/* CollisionLaunch is strongets when colliding at this velocity and over. 
	 * And the time length is lower the weaker the incomming velocity is. 
	 * The calculation uses this velocity as the dividing factor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Launched Reflected Collision")
		float LaunchedCollision_FreezeVelocityMultiplier{ 1000.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Launched Reflected Collision|Wall")
		float LaunchedCollision_VelocityMultiplier{ 0.8f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Launched Reflected Collision|Ground")
		float LaunchedGroundCollision_VelocityMultiplier{ 0.3f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Launched Reflected Collision|OtherCharacters")
		float LaunchedCharacterCollision_VelocityCap{ 2000.f };

public:
	UTimelineComponent* TlComp_LaunchedCollision;
	UPROPERTY(EditAnywhere)
		UCurveVector* Curve_LaunchedCollisionShake;
	UFUNCTION()
		void Tl_LaunchedCollision_End();
	UFUNCTION()
		void Tl_LaunchedCollisionShake(FVector vector);
public:
	UFUNCTION()
		void OnCapsuleComponentLaunchHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& SweepHit);
	bool CanReflectCollisionLaunch() const;
	void ReflectedCollisionLaunch_PreLaunch(FVector SurfaceNormal, FVector SurfaceLocation, bool bAlwaysFreeze = false);
	void ReflectedCollisionLaunch_Launch();
	void LaunchedLandCollision_PreLaunch(FVector SurfaceNormal, FVector SurfaceLocation);
	void LaunchedLandCollision_Launch();
	FVector GetCollisionDirection(const FVector& SurfaceNormal);
	void GetCollisionTime(float& OUT_Time, float& OUT_Multiplier);

	void DogToDogCollision(const FHitResult& SweepHit, ASmallEnemy* OtherDog);
	void GettingDogCollision(FVector SurfaceNormal, FVector SurfaceLocation);
	bool DogEnvironmentCollision(const FHitResult& SweepHit);

	UFUNCTION(BlueprintCallable)
		void CancelCollisionLaunch();

public:	// Visual effects
	void LC_SpawnEffect_Pure(float Time, float VelocityMultiplier, FVector SurfaceNormal, FVector SurfaceLocation);
	UFUNCTION(BlueprintImplementableEvent)
		void LC_SpawnEffect(float Time, float VelocityMultiplier, FVector SurfaceNormal, FVector SurfaceLocation);
	UFUNCTION(BlueprintCallable)
		void LC_GetEffectLocation(FVector& SurfaceLocation, FVector& SurfaceNormal);

	UFUNCTION(BlueprintImplementableEvent)
		void SpawnStunnedEffect(float lifetime);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void DeleteStunnedEffect();

	UFUNCTION(BlueprintImplementableEvent)
		void Effect_StuckToThornwall_Start();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Effect_StuckToThornwall_End();
#pragma endregion //LaunchedCollision
#pragma region GrappleHooked
public: 
	UPROPERTY(BlueprintReadOnly)
		bool bIsHooked{};
	FTimerHandle TH_UncheckIsHooked;
	bool bCanBeGrappleHooked{ true };
	/* The internal cooldown before enemy can be grapplehooked again */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GrappleHook")
		float GrappleHookedInternalCooldown{ 0.5f };

	FTimerHandle Handle_GrappledCooldown;
	void ResetCanBeGrappleHooked() { bCanBeGrappleHooked = true; }

	/* Time it should take to reach the Grappled Instigator : 2nd method */	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GrappleHook")
		float GrappledLaunchTime{ 1.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GrappleHook")
		float GrappledLaunchTime_CollisionActivation{ 0.1f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GrappleHook")
		float GrappledInstigatorOffset{ 50.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GrappleHook|PulledFree", meta = (UIMin = "0", UIMax = "3"))
		float Grappled_PulledFreeStrengthMultiplier{ 1.5f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GrappleHook|PulledFree")
		float Grappled_PulledFreeNoCollisionTimer{ 0.5f };

public:	/* ----- Grapple Interface ------ */
	virtual void TargetedPure() override;
	virtual void UnTargetedPure() override;
	virtual void InReach_Pure() override;
	virtual void OutofReach_Pure() override;
	virtual void HookedPure() override;
	virtual void HookedPure(const FVector InstigatorLocation, bool OnGround,bool PreAction = false) override;
	virtual void UnHookedPure() override;
	virtual void PullFree_Pure(const FVector InstigatorLocation);
public:
	void PullFree_Launch(const FVector& InstigatorLocation);
	void GrappleLaunchToInstigator(FVector InstigatorLocation, float Time, bool OnGround);
#pragma endregion	//GrappleHooked
#pragma region GettingSmacked
public:
	bool bCanBeSmackAttacked{ true };
	FTimerHandle THandle_GotSmackAttacked{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|SmackAttack")
		float SmackAttack_OnGroundMultiplication{ 0.1f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|SmackAttack")
		float SmackAttack_InternalTimer{ 0.156f };

	/**
	*	Sets a timer before character can be damaged again 
	*	With respect to the specific handle it should use */
	void WaitBeforeNewDamage(FTimerHandle TimerHandle, float Time);	
	
public:	// AttackInterface
	bool CanBeAttacked() override;
	virtual void Do_SmackAttack_Pure(IAttackInterface* OtherInterface, AActor* OtherActor) override;	// Getting SmackAttacked
	virtual void Receive_SmackAttack_Pure(const FVector Direction, const float Strength, const bool bOverrideStrength = false) override;
	bool GetCanBeSmackAttacked() const override { return bCanBeSmackAttacked; }
	void ResetCanBeSmackAttacked() override { bCanBeSmackAttacked = true; }

	virtual void Do_GroundPound_Pure(IAttackInterface* OtherInterface, AActor* OtherActor) override {}
	virtual void Receive_GroundPound_Pure(const FVector& PoundDirection, const float& GP_Strength) override;
	virtual void Receive_LeewayPause_Pure(float Pausetime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|AttackingOtherDogs")
		float ChompOther_Strength{ 1400.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|AttackingOtherDogs")
		float ChompOther_Angle{ 0.5f };
	void ChompingAnotherEnemy(IAttackInterface* OtherInterface, AActor* OtherActor);

public: // Particles for getting smacked
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles|Trail")
		float NS_Trail_SpawnRate{ 50.f };
	float NS_Trail_SpawnRate_Internal{};
	/* Multiplied by velocity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles|Trail")
		float NS_Trail_SpeedMin{ 0.4f };
	/* Multiplied by velocity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles|Trail")
		float NS_Trail_SpeedMax{ 1.0f };
	bool bTrailingParticles{};
	void NS_Start_Trail(FVector direction);
	void NS_Update_Trail(float DeltaTime);
	void NS_Stop_Trail();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particles|Trail")
		UCurveFloat* Curve_NSTrail{ nullptr };
	UFUNCTION()
		void Tl_Smacked(float value);

	void Launched();
	void Launched(FVector direction);
	FLaunchedLand Delegate_LaunchedLand;
	void LandedLaunched(const FHitResult& LandHit);
#pragma endregion	//GettingSmacked
#pragma region Pogo
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Pogo|Collision")
		float PB_SphereRadius{ 90.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Pogo|Collision")
		float PB_SphereRadius_Stuck{ 150.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Pogo")
		float PB_Groundpound_LaunchWallNormal{ 0.2f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Pogo")
		float PB_Groundpound_LaunchStrength{ 1200.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Pogo")
		float PB_Pogo_Passive_Stunduration{ 1.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Pogo")
		float PB_Pogo_Groundpound_Stunduration{ 1.f };
public:
	virtual void Receive_Pogo_GroundPound_Pure() override;
	virtual void IA_Receive_Pogo_Pure() override;
#pragma endregion			//Pogo
#pragma region Bounce
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UBouncyShroomActorComponent* BounceComp;
#pragma endregion // Bounce

	/*  DEBUG  */
public:
	void PrintState();
};
