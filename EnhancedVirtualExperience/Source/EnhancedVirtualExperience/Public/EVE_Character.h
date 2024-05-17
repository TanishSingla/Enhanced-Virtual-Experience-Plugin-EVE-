
#pragma once

#include "CoreMinimal.h"
#include "VRCharacter.h"
#include "HeadMountedDisplayTypes.h"
#include "VR_TeleportController.h"
#include "InputActionValue.h"
#include "GameplayTags.h"
#include "EVE_Character.generated.h"

class AVR_TeleportController;
class UInputAction;
class UInputMappingContext;
class USphereComponent;


/* ********************************************************************** */

USTRUCT(BlueprintType)
struct FControllerHandTransformOffsets
{
	GENERATED_BODY()

	FControllerHandTransformOffsets() {}

	// Constructor initializing with four transforms
	FControllerHandTransformOffsets(const FTransform& LeftControllerOffset, const FTransform& RightControllerOffset, const FTransform& LeftHandOffset, const FTransform& RightHandOffset)
		: LeftControllerOffset(LeftControllerOffset), RightControllerOffset(RightControllerOffset), LeftHandOffset(LeftHandOffset), RightHandOffset(RightHandOffset) {}

	// Four transforms
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform LeftControllerOffset = FTransform(FRotator(-60.f, 0.f, 0.f),FVector(5.162f, 0.f, -2.605f), FVector(1.f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform RightControllerOffset = FTransform(FRotator(-60.f, 0.f, 0.f),FVector(5.162f, 0.f, -2.605f), FVector(1.f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform LeftHandOffset = FTransform(FRotator(-66.702f, 9.914f, 79.450f), FVector(-2.62f, -2.34f, 4.006f), FVector(1.f, 1.f, -1.f));

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform RightHandOffset = FTransform(FRotator(-66.702f, 9.914f, 79.450f), FVector(-2.62f, 2.34f, 4.006f), FVector(1.f));
};

UCLASS()
class ENHANCEDVIRTUALEXPERIENCE_API AEVE_Character : public AVRCharacter
{
	GENERATED_BODY()

public:
	
	AEVE_Character();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Variables

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | General",Meta = (ToolTip = "By default Character movement is set according to leftHand, Setting variable to true will set the character movement according to right controller and also then make sure to give the inputs according to right hand."))
	bool bIsRightHand = false;

	UFUNCTION()
	void NotifyServerOfTossRequest(bool LeftHand,UPrimitiveComponent * TargetPrim);

	UFUNCTION()
	void UpdateClimbingMovement_Binding();

	UFUNCTION(BlueprintCallable, Category="Character Properties | Grabbing | Functions" )
	void RumbleController(UGripMotionControllerComponent * Hand,UHapticFeedbackEffect_Base * HapticEff,float Intensity = 1.0f);
	
private:

	UPROPERTY()
	bool bTurningFlag = false;

	UPROPERTY()
	float InputValue = 0.0;
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction* IA_LeftTeleport;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction* IA_RightTeleport;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_RightGrip;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_LeftGrip;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_RightTriggerAxis;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_LeftTriggerAxis;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_Turn;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_Move;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_LaserBeamRight;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_LaserBeamLeft;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_LeftThumbstick;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Properties | InputSetup| VR")
	UInputAction * IA_RightThumbstick;
	
	//Inputs Functions :-

	//Teleportation
	void LeftTeleportStarted();
	void LeftTeleportCompleted();

	void RightTeleportStarted();
	void RightTeleportCompleted();

	//Left and Right grip
	void RightGripStarted();
	void RightGripCompleted();

	void LeftGripStarted();
	void LeftGripCompleted();

	void HandleTurn(const FInputActionValue & InputAxis);
	void HandleMove(const FInputActionValue & Input);

	void HandleLeftTeleportedRotation(const FInputActionValue & Input);
	void HandleLeftTeleportedRotationCompleted(const FInputActionValue & Input);
	void HandleRightTeleportedRotation(const FInputActionValue & Input);
	void HandleRightTeleportedRotationCompleted(const FInputActionValue & Input);
	
	void HandleLaserBeamRight();
	void HandleLaserBeamLeft();
	
	void RightTriggerStarted();
	void RightTriggeredCompleted();
	void LeftTriggerStarted();
	void LeftTriggeredCompleted();
	
	//Components
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="VR Components")
	USphereComponent * RightGrabSphere;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Character Properties | Properties | CharacterMovement | Teleportation")
	bool bEnableTeleportation = true;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Character Properties | Properties | CharacterMovement | Snapping")
	bool bEnableCharacterSmoothMovement = true;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Character Properties | Properties | CharacterMovement | Snapping")
	bool bEnableCharacterSnapping = true;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Character Properties | Properties | LaserBeam")
	bool bEnableLaserBeam = false;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Character Properties | Properties | LaserBeam",meta = (ToolTip = "Enabling the variable bEnableLaserBeamTriggerAxis will allow the hovered object to be tossed in your hands."))
	bool bEnableLaserBeamTriggerAxis = false;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="VR Components")
	USphereComponent * LeftGrabSphere;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Character Properties | Properties | CharacterMovement | General",Meta=(ToolTip = "Enable/disable character movement according to the rotation of the controller. When enabled, the character's movement direction follows the controller's rotation."))
	bool bCharacterMovementAccordingToController = false;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="VR Components")
	USceneComponent * AimRight;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="VR Components")
	USceneComponent * AimLeft;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="VR Components")
	USkeletalMeshComponent * HandMeshRight;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="VR Components")
	USkeletalMeshComponent * HandMeshLeft;
	
	UPROPERTY(BlueprintReadWrite,Category="VR Components")
	UPrimitiveComponent* LeftHandGripComponent;
	
	UPROPERTY(BlueprintReadWrite,Category="VR Components")
	UPrimitiveComponent* RightHandGripComponent;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | InputSetup| VR")
	UInputMappingContext * VRInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Character Properties | Properties")
	bool UseGraspingHandsWhenPossible = true;

	UPROPERTY(EditDefaultsOnly, Category = "Character Properties | Properties")
	FControllerHandTransformOffsets TransformOffsets;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | Tracking Origin")
	TEnumAsByte<EHMDTrackingOrigin::Type> TrackingOrigin = EHMDTrackingOrigin::Floor;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | Spectator")
	bool bEnableSmoothSpectator = true;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | Spectator", meta = (EditCondition = "bEnableSmoothSpectator"))
	float SpectatorFOV = 90.f;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | Spectator", meta = (EditCondition = "bEnableSmoothSpectator"))
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(BlueprintReadOnly,Category="Teleport Controllers")
	AVR_TeleportController * TeleportControllerLeft;

	UPROPERTY(BlueprintReadOnly,Category="Teleport Controllers")
	AVR_TeleportController * TeleportControllerRight;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Teleportation")
	TSubclassOf<AVR_TeleportController> TeleportControllerClass;

	UPROPERTY(BlueprintReadWrite,Category="Properties | Teleportation")
	bool IsTeleporting = false;

	UPROPERTY()
	bool bIsLeftTeleportInputActive = false;

	UPROPERTY()
	bool bIsRightTeleportInputActive = false;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Teleportation")
	float FadeOutDuration  = 0.40f;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Teleportation")
	FColor TeleportFadeColor;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Teleportation",Meta = (ToolTip = "Disabling this option will prevent the user from altering rotation during teleportation."))
	bool bTeleportUsesThumbRotation = true;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Teleportation")
	float TeleportThumbDeadZone = 0.4f;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
	bool bUsingGraspingHands = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsMobile = false;

	//Variables
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	TEnumAsByte<ETraceTypeQuery> CollisionChannelTraceTypeQuery = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	float GripTraceLength = 0.1f;
	
	UPROPERTY(BlueprintReadOnly,Category="Character Properties | Grabbing | Variables")
	UObject * NearestOverlappingObjectValue = nullptr;
	
	//Tags
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	FGameplayTag DefaultSecondaryDropTag;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	FGameplayTag DefaultSecondaryGripTag;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	FGameplayTag DefaultDropTag;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	FGameplayTag DefaultGripTag;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	bool bForceOverlapOnlyGripChecks = true;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity")
	bool UseControllerVelocityOnRelease = true;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity")
	bool ScaleVelocityByMass = false;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity")
	float ThrowingMassMaximum = 50.0f;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity")
	float MassScalerMin = 0.3f; 

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity")
	float MaximumThrowingVelocity = 800.0f;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity")
	bool SampleGripVelocity  = false;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity", meta = (EditCondition = "SampleGripVelocity"))
	FBPLowPassPeakFilter LeftPeakVel;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity", meta = (EditCondition = "SampleGripVelocity"))
	FBPLowPassPeakFilter RightPeakVel;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Velocity")
	bool LimitMaxThrowVelocity = true;
	
	bool bIsSecondaryGrip = false;
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool TryToGrabObject(UObject * ObjectToTryToGrab,FTransform WorldTransform,UGripMotionControllerComponent * Hand,UGripMotionControllerComponent * OtherHand,bool bIsSlotGrip,FGameplayTag GripSecondaryTag,FName GripBoneName,FName SlotName,bool IsSecondaryGrip);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool TryToSecondaryGripObject(UGripMotionControllerComponent * Hand,UGripMotionControllerComponent * OtherHand,UObject * ObjectToTryToGrab,FGameplayTag GripSecondaryTag,bool ObjectImplementsInterface,FTransform RelativeSecondaryTransform,FName SlotName,bool bHadSlot);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void GetNearestOverlappingObject(UPrimitiveComponent * OverlapComponent,UGripMotionControllerComponent * Hand,FGameplayTagContainer RelevantGameplayTags ,UObject *&NearestObject,bool &bImplementsInterface,FTransform &ObjectTransform,bool &bCanBeClimbed,FName &BoneName,FVector &ImpactLoc);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool HasValidGripCollision(UPrimitiveComponent * Component);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void SelectObjectFromHitArray(TArray<FHitResult>Hits,FGameplayTagContainer RelevantGameplayTags,UGripMotionControllerComponent*Hand,bool &bShouldGrip,bool &ObjectImplementsInterface,UObject *&ObjectToGrip,FTransform &WorldTransform,UPrimitiveComponent * &FirstPrimitiveHit,FName &BoneName,FVector &ImpactPoint);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool ValidateGameplayTagContainer(FGameplayTag BaseTag,UObject * Object,FGameplayTag DefaultTag,FGameplayTagContainer GameplayTags);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	UObject * GetNearestOverlapOfHand(UGripMotionControllerComponent *Hand,UPrimitiveComponent *OverlapSphere);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void TryRemoveSecondaryAttachment(UGripMotionControllerComponent *CallingMotionController,UGripMotionControllerComponent*OtherController,FGameplayTagContainer GameplayTags,bool &DroppedSecondary,bool &HadSecondary);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool IsALocalGrip(EGripMovementReplicationSettings GripRepType);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool ValidateGameplayTag(FGameplayTag BaseTag,FGameplayTag GameplayTag,UObject * Object,FGameplayTag DefaultTag);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void DropSecondaryAttachment(UGripMotionControllerComponent * CallingMotionController,UGripMotionControllerComponent *OtherController,FGameplayTagContainer GameplayTags,bool &DroppedSecondary,bool &HadSecondary);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void ShouldSocketGrip(FBPActorGripInformation Grip,bool &ShouldSocket,USceneComponent * &SocketParent,FTransform_NetQuantize &RelativeTransform,FName & OptionalSocketName);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void TryDropSingleClient(UGripMotionControllerComponent * Controlller,FBPActorGripInformation GripToDrop,FVector AngleVel,FVector LinearVel);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void TryDropSingle(UGripMotionControllerComponent *Controlller,FVector_NetQuantize AngleVel,FVector_NetQuantize LinearVel,int Hash);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void CheckGripPriority(UObject *ObjectTsCheck,int PrioToCheckAgainst, bool CheckAgainstPrior, bool &HadHigherPriority,int &NewGripPrio);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool CheckIsValidForGripping(UObject * Object,FGameplayTagContainer RelevantGameplayTags); 

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void CanAttemptSecondaryGrabOnObject(UObject * ObjectToCheck,bool &CanAttemptSecondaryGrab,ESecondaryGripType &SecondaryGripType);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void ShouldGripComponent(UPrimitiveComponent * ComponentToCheck,int GripPrioToCheckAgainst,bool bCheckAgainstPrior,FName BoneName,FGameplayTagContainer RelevantGameplayTags,UGripMotionControllerComponent * CallingController,bool &ShouldGrip,UObject*&ObjectToGrip,bool &ObjectImplementsInterface,FTransform &ObjectsWorldTransform,int &GripPrio);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void CheckUseSecondaryAttachment(UGripMotionControllerComponent * CallingMotionController,UGripMotionControllerComponent * OtherController,bool ButtonPressed,bool &DroppedOrUsedSecondary,bool &HadSecondary);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void CheckUseHeldItems(UGripMotionControllerComponent * Hand,bool ButtonState);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	FTransform RemoveControllerScale(FTransform SocketTransform,UGripMotionControllerComponent * GrippingController);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool CanAttemptGrabOnObject(UObject * ObjectToCheck);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool CanSecondaryGripObject(UGripMotionControllerComponent *Hand,UGripMotionControllerComponent *OtherHand,UObject * ObjectToTryToGrab,FGameplayTag GripSecondaryTag,bool HadSlot,ESecondaryGripType SecGripType);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	FTransform GetBoneTransform(UObject * Object,FName BoneName);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void CallCorrectDropSingleEvent(UGripMotionControllerComponent*Hand,FBPActorGripInformation Grip);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void GetThrowingVelocity(UGripMotionControllerComponent *ThrowingController,FBPActorGripInformation Grip,FVector AngularVel,FVector ObjectsLinearVel,FVector &OutAngleVel,FVector &OutLinearVel);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void TryGrabClient(UObject *ObjectToGrab,bool IsSlotGrip,FTransform_NetQuantize GripTransform,EControllerHand Hand,FGameplayTag GripSecondaryTag,FName GripBoneName,FName SlotName,bool IsSecondaryGrip);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void TryGrabServer(UObject *ObjectToGrab,bool IsSlotGrip,FTransform_NetQuantize GripTransform,EControllerHand Hand,FGameplayTag GripSecondaryTag,FName GripBoneName,FName SlotName,bool IsSecondaryGrip);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void CallCorrectGrabEvent(UObject *ObjectToGrip,EControllerHand Hand,bool IsSlotGrip,FTransform GripTransform,FGameplayTag GripSecondaryTag,FName OptionalBoneName,FName SlotName,bool IsSecondaryGrip);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void DropItem(UGripMotionControllerComponent * Hand,FBPActorGripInformation GripInfo,FGameplayTagContainer GameplayTags);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void DropItems(UGripMotionControllerComponent *Hand,FGameplayTagContainer GameplayTags);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	FName GetCorrectPrimarySlotPrefix(UObject *ObjectToCheckForTag,EControllerHand Hand,FName NearestBoneName);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	bool GripOrDropObjectClean(UGripMotionControllerComponent * CallingMotionController,UGripMotionControllerComponent * OtherController,bool CanCheckClimb,UPrimitiveComponent * GrabSphere,FGameplayTagContainer RelevantGameplayTags);
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void TriggerGripOrDrop(UGripMotionControllerComponent *CallingHand,UGripMotionControllerComponent*OtherHand,bool isGrip,UPrimitiveComponent * GrabSphere);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Grabbing | Functions")
	void RemoveSecondaryGrip(UGripMotionControllerComponent*Hand,UObject * GrippedActorToRemoveAttachment);

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	UHapticFeedbackEffect_Base * GrabHapticEffect = nullptr;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Grabbing | Variables")
	float IntensityHapticEffect = 1.0f;
	
	/* *********************************************************************************  */
	
	//Overlapping
	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | OverlappingEvents")
	void OnRightGrabSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	

	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | OverlappingEvents")
	void OnRightGrabSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | OverlappingEvents")
	void OnLeftGrabSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	

	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | OverlappingEvents")
	void OnLeftGrabSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Climbing :- (Currently Not Working in Cpp)
	// virtual void UpdateClimbingMovement(float DeltaTime) override;
	
	//Teleportation
	UFUNCTION(BlueprintCallable,Category="Character Properties | InputSetup")
	void MapInput(UInputMappingContext * InputMapping, int32 Priority = 0);
	
	UFUNCTION(BlueprintCallable)
	void SpawnController();
	
	UFUNCTION(BlueprintCallable)
	void SetGripComponents(UPrimitiveComponent * LeftHand,UPrimitiveComponent* RightHand);
	
	UFUNCTION()
	void InitTeleportControllers();

	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions  | Teleportation")
	void UpdateTeleportationRotations(AVR_TeleportController* TeleportController, FVector2D Input);

	UFUNCTION()
	void SetTeleportActive(EControllerHand Hand,bool Active);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions  | Teleportation")
	void ExecuteTeleportation(AVR_TeleportController * MotionController,EControllerHand Hand);

	UFUNCTION()
	void GetCharacterRotatedPosition(const FVector& OriginalLocation, const FRotator& DeltaRotation,FVector PivotPoint,FVector &OutLocation,FRotator &OutRotation);

	UFUNCTION()
	FRotator GetCorrectRotation();
 
	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | Teleportation")
	void NotifyTeleportActive(EControllerHand Hand,bool State);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | Teleportation")
	void MulticastTeleportActive(EControllerHand Hand,bool State);
	
	void DelayFunctionRunAfterFadeTime();

	EControllerHand TeleportHandForFade;
	FVector TeleportLocationForFade;
	FRotator TeleportRotationForFade;
	
	//Snapping
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Snapping")
	bool bTurnModeIsSnap;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Snapping",meta = (EditCondition = "!bTurnModeIsSnap"))
	float TurningActivationThreshold = 0.7f;

	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Snapping", meta = (EditCondition = "!bTurnModeIsSnap"))
	float SmoothTurnSpeed = 50.0f;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | Snapping", meta = (EditCondition = "bTurnModeIsSnap"))
	float SnapTurnAngle = 45.0f;
	
	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions  | Snapping")
	void HandleTurnInput(float InputAxis);

	// Character Movement 
	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | Character Movement")
	void HandleCurrentMovementInput(FVector2D MovementInput);
	
	UFUNCTION()
	void CalcPadRotationAndMagnitude(float YAxis,float XAxis,float OptMagnitudeScaler,float OptionalDeadzone,FRotator &OutRotation,float &OutMagnitude,bool &WasValid);

	UFUNCTION()
	void GetDPadMovementFacing(const UGripMotionControllerComponent*CallingHand,FVector &OutForwardVector,FVector &OutRightVector);

	UFUNCTION()
	void MapThumbToWorld(const UGripMotionControllerComponent*CallingHand, const FRotator& Rotation,FVector & OutDirection);

	UFUNCTION()
	USceneComponent *GetCorrectAimComp(const UGripMotionControllerComponent*CallingHand);
	
	float ThumbX = 0.0f;
	float ThumbY = 0.f;
	float DPadVelocityScaler =  1.25f;
	FVector2D LeftSmoothTeleportRotator = FVector2D::Zero();
	FVector2D RightSmoothTeleportRotator = FVector2D::Zero();
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Properties | CharacterMovement | General")
	float SlidingMovementDeadZone = 0.0f;	

	UPROPERTY()
	FVector Direction = FVector(0,0,0);
	
	//Climbing
	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | Climbing")
	void InitializeClimbing(UPrimitiveComponent * NewClimbingGrip,UGripMotionControllerComponent*CallingHand);

	UFUNCTION(BlueprintCallable,Category="Character Properties | Functions | Climbing")
	void ExitClimbing();
	
	UPROPERTY(BlueprintReadOnly,Category="Character Properties | Climbing")
	FVector ClimbGripLocation;
	
	UPROPERTY(BlueprintReadOnly,Category="Character Properties | Climbing")
	UPrimitiveComponent * ClimbingGrip;

	UPROPERTY(BlueprintReadOnly,Category="Character Properties | Climbing")
	bool bIsClimbing = false;

	UPROPERTY(BlueprintReadOnly,Category="Climbing")
	UGripMotionControllerComponent * ClimbingHand;

	UPROPERTY(BlueprintReadOnly,Category="Climbing")
	UPrimitiveComponent * ClimbingWallGripRight;

	UPROPERTY(BlueprintReadOnly,Category="Climbing")
	UPrimitiveComponent * ClimbingWallGripLeft;
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Climbing",Meta=(ToolTip = "Make sure to apply the Same tag to the item you intend to ascend."))
	FName ClimbingTag = "climbable";
	
	UPROPERTY(EditDefaultsOnly,Category="Character Properties | Climbing",Meta=(ToolTip = "If Enabled will allow Snapping/Turning on Climbing."))
	bool bEnableSnapTurnWhileClimbing = false;

	/*   */
	UFUNCTION(BlueprintCallable)
	FString CheckXRApi();

	void SpawnSpectator();

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnGraspingHands();

	UFUNCTION(BlueprintImplementableEvent)
	void EnableControllerAnimations();

	/*   Widget Interaction  */
	UFUNCTION(BlueprintCallable)
	bool IfOverWidgetUse(UGripMotionControllerComponent * CallingHand,bool Pressed);
	
	
	// Print function.
	void Print(const FString& Message,int key = 1 ,FColor Color  =FColor::Red,float TimeToDisplay=3);
	void Print(int Message,int key = 1 ,FColor Color  =FColor::Red,float TimeToDisplay=3);
	void Print(const FTransform& Message,int key = 1 ,FColor Color  =FColor::Red,float TimeToDisplay=3,bool PrintOnlyLocation = true);
	
};

/* ********************************************************************** */