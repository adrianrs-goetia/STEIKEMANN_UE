// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseClasses/AbstractClasses/AbstractCharacter.h"
#include "SteikePlayerController.h"

#include "../Interfaces/GrappleTargetInterface.h"
#include "../Interfaces/AttackInterface.h"
#include "../Interfaces/CameraGuideInterface.h"
#include "Camera/CameraShakeBase.h"
#include "SteikeAnimInstance.h"
#include "GameplayTagAssetInterface.h"
#include "../Walldetection/WallDetection_EnS.h"
#include "../StaticActors/EnS/StaticActors_EnS.h"

#include "SteikemannCharacter.generated.h"

#define GRAPPLE_HOOK ECC_GameTraceChannel1
#define ECC_PogoCollision ECC_GameTraceChannel2

DECLARE_DELEGATE(FAttackActionBuffer)
DECLARE_DELEGATE_OneParam(FPostAttackBuffer, EPostAttackType& PostAttackType);
DECLARE_DELEGATE(FGrappleEnemyLandDelegate)

class USoundBase;
class UWallDetectionComponent;

UENUM()
enum class EMovementInput : int8
{
	Open,
	Locked,
	PeriodLocked
};
UENUM()
enum class EState : int8
{
	STATE_None, // Used when leaving a state and reevaluating the next state

	// Default States
	STATE_OnGround,
	STATE_InAir,
	
	// Advanced States
	STATE_OnWall,
	STATE_Attacking,
	STATE_Grappling
};
UENUM()
enum class EGroundState : int8
{
	GROUND_Walk,
	GROUND_Roll,
	GROUND_Dash
};
UENUM()
enum class EAirState : int8
{
	AIR_None,
	AIR_Freefall,
	AIR_Jump,
	AIR_Pogo
};

UENUM() 
enum class EPogoType : int8
{
	POGO_None,
	POGO_Passive,
	POGO_Active,
	POGO_Groundpound,

	POGO_Leave
};

UENUM()
enum class EGrappleState : int8
{
	None,

	Pre_Launch,
	Post_Launch,
	Leave
};
UENUM()
enum class EGrappleType : int8
{
	None, 

	Static,
	Static_StuckEnemy_Air,
	Static_StuckEnemy_Ground,
	
	Dynamic_Air,
	Dynamic_Ground
};

UENUM()
enum class EAttackState : int8
{
	None,

	Smack,
	GroundPound

	,Post_GroundPound
	,Post_Buffer
};


UENUM()
enum class ESmackAttackType : int8
{
	Regular,
	GrappleSmack
};
UENUM()
enum class EPostAttackType : int8
{
	GrappleSmack
};

UENUM()
enum class EPromptState : int8
{
	None,
	WithingArea,
	InPrompt
};
UENUM(BlueprintType)
enum class ELoseSapType : uint8
{
	PlayerHasNoSaps,
	PlayerHasNotEnough,
	LoseAllSaps
};

UCLASS()
class STEIKEMANN_UE_API ASteikemannCharacter : public ABaseCharacter, 
	public IGrappleTargetInterface,
	public IAttackInterface,
	public IGameplayTagAssetInterface,
	public ICameraGuideInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASteikemannCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class USpringArmComponent* CameraBoom{ nullptr };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class UCameraComponent* Camera{ nullptr };


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UPROPERTY(BlueprintReadOnly)
		ASteikePlayerController* SteikePlayerController{ nullptr };
	UPROPERTY(BlueprintReadWrite)
		EInputType m_EInputType;

	/* The Raw InputVector */
	UPROPERTY(BlueprintReadOnly)
		FVector InputVectorRaw;
	/* Input vector rotated to match the playercontrollers rotation */
	UPROPERTY(BlueprintReadOnly)
		FVector m_InputVector;
	/* Mouse Movement Input */
	UPROPERTY(BlueprintReadOnly)
		FVector m_MouseMovementInput;


	/* Camera input value for gamepad */
	UPROPERTY(BlueprintReadOnly)
		FVector m_GamepadCameraInputRAW;
	UPROPERTY(BlueprintReadOnly)
		FVector m_GamepadCameraInput;

	EMovementInput m_EMovementInputState = EMovementInput::Open;
	UFUNCTION(BlueprintCallable)
		void MovementInput_Lock()	{ m_EMovementInputState = EMovementInput::Locked; }
	UFUNCTION(BlueprintCallable)
		void MovementInput_Open()	{ m_EMovementInputState = EMovementInput::Open; }

	TWeakObjectPtr<class USteikemannCharMovementComponent> MovementComponent;
	TWeakObjectPtr<class USteikemannCharMovementComponent> GetMoveComponent() const { return MovementComponent; }

	USteikeAnimInstance* SteikeAnimInstance{ nullptr };

	void AssignAnimInstance(USteikeAnimInstance* AnimInstance) { SteikeAnimInstance = AnimInstance; }
	USteikeAnimInstance* GetAnimInstance() const { return SteikeAnimInstance; }

	APlayerController* PlayerController{ nullptr };
	APlayerController* GetPlayerController() const { return PlayerController; }

	/* GameplayTags */
	FGameplayTagContainer GameplayTags;
	UFUNCTION(BlueprintCallable, Category = GameplayTags)
		virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override { TagContainer = GameplayTags; return; }

	/* ----------------------- Actor Rotation Functions ---------------------------------- */
	void ResetActorRotationPitchAndRoll(float DeltaTime);
	/* Rotates actors yaw to direction. Vector does not need to be normalized */
	void RotateActorYawToVector(FVector AimVector, float DeltaTime = 0);
	void RotateActorPitchToVector(FVector AimVector, float DeltaTime = 0);
		void RotateActorYawPitchToVector(FVector AimVector, float DeltaTime = 0);	//Old
	void RollActorTowardsLocation(FVector Point, float DeltaTime = 0);

	UPhysicalMaterial* DetectPhysMaterial();

#pragma region Prompt Area
	/**
	* To avoid the player locking the game in case they spam the cancel and activate button
	*/
	FTimerHandle TH_BetweenPromptActivations;
	float Prompt_BetweenPromptActivations_Timer{ 0.8f };
	/* Player within prompt area */
	UPROPERTY(BlueprintReadOnly)
		EPromptState m_PromptState = EPromptState::None;
	class ADialoguePrompt* m_PromptActor{ nullptr };
	UPROPERTY(BlueprintReadWrite)
		bool bInPrompt{};

	UPROPERTY(BlueprintReadOnly)
		bool bCameraLerpBack_PostPrompt{};
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Prompt")
		float CameraLerpSpeed_Prompt{ 2.f };
	float m_CameraLerpAlpha_PostPrompt{};

	void EnterPromptArea(ADialoguePrompt* promptActor, FVector promptLocation);
	void LeavePromptArea();
	void SetPlayerPromptTransform();
	bool ActivatePrompt();
	UFUNCTION(BlueprintCallable)
		bool PlayerExitPrompt();

	UFUNCTION(BlueprintCallable)
		ELoseSapType GetLoseSapType();
	UFUNCTION(BlueprintCallable)
		void LoseSaps(int amount = -1);
	void StartSapLoss();
	void IncrementSapLoss();
	UPROPERTY(BlueprintReadOnly)
		int SapsToBeLost{};
	int CurrentSapsLost{};
	UPROPERTY(EditAnywhere, Category = "HUD")
		float SapLoss_Start_Timer{ 0.7 };
	UPROPERTY(EditAnywhere, Category = "HUD")
		float SapLoss_Start_Update{ 0.1 };
	FTimerHandle TH_SapLoss_Start;
	FTimerHandle TH_SapLoss_Update;
#pragma endregion				// Prompt Area
#pragma region Audio
	UPROPERTY(EditAnywhere, Category = "Audio")
		UAudioComponent* Component_Audio{ nullptr };


#pragma endregion					//Audio
#pragma region Material
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Materialss", meta = (DisplayPriority = "1"))
		class UMaterialParameterCollection* MPC_Player;
	void Material_UpdateParameterCollection_Player(float DeltaTime);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Materialss", meta = (DisplayPriority = "2"))
		UCurveFloat* Curve_DecalAlpha;
#pragma endregion				//Material
#pragma region ParticleEffects

	/* ------------------- Particle Effects ------------------- */
	UPROPERTY(EditAnywhere, Category = "Particle Effects")
		UNiagaraComponent* Component_Niagara{ nullptr };


	#pragma region Landing
		/* ------------------- PE: Landing ------------------- */
		UPROPERTY(EditAnywhere, Category = "Particle Effects|Land")
			UNiagaraSystem* NS_Land{ nullptr };

		UFUNCTION(BlueprintCallable)
			void NS_Land_Implementation(const FHitResult& Hit);

		/* The amount of particles that will spawn determined by the characters landing velocity, times this multiplier */
		UPROPERTY(EditAnywhere, Category = "Particle Effects|Land")
			float NSM_Land_ParticleAmount{ 0.5f };
		/* The speed of the particles will be determined by the characters velocity when landing, times this multiplier */
		UPROPERTY(EditAnywhere, Category = "Particle Effects|Land")
			float NSM_Land_ParticleSpeed{ 0.5f };

		#pragma endregion //Landing
	#pragma region OnWall
		/* ------------------- PE: OnWall ------------------- */
		UPROPERTY(EditAnywhere, Category = "Particle Effects|WallJump")
			UNiagaraSystem* NS_WallSlide{ nullptr };
		/* The amount of particles per second the system should emit */
		UPROPERTY(EditAnywhere, Category = "Particle Effects|WallJump")
			float NS_WallSlide_ParticleAmount{ 1000.f };
	#pragma endregion //OnWall
	#pragma region Attack
	UNiagaraComponent* NiagaraComp_Attack{ nullptr };
	UPROPERTY(EditAnywhere, Category = "Particle Effects|Attack")
		UNiagaraSystem* NS_AttackContact{ nullptr };

	#pragma endregion //Attack
	#pragma region Crouch
		UNiagaraComponent* NiComp_CrouchSlide{ nullptr };

		UPROPERTY(EditAnywhere, Category = "Particle Effects|Crouch")
			UNiagaraSystem* NS_CrouchSlide{ nullptr };

	#pragma endregion //Crouch

#pragma endregion			//ParticleEffects

#pragma region Camera
	/* ------------------- Camera Shakes ------------------- */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UCameraShakeBase> MYShake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CameraShakes|Jump")
		TSubclassOf<UCameraShakeBase> Jump_Land;

	UFUNCTION(BlueprintCallable)
		void PlayCameraShake(TSubclassOf<UCameraShakeBase> shake, float falloff);

	FTransform m_CameraTransform;
	bool LerpCameraBackToBoom(float DeltaTime);

	void GC_SlerpTowardsVector(const FVector& direction, float alpha, float DeltaTime);

	#pragma region CameraBoomPlacement
	/*	*	The placement of the camera boom will lerp towards the root, making the player the centre on the 
		*	screen when they are in the air. Making it easier to platform and see objects beneath the player character */
	// The location of the camera boom, in relation to the character root
	FVector CameraBoom_Location{};
	UPROPERTY(EditAnywhere, Category = "Camera|Boom")
		float CameraBoom_LerpSpeed{ 1.f };
	UPROPERTY(EditAnywhere, Category = "Camera|Boom")
		float CameraBoom_LerpHeight{ 200.f };
	UPROPERTY(EditAnywhere, Category = "Camera|Boom")
		float CameraBoom_MinHeightFromRoot{ 20.f };
	float CameraBoom_PlacementAlpha{};
	void PlaceCameraBoom(float DeltaTime);

	#pragma endregion //CameraBoomPlacement
	#pragma region CameraGuide
	float CameraGuide_Pitch{ 0.f };

	UPROPERTY(EditAnywhere, Category = "Camera|Volume|Pitch", meta = (UIMin = "0", UIMax = "500"))
			float CameraGuide_Pitch_MIN{ 100.f };

	UPROPERTY(EditAnywhere, Category = "Camera|Volume|Pitch", meta = (UIMin = "0", UIMax = "5000"))
			float CameraGuide_Pitch_MAX{ 500.f };

	UPROPERTY(EditAnywhere, Category = "Camera|Volume|Pitch", meta = (UIMin = "0", UIMax = "2"))
			float CameraGuide_ZdiffMultiplier{ 1.f };
			
	/* Maximum distance for pitch adjustment */
	UPROPERTY(EditAnywhere, Category = "Camera|Volume|Pitch", meta = (UIMin = "0", UIMax = "10000"))
			float CameraGuide_Pitch_DistanceMAX{ 2000.f };

	/* Minimum distance for pitch adjustment */
	UPROPERTY(EditAnywhere, Category = "Camera|Volume|Pitch", meta = (UIMin = "0", UIMax = "10000"))
			float CameraGuide_Pitch_DistanceMIN{ 100.f };

	EPointType CurrentCameraGuide;
	EPointType PreviousCameraGuide;
	float Base_CameraBoomLength;
	bool bCamLerpBackToPosition{};

	float CameraGuideAlpha{};

	UPROPERTY(EditAnywhere, Category = "Camera|Default", meta = (UIMin = "0", UIMax = "1"))
		float Default_CameraGuideAlpha{ 0.1f };

	/* ---- GUIDING CAMERA -----
	* Every type of automatic camera guide interaction within one function
	* Volume
	* Automatic during movement
	* */
	virtual void GuideCamera(float DeltaTime) override;
	float Internal_SplineInputkey{};
	UPROPERTY(EditAnywhere, Category = "Camera|Volume|Spline", meta = (UIMin = "0", UIMax = "10"))
		float SplineLerpSpeed{ 10.f };
	virtual void SetSplineInputkey(const float SplineKey) override { Internal_SplineInputkey = SplineKey; }


	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleDynamic", meta = (UIMin = "0", UIMax = "1"))
		float GrappleDynamic_DefaultAlpha{ 0.3f };
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleDynamic", meta = (UIMin = "0", UIMax = "90"))
		float GrappleDynamic_MaxYaw{ 30.f };
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleDynamic", meta = (UIMin = "0", UIMax = "1"))
		float GrappleDynamic_YawAlpha{ 0.3f };
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleDynamic", meta = (UIMin = "0", UIMax = "1"))
		float GrappleDynamic_MaxPitch{ 0.5f };
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleDynamic", meta = (UIMin = "0", UIMax = "1"))
		float GrappleDynamic_MinPitch{ -0.5f };
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleDynamic", meta = (UIMin = "0", UIMax = "1"))
		float GrappleDynamic_PitchAlpha{ 0.2f };
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleDynamic", meta = (UIMin = "-1", UIMax = "1"))
		float GrappleDynamic_DefaultPitch{ 0.2f };

	float InitialGrappleDynamicZ{};

	float GrappleDynamic_SLerpAlpha{};
	void GuideCameraTowardsVector(FVector vector, float alpha);
	void GuideCameraPitch(float z, float alpha);
		
		// decrepid? -- not in use
	float GuideCameraPitchAdjustmentLookAt(FVector LookatLocation, float MinDistance, float MaxDistance, float PitchAtMin, float PitchAtMax, float ZdiffMultiplier);	

	void GrappleDynamicGuideCamera_Gamepad(AActor* target, float deltatime);
	void GrappleDynamicGuideCamera(AActor* target, float deltatime);


	/* Default camera guide towards movement direction 
		* Runs the entire time but should not be noticable until the player has started moving a certain amount of time */
	void GuideCamera_Movement(float DeltaTime);
	UPROPERTY(EditAnywhere, Category = "Camera|Movement|Default")
		float GC_Mov_PositiveLerpSpeed{ 1.f };
	UPROPERTY(EditAnywhere, Category = "Camera|Movement|Default")
		float GC_Mov_NegativeLerpSpeed{ 2.f };
	// Without input, how fast will the alpha lerp down to 0
	UPROPERTY(EditAnywhere, Category = "Camera|Movement|Default")
		float GC_Mov_BaseNegativeLerpSpeed{ 2.f };

	// At what speed does the camera guide alpha start adding its max value
	UPROPERTY(EditAnywhere, Category = "Camera|Movement|Default")
		float GC_Mov_MaxVelocity{ 600.f };
	// Once the camera is within this angle of the input/velocity, its starts subtracting from the camera guide alpha value
	UPROPERTY(EditAnywhere, Category = "Camera|Movement|Default")
		float GC_Mov_DotproductLimit{ 0.8f };
	UPROPERTY(EditAnywhere, Category = "Camera|Movement|Default")
		float GC_Mov_Alpha_MAX{ 0.2f };
	float GC_Mov_Alpha{};

	/* Camera Guide towards static grapple target */
	void GuideCamera_StaticGrapple(float DeltaTime);
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleStatic")
		float GC_StaticGrapple_SlerpSpeed{ 2.f };
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleStatic")
		float GC_StaticGrapple_StartSpeed{ 0.2f };
	UPROPERTY(EditAnywhere, Category = "Camera|Mechanic|GrappleStatic")
		float GC_StaticGrapple_PitchMulti{ 0.5f };

	float GC_StaticGrapple_Alpha{};

	#pragma endregion	//CameraGuide

#pragma endregion					//Camera
#pragma region Basic_Movement
public:	// States
	EState GetState() const { return m_EState; }
	void SetState(EState state) { m_EState = state; }
	UFUNCTION(BlueprintCallable)
		void ResetState();
	void SetDefaultState();
	UFUNCTION(BlueprintCallable)
		virtual void AllowActionCancelationWithInput() override;
private:
	EState m_EState = EState::STATE_OnGround;
	EGroundState m_EGroundState = EGroundState::GROUND_Walk;
	EAirState m_EAirState = EAirState::AIR_None;
	//float m_BaseGravity{};

public:/* ------------------- Basic Movement ------------------- */
	UPROPERTY(EditAnywhere, Category = "Movement|Walk/Run", meta = (AllowPrivateAcces = "true"))
	float TurnRate{ 50.f };

	bool BreakMovementInput(float value);
	bool ActionLocked() const;

	void MoveForward(float value);
	void MoveRight(float value);
	virtual void AddControllerYawInput(float Val) override;
	virtual void AddControllerPitchInput(float Val) override;
	void Mouse_AddControllerYawInput(float Val);
	void Mouse_AddControllerPitchInput(float Val);
	void TurnAtRate(float rate);
	void LookUpAtRate(float rate);

	FTimerHandle TH_MovementPeriodLocked;
	FPostLockedMovement PostLockedMovementDelegate;
	void LockMovementForPeriod(float time, TFunction<void()> lambdaCall = nullptr);

	class UTimelineComponent* TLComp_AirFriction;
	UPROPERTY(EditAnywhere)
		UCurveFloat* Curve_AirFrictionMultiplier{ nullptr };

	bool bActivateJump{};
	UPROPERTY(BlueprintReadOnly, Category = "Movement|Jump")
		bool bJumping{};
	UPROPERTY(BlueprintReadOnly)
		bool bCanEdgeJump{};

	/* How long after walking off an edge the player is still allowed to jump */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
		float PostEdge_JumpTimer_Length{ 0.5f };
	bool bCanPostEdgeRegularJump{};
	FTimerHandle PostEdgeJump;

	/* The angle from the Upwards axis the jump direction will go towards input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
		float DoubleJump_AngleFromUp{ 60.f };

	/* What the jump strength will be multiplied by when double jumping */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
		float DoubleJump_MultiplicationFactor{ 0.6f };

	/* How long, post double jump, the character is held in the air (Z direction) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
		float Jump_HeightHoldTimer{ 1.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
		float JumpHeightHold_VelocityInterpSpeed{ 5.f };

	void Landed(const FHitResult& Hit) override;

	void Jump() override;
	void JumpRelease();

	void Jump_OnGround();
	void Jump_DoubleJump();
	void Jump_Undetermined();

	/* Animation activation */
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Activate_Jump();
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Activate_DoubleJump();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Anim_Land();

	bool CanDoubleJump() const;
	bool IsJumping() const;

	bool IsFalling() const;
	bool IsOnGround() const;

	/* 
	 * -------------------- New Jump : Cartoony --------------------------- 
	*/
	bool bJumpClick{};

	/* The strength of the Jump */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump|NewJump")
		float JumpStrength{ 2500.f };

	/* If the jump button is released, how fast will the character slow down? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump|NewJump")
		float JumpPrematureSlowDownTime{ 0.2f };

	/* How far through the jump, percentage wise, can the player go. Before releasing the button and still get the full jump height? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump|NewJump")
		float JumpFullPercentage{ 0.8f };

	/* Cancels the currently running Animation montage */
	UFUNCTION(BlueprintImplementableEvent)
		void CancelAnimation();
	void CancelAnimationMontageIfMoving(TFunction<void()> lambdaCall);
#pragma endregion			//Basic_Movement
#pragma region Pogo
private:
	EPogoType m_EPogoType = EPogoType::POGO_None;
	AActor* m_PogoTarget{ nullptr };

public:
	/* The strength of the pogo bounce */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Passive")
		float PB_LaunchStrength_Z_Passive{ 1300.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Passive")
		float PB_LaunchStrength_MultiXY_Passive{ 500.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Active")
		float PB_LaunchStrength_Active{ 1800.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Groundpound")
		float PB_LaunchStrength_Groundpound{ 2500.f };

	// Detection
	/* Extra contingency length checked between the player and the enemy they are falling towards, before the PogoBounce is called */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce")
		float PB_TargetLengthContingency{ 50.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce")
		float PB_Max2DTargetDistance{ 100.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Active", meta = (DisplayPriority = "2"))
		float PB_ActiveDetection_CapsuleZLocation{ 100.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Active", meta = (PrioDisplayPriorityrity = "3"))
		float PB_ActiveDetection_CapsuleHalfHeight{ 100.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Active", meta = (PrioDisplayPriorityrity = "4"))
		float PB_ActiveDetection_CapsuleRadius{ 70.f };

	// Minimum time the pogo state lasts - Will disable some mechanics while in that state
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Passive", meta = (DisplayPriority = "1"))
		float PB_StateTimer_Passive{ 0.2f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Active", meta = (DisplayPriority = "1"))
		float PB_StateTimer_Active{ 0.6f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Groundpound", meta = (DisplayPriority = "1"))
		float PB_StateTimer_Groundpound{ 0.4f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Passive")
		float PB_InputMulti_Passive{ 0.6f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Active")
		float PB_InputMulti_Active{ 0.3f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|PogoBounce|Groundpound")
		float PB_InputMulti_Groundpound{ 0.05f };
	
private: // Within Collision bools
	AActor* PB_Groundpound_TargetActor{ nullptr };
	AActor* PB_Active_PogoTarget{ nullptr };

	FTimerHandle TH_PB_ExitHandle; // Timer handle holding exit time. For validating buffering of PB_Active inputs
	FTimerHandle TH_Pogo;
	FTimerHandle TH_Pogo_NoCollision;
	bool bIsPogo_Active{};	// trash fix to active pogo issue

public:	// Target detection
	bool PB_TargetBeneath();
	bool PB_ValidTargetDistance(const FVector OtherActorLocation);	// Decrepid

	bool PB_Active_TargetDetection();

private:  
	bool Do_Pogo(const FHitResult& Hit);

	void PB_Pogo();
	void PB_EnterPogoState(float time);

	//bool PB_Passive_IMPL(AActor* OtherActor);
	bool PB_Passive_IMPL(const FHitResult& Hit);
	void PB_Launch_Passive(bool bOnStuckEnemy);

	void PB_Active_IMPL(AActor* PogoedActor);
	void PB_Launch_Active();

	bool PB_Groundpound_IMPL(AActor* OtherActor);
	bool PB_Groundpound_Predeterminehit();	// Decrepid
	void PB_Launch_Groundpound();

	void PB_Exit();

	bool ValidLengthToCapsule(FVector HitLocation, FVector capsuleLocation, float CapsuleHeight, float CapsuleRadius);

public: // Animation
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Pogo_Passive();
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Pogo_Active();

#pragma endregion					//Pogo
#pragma region Dash
	float Dash_WalkSpeed_Base{};
	FTimerHandle TH_Dash;
	/* How much speed is added to the base walkspeed? */
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
		float Dash_WalkSpeed_Add{ 200.f };
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
		float Dash_StartAgainPercentage{ 0.5f };
	
	void Dash_Start();
	UTimelineComponent* TLComp_Dash;
	UPROPERTY(EditAnywhere, Category = "Movement|Dash")
		UCurveFloat* Curve_DashStrength;
	UFUNCTION()
		void TL_Dash(float value);
	void TL_Dash_End();
	bool Can_Dash_Start() const;
#pragma endregion					//Dash
#pragma region Bounce
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UBouncyShroomActorComponent* BounceComp;
	bool ShroomBounce(FVector direction, float strength) override;
#pragma endregion					//Bounce
#pragma region Right Facebutton
	bool bPressedCancelButton{};	
	void Click_RightFacebutton();
	void UnClick_RightFacebutton();
#pragma endregion		//Right Facebutton
#pragma region Collectibles & Health
public:
	void ReceiveCollectible(class ACollectible* collectible);
	void ReceiveCollectible(class ACollectible_Static* collectible);
	UFUNCTION(BlueprintImplementableEvent)
		void ReceiveNewspaper();
	void ReceiveNewspaper_Pure(int index);
	UPROPERTY(BlueprintReadWrite)
		TArray<int> CollectedNewspapers;
	UPROPERTY(BlueprintReadWrite)
		TArray<int> CollectedNewspapers_WidgetIndexes;

	UPROPERTY(BlueprintReadWrite, Category = "Collectibles")
		int CollectibleCommon{};
	UPROPERTY(BlueprintReadWrite, Category = "Collectibles")
		int CollectibleCorruptionCore{};
	UPROPERTY(BlueprintReadWrite, Category = "Collectibles")
		int InkFlowerCollectible{};

	UPROPERTY(BlueprintReadWrite, Category = "Collectibles")
		int JournalEntries{ 0 };

	UFUNCTION()
		void OnCapsuleComponentBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnCapsuleComponentEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void OnCapsuleComponentHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health & Damage", meta = (UIMin = "1", UIMax = "10"))
		int Health{ 3 };
	int MaxHealth{};

	FTimerHandle THDamageBuffer;
	bool bPlayerCanTakeDamage{ true };
	/* Time player is invincible after taking damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health & Damage", meta = (UIMin = "0.0", UIMax = "2.0"))
		float DamageInvincibilityTime{ 1.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health & Damage", meta = (UIMin = "0.0", UIMax = "4000.0"))
		float SelfDamageLaunchStrength{ 1000.f };

	void GainHealth(int amount);
	void PTakeDamage(int damage, AActor* otheractor, int i = 0);
	void PTakeDamage(int damage, const FVector& Direction, int i = 0);
	bool PTakeRepeatDamage();

	UFUNCTION(BlueprintImplementableEvent)
		void HealthHairColor(int hp);
	UFUNCTION(BlueprintImplementableEvent)
		void GainHealth_Impl();
	UFUNCTION(BlueprintImplementableEvent)
		void TakeDamage_Impl();

	UFUNCTION(BlueprintImplementableEvent)
		void Update_WGT_CollectibleCounter();

	void Pickup_InkFlower();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateInkCollectible();
	UFUNCTION(BlueprintCallable)
		void CheckForNewJournalEntry();
	UFUNCTION(BlueprintImplementableEvent)
		void GetJournalEntry();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateHealthWidget();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateSapCollectible();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void ShowHUD_Timed();
	void ShowHUD_Timed_Pure();

	UFUNCTION(BlueprintImplementableEvent)
		void Anim_TakeDamage();
	
	bool bIsDead{};
	//bool IsDead() const { return bIsDead; }

	FDeath DeathDelegate;
	FDeath DeathDelegate_Land;
	void Death();
	void Death_Deathzone();
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Death();

	FTransform StartTransform;
	class APlayerRespawn* Checkpoint{ nullptr };
	void Respawn();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health & Damage", meta = (UIMin = "0.0", UIMax = "4000.0"))
		float RespawnTimer{ 2.f };

#pragma endregion	//Collectibles & Health
#pragma region OnWall
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		class UWallDetectionComponent* WallDetector{ nullptr };
	
	// Wall Decetion
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection")
		float WDC_Capsule_Radius{ 40.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection")
		float WDC_Capsule_Halfheight{ 90.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection")
		float Wall_HeightCriteria{ 20.f };
	// On Wall
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection|WallJump")
		float WDC_Length{ 40.f };
	// Ledge Grab
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection|Ledge")
		float LedgeGrab_Height{ 100.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection|Ledge")
		float LedgeGrab_Inwards{ 50.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|WallDetection|Ledge")
		FVector LedgeGrab_ActorZOffset {};

public: // OnWall
	FTimerHandle TH_OnWall_Cancel;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall")
		float OnWall_CancelTimer{ 0.5f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall")
		float OnWall_HangTime{ 0.5f };

public: // Walljump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|Walljump")
		float OnWall_Reset_OnWallJump_Timer{ 1.f };

	/* How long after a regular jump before OnWall mechanics are activated */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall")
		float OnWallActivation_PostJumpingOnGround{ 0.5f };
	/* After grapplehooking to a stuck enemy, disable wall jump for 'time' + time to stuck enemy */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall")
		float OnWallActivation_PostStuckEnemyGrappled{ 0.5f };

public:	// Ledge grab
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|Ledge")
		float LedgeLift_DotproductLimit{ 0.7f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|OnWall|Ledge")
		float LedgeLift_Timer{ 0.7f };
	FTimerHandle TH_Ledgelift;

	
public: // General
	EOnWallState m_WallState = EOnWallState::WALL_None;

	// Input direction to the wall 
	float WallInputDirection{};	
	void SetWallInputDirection();

	void ExitOnWall(EState state);
	void LeaveWall();
public: // Is funcitons
	bool IsOnWall() const;
	bool IsLedgeGrabbing() const;

public: // Animation and Particle effects
	bool Anim_IsOnWall() const;
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_OnWallContact();
	UFUNCTION(BlueprintImplementableEvent)
		void LedgeLift();
	UFUNCTION(BlueprintCallable)
		void EndLedgeLift();
private:
	Wall::WallData m_Walldata;
	Wall::WallData m_WallJumpData;
	Wall::LedgeData m_Ledgedata;

	void DrawDebugArms(const float& InputAngle);

	bool Validate_Ledge(FHitResult& hit);
	void Initial_LedgeGrab();
	void LedgeGrab();
	bool Determine_LedgeLift() const;


	bool ValidateWall();

	void OnWall_IMPL(float deltatime);
	void OnWall_Drag_IMPL(float deltatime, float velocityZ);

	void ExitOnWall_GROUND();

	void CancelOnWall();

#pragma endregion					//OnWall
#pragma region GrappleHook
public:
	void RightTriggerClick();
	void RightTriggerUn_Click();
	void GH_Click();	// The start of the grapple function -- Called by RightTriggerClick and other possible delegations
	void GH_SetGrappleType(IGameplayTagAssetInterface* ITag, IGrappleTargetInterface* IGrapple);

	UFUNCTION(BlueprintCallable)
		bool IsGrappling() const { return m_EState == EState::STATE_Grappling; }
	UFUNCTION(BlueprintCallable)
		bool Is_GH_PreLaunch() const { return IsGrappling() && m_EGrappleState == EGrappleState::Pre_Launch; }
	UFUNCTION(BlueprintCallable)
		bool Is_GH_StaticTarget() const;

public:	// Launch Functions
	void GH_PreLaunch();
	void GH_PreLaunch_Static(void(ASteikemannCharacter::* LaunchFunction)(), IGrappleTargetInterface* IGrapple);
	void GH_PreLaunch_Dynamic(IGrappleTargetInterface* IGrapple, bool OnGround);
	void GH_Launch_Dynamic(IGrappleTargetInterface* IGrapple, bool OnGround);

	void GH_Launch_Static();
	void GH_Launch_Static_StuckEnemy();

	void GH_Stop(EState newstate);
	void GH_Stop();
	void GH_Cancel();

	void PullDynamicTargetOffWall();
	void PullDynamicTargetOffWall_Instant();
	
	FTimerHandle TH_UnbindGrappleEnemyOnLand;
	FGrappleEnemyLandDelegate Delegate_GrappleEnemyOnLand;
	bool GH_GrappleLaunchLandDelegate();

public:	// Animation functions
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Grapple_Start();
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Grapple_Middle();
	UFUNCTION(BlueprintImplementableEvent)
		void Anim_Grapple_End();
	void Anim_Grapple_End_Pure();
	void GH_StopControlRig();
	void GH_DelegateDynamicLaunch();

	// For the control rig 
	FVector GH_GetTargetLocation() const;
	bool bGH_LerpControlRig{};

	virtual void StartAnimLerp_ControlRig() override;	// Called in animation montages

public: // Aiming and Visual Aid 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook|DynamicGround")
		FVector2D GH_GrappleSmackAiming_Gamepad_Multiplier = FVector2D(1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook|DynamicGround")
		FVector2D GH_GrappleSmackAiming_MNK_Multiplier = FVector2D(1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook|DynamicGround")
		FVector2D GH_GrappleSmackAiming_MNK_CameraWeight = FVector2D(1.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook|DynamicGround")
		FVector2D GH_GrappleSmackAimingVector_Default;
	FVector2D GH_GrappleSmackAimingVector;
	void GH_SetInitialGrappleSmackAimingVector(FVector GrappledActorLocation);

	UFUNCTION(BlueprintImplementableEvent)
		void GH_GrappleDynamic_Start();
	UFUNCTION(BlueprintImplementableEvent)
		void GH_GrappleDynamic_End();

	void GH_AimIndicator_LocationDirection_Pure(FVector Location, FVector Normal, FVector Direction, FVector CameraDirection);
	UFUNCTION(BlueprintImplementableEvent)
		void GH_AimIndicator_LocationDirection(FVector Location, FVector Normal, FVector Direction, FVector CameraDirection);
	UFUNCTION(BlueprintCallable)
		void GH_AimIndicator_LocationComplex(FVector& SurfaceLocation, FVector& SurfaceNormal);

	FVector GH_GrappleSmackAiming_MNK(AActor* Target);
	void GH_ShowGrappleSmackCurveIndicator_Gamepad(float DeltaTime, float DrawTime);
	void GH_ShowGrappleSmackCurveIndicator(float DeltaTime, float DrawTime);
	void GH_ShowGrappleSmackCurve(float DeltaTime, FVector Direction, float SmackStrength, float DrawTime);
	bool GH_ShowGrappleSmackImpactIndicator(FVector start, FVector end, float DrawTime);

public:
	UPROPERTY(BlueprintReadOnly)
		FVector Active_GrappledActor_Location{};
private:
	bool bGrappleClick{};

	EGrappleState m_EGrappleState = EGrappleState::None;
	EGrappleType m_EGrappleType = EGrappleType::None;
	TWeakObjectPtr<AActor> GrappledActor{ nullptr };
	TWeakObjectPtr<AActor> Active_GrappledActor{ nullptr };
	TWeakObjectPtr<AActor> GrappledEnemy{ nullptr };

	FTimerHandle TH_GrappleHold;
	bool GH_InvalidRelease{};
	FTimerHandle TH_GrappleHoldRelease;
	TFunction<void()> TFunc_GrappleHoldFunction;
	TFunction<void()> TFunc_GrappleLaunchFunction;

public:	// UPROPERTY Variables
	// How long movement input will be disabled after pulling a dynamic target free from being stuck
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook")
		float GH_PostPullingTargetFreeTime{ 0.5f };
	/* How long can the player hold the grappled enemy before they are launched? */
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook")
		float GH_GrapplingEnemyHold{ 1.f };
	//-----------------------------------------------

public: /* ------- GrappleTargetInterface ------ */
	virtual void TargetedPure() override {}

	virtual void UnTargetedPure() override {}

	virtual void HookedPure() override {}
	virtual void HookedPure(const FVector InstigatorLocation, bool OnGround, bool PreAction = false) override {}

	virtual void UnHookedPure() override {}

public:	// AttackInterface
	virtual void Cause_LeewayPause_Pure(float Pausetime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackInterface|Grapplehook")
		float AttackInterface_LeewayPause_Time{ 1.f };

public: /* ------- Native Variables and functions -------- */
	void LeftTriggerClick();
	void LeftTriggerUn_Click();


	UPROPERTY(BlueprintReadOnly)
		FGameplayTag GpT_GrappledActorTag;

	/* The rope that goes from the player character to the grappled actor */
	FVector GrappleRope{};

	/* Collision sphere used for detecting nearby grappleable actors */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Grappling Hook")
		class USphereComponent* GrappleTargetingDetectionSphere{ nullptr };

	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook")
		float GrappleHookRange{ 2000.f };

	TArray<AActor*> InReachGrappleTargets;

	UFUNCTION()
		void OnGrappleTargetDetectionBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnGrappleTargetDetectionEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/* The added percentage of the screens height that is added to the aiming location. A higher number turns it closer to the
		middle, with a lower number further up. 0 directly to the middle */
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook|Targeting")
		float GrappleAimYChange_Base{ 4.f };
	float GrappleAimYChange{};

	void GH_RemoveFromAiming(AActor* target);
	void GH_GrappleAiming();

	FTimerHandle TH_Grapplehook_Start;
	FTimerHandle TH_Grapplehook_Pre_Launch;
	FTimerHandle TH_Grapplehook_End_Launch;
	FTimerHandle TH_Grapplehook_OpenMovement;
	FTimerHandle TH_Grapplehook_StopControlRig;
	
	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook")
		float GrappleHook_LaunchSpeed{ 2000.f };

	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook")
		float GrappleHook_Threshhold{ 500.f };

	UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook")
		float GrappleHook_DividingFactor{ 2.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook")
		float GrappleHook_PostLaunchTimer{ 1.f };


	/* How long the player will be held in the air before being launched towards the grappled actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook")
		float GrappleDrag_PreLaunch_Timer_Length{ 0.25f };


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Grappling Hook|StuckEnemy")
		float GrappleHook_Time_ToStuckEnemy{ 0.3f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Grappling Hook|StuckEnemy")
		float GrappleHook_AboveStuckEnemy{ 50.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Grappling Hook|StuckEnemy")
		float GH_StuckEnemyVerticalOffsetScale{ 0.1f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Grappling Hook|StuckEnemy")
		float GH_StuckEnemy2DOffsetScale{ 0.1f };
	/* -- GRAPPLE CAMERA VARIABLES -- */
	/* Interpolation speed of the camera rotation during grapplehook Drag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook|Drag|Camera Rotation")
		float GrappleDrag_Camera_InterpSpeed{ 3.f };

	/* Pitch adjustment for the camera rotation during the Pre_Launch of Grapple Drag  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Grappling Hook|Drag|Camera Rotation")
		float GrappleDrag_Camera_PitchPoint{ 20.f };

#pragma endregion				//GrappleHook
#pragma region Attacks
	#pragma region General
	EAttackState m_EAttackState = EAttackState::None;
	FTimerHandle TH_BufferAttack;
	FAttackActionBuffer Delegate_AttackBuffer;
	ESmackAttackType m_ESmackAttackType = ESmackAttackType::Regular;
	EAttackType m_EAttackType = EAttackType::None;
	virtual void AttackContact(AActor* target) override;
	void AttackContact_Particles(FVector location, FQuat direction);

	// Time removed from
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|SmackAttack")
		float SmackAttack_GH_TimerRemoval{ 0.1f };

	UFUNCTION(BlueprintCallable)
		void Activate_AttackCollider() override;

	UFUNCTION(BlueprintCallable)
		void Deactivate_AttackCollider() override;

	void StartAttackBufferPeriod() override;
	void ExecuteAttackBuffer() override;
	void EndAttackBufferPeriod() override;

	FPostAttackBuffer Delegate_PostAttackBuffer;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|SmackAttack|GrappleSmack")
		float GrappleSmack_HeightMultiplier{ 2.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|SmackAttack|GrappleSmack", meta = (UIMin = "0.0", UIMax = "1.0"))
		float GrappleSmack_MaxHeight{ 0.7f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|SmackAttack|GrappleSmack", meta = (UIMin = "0.0", UIMax = "1.0"))
		float GrappleSmack_MinHeight{ 0.3f };
	void PostAttack_GrappleSmack(EPostAttackType& type);
	
	void BufferDelegate_Attack(void(ASteikemannCharacter::* func)());

	bool CanBeAttacked() override;

	FVector AttackDirection{};
	FVector AttackColliderScale{};

	void Click_Attack();
	void UnClick_Attack();

	UFUNCTION(BlueprintImplementableEvent)
		void AttackSmack_Start();
	UFUNCTION(BlueprintCallable)
		void AttackSmack_Start_Pure();
	void AttackSmack_Start_Ground_Pure();
	void AttackSmack_Grapple_Pure();
	// Combo
	int AttackComboCount{};
	UFUNCTION(BlueprintImplementableEvent)
		void ComboAttack(int combo);
	void ComboAttack_Pure();
	UFUNCTION(BlueprintCallable)
		void Cancel_SmackAttack();
	UFUNCTION(BlueprintCallable)
		void Stop_Attack();
	
	void RotateToAttack();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Attack")
		class UBoxComponent* AttackCollider{ nullptr };

	UFUNCTION()
		void OnAttackColliderBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void Gen_Attack(IAttackInterface* OtherInterface, AActor* OtherActor, const EAttackType AType) override;

	/***** TIMELINE COMPONENTS *****/
	/* Turning Character during attack		
		*	When an attack button is clicked, the player will be able to turn the character a certain degree 
		*	before the movement locks in. To make aiming the attack a bit easier */
	void TlCurve_AttackTurn_IMPL(float value);
	void TlCurve_AttackMovement_IMPL(float value);

	/// TIMELINE *** SMACK ATTACK ***
	class UTimelineComponent* TLComp_Attack_SMACK;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|Movement")
		UCurveFloat* Curve_AttackTurnStrength;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|Movement")
		UCurveFloat* Curve_AttackMovementStrength;
	UFUNCTION()
		void TlCurve_AttackTurn(float value);
	UFUNCTION()
		void TlCurve_AttackMovement(float value);

	#pragma endregion		//General
	#pragma region SmackAttack
	bool bAttackPress{};

	/* SMACK DIRECTION 
	 *  0. None of the below
	 *  1. Based on input 
	 *  2. Based on camera direction
	 *  3. Mixture of both 
	 * Currently no aiming method outside of this */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|SmackDirection", meta = (UIMin = "0", UIMax = "3"))
		uint8 SmackDirectionType{ 1 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|SmackDirection", meta = (UIMin = "1.0", UIMax = "4.0", EditCondition = "SmackDirectionType == 2 || SmackDirectionType == 3", EditConditionHides))
		float SmackDirection_CameraMultiplier{ 1.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks|SmackDirection", meta = (UIMin = "1.0", UIMax = "4.0", EditCondition = "SmackDirectionType == 1 || SmackDirectionType == 3", EditConditionHides))
		float SmackDirection_InputMultiplier{ 1.f };

	
	/* The angle from the ground the enemy will be smacked. 0.0: Is parallel to the ground. 1.0: Is directly upwards */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks", meta = (UIMin = "0.0", UIMax = "1.0"))
		float SmackUpwardAngle{ 0.6f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks")
		float SmackAttackStrength{ 1600.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks")
		float GrappleSmack_Strength{ 2300.f };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks", meta = (UIMin = "0", UIMax = "1"))
		float SmackAttack_InputAngleMultiplier{ 0.2 };
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|BasicAttacks", meta = (UIMin = "0", UIMax = "1"))
		float Grapplesmack_DirectionMultiplier{ 0.2 };

	UFUNCTION(BlueprintCallable)
		bool IsSmackAttacking() const;

	bool bCanBeSmackAttacked{ true };

	void Do_SmackAttack_Pure(IAttackInterface* OtherInterface, AActor* OtherActor) override;
	void Receive_SmackAttack_Pure(const FVector Direction, const float AttackStrength, const bool bOverrideStrength = false) override;

	bool GetCanBeSmackAttacked() const override { return bCanBeSmackAttacked; }
	void ResetCanBeSmackAttacked() override { bCanBeSmackAttacked = true; }

	#pragma endregion //SmackAttack
	#pragma region GroundPound

	bool bGroundPoundPress{};
	UFUNCTION(BlueprintCallable)
		bool IsGroundPounding() const { return m_EState == EState::STATE_Attacking && m_EAttackState == EAttackState::GroundPound; }

	void Click_GroundPound();
	void UnClick_GroundPound();

	void Do_GroundPound();

	/* Movement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GroundPound")
		float GP_PrePoundAirtime{ 0.3f };
	/* How fast, visually, the player will launch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|GroundPound")
		float GP_VisualLaunchStrength{ 2500.f };
	/* The launch strength the enemies will recieve */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|GroundPound")
		float GP_LaunchStrength{ 2500.f };
	/* How long the movement will be locked after landing with ground pound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|GroundPound")
		float GP_MovementPeriodLocked{ 1.f };
	FTimerHandle THandle_GPHangTime;


	void Launch_GroundPound();

	UFUNCTION(BlueprintImplementableEvent)
		void Anim_GroundPound_Initial();
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void Anim_GroundPound_Land_Ground();

	/* Collider */
	/* Time it takes for the ground pound collider to expand to max size */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GroundPound")
		float GroundPoundExpandTime{ 0.5f };
	/* The size of the ground pound hitbox's radius */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|GroundPound")
		float MaxGroundPoundRadius{ 300.f };
	float CurrentGroundPoundColliderSize{};

	FTimerHandle THandle_GPExpandTime{};
	FTimerHandle THandle_GPReset{};

	void Start_GroundPound();
	void Deactivate_GroundPound();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
		class USphereComponent* GroundPoundCollider{ nullptr };


	UFUNCTION(BlueprintCallable)
		void GroundPoundLand(const FHitResult& Hit);
	void ExpandGroundPoundCollider(float DeltaTime);

	void Do_GroundPound_Pure(IAttackInterface* OtherInterface, AActor* OtherActor);
	void Receive_GroundPound_Pure(const FVector& PoundDirection, const float& GP_Strength);

	#pragma endregion //GroundPound

#pragma endregion					//Attacks

#ifdef UE_BUILD_DEBUG
	void Print_State();
	void Print_State(float time);
#endif
};
