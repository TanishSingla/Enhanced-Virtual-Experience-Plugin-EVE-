
#pragma once

#include "CoreMinimal.h"
#include "VRCharacter.h"
#include "HeadMountedDisplayTypes.h"
#include "TeleportController.h"
#include "InputActionValue.h"
#include "GameplayTags.h"
#include "IkarusVRBaseCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USphereComponent;


/* ********************************************************************** */


UCLASS()
class VREXPANSIONPLUGIN_API AIkarusVRBaseCharacter : public AVRCharacter
{
	GENERATED_BODY()

public:
	
	AIkarusVRBaseCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Variables

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement",Meta = (ToolTip = "By default Character movement is set according to leftHand, Setting variable to true will set the character movement according to right controller and also then make sure to give the inputs according to right hand."))
	bool bIsRightHand = false;

	UFUNCTION()
	void NotifyServerOfTossRequest(bool LeftHand,UPrimitiveComponent * TargetPrim);
	
private:

	UPROPERTY()
	FInputActionValue TeleportRotationInput;
	bool bTurningFlag = false;
	float InputValue = 0.0;
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Ikarus Character | InputSetup|Inputs|VR")
	UInputAction* IA_Teleport;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Ikarus Character | InputSetup|Inputs|VR")
	UInputAction * IA_RightGrip;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Ikarus Character | InputSetup|Inputs|VR")
	UInputAction * IA_LeftGrip;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Ikarus Character | InputSetup|Inputs|VR")
	UInputAction * IA_Turn;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Ikarus Character | InputSetup|Inputs|VR")
	UInputAction * IA_Move;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Ikarus Character | InputSetup|Inputs|VR")
	UInputAction * IA_LaserBeam;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "true"), Category = "Ikarus Character | InputSetup|Inputs|VR")
	UInputAction * IA_LaserBeamTriggerAxis;

	
	
	//Inputs Functions :-

	//Teleportation
	void TeleportStarted();
	void TeleportTriggered();
	void TeleportCompleted();

	//Left and Right grip
	void RightGripStarted();
	void RightGripTriggered();
	void RightGripCompleted();

	void LeftGripStarted();
	void LeftGripTriggered();
	void LeftGripCompleted();

	void HandleTurn(const FInputActionValue & InputAxis);
	void HandleMove(const FInputActionValue & Input);
	void HandleLaserBeam();
	void HandleLaserBeamAxis();
	
	
	//Components
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="VR Components")
	USphereComponent * RightGrabSphere;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Ikarus Character | Properties | CharacterMovement | Teleportation")
	bool bEnableTeleportation = true;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Ikarus Character | Properties | CharacterMovement | SmoothMovement")
	bool bEnableCharacterSmoothMovement = true;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Ikarus Character | Properties | CharacterMovement | Snapping")
	bool bEnableCharacterSnapping = true;

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Ikarus Character | Properties | Components | LaserBeam")
	bool bEnableLaserBeam = false;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Ikarus Character | Properties | Components | LaserBeam",meta = (ToolTip = "Enabling the variable bEnableLaserBeamTriggerAxis will allow the hovered object to be tossed in your hands."))
	bool bEnableLaserBeamTriggerAxis = false;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="VR Components")
	USphereComponent * LeftGrabSphere;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Ikarus Character | Properties | CharacterMovement",Meta=(ToolTip = "Enable/disable character movement according to the rotation of the controller. When enabled, the character's movement direction follows the controller's rotation."))
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

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | InputSetup|Inputs|VR")
	UInputMappingContext * VRInputMapping;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | Tracking Origin")
	TEnumAsByte<EHMDTrackingOrigin::Type> TrackingOrigin = EHMDTrackingOrigin::Floor;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | Spectator Screen Mode")
	ESpectatorScreenMode SpectatorScreenMode = ESpectatorScreenMode::SingleEyeCroppedToFill;

	UPROPERTY(BlueprintReadOnly,Category="Teleport Controllers")
	ATeleportController * TeleportControllerLeft;

	UPROPERTY(BlueprintReadOnly,Category="Teleport Controllers")
	ATeleportController * TeleportControllerRight;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Teleportation")
	TSubclassOf<ATeleportController> TeleportControllerClass;

	UPROPERTY(BlueprintReadWrite,Category="Properties | Teleportation")
	bool IsTeleporting = false;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Teleportation")
	float FadeOutDuration  = 0.40f;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Teleportation")
	FColor TeleportFadeColor;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Teleportation",Meta = (ToolTip = "Disabling this option will prevent the user from altering rotation during teleportation."))
	bool bTeleportUsesThumbRotation = true;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Teleportation")
	float TeleportThumbDeadZone = 0.4f;

	
	/* ---------Functions---------- */
	
	
	/* *********************************************************************************  */
	// Grabbing Functions (temporary)
	
	// UPrimitiveComponent * LeftHitComp = NULL;
	// UPrimitiveComponent * RightHitComp = NULL;
	//
	// UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | Grabbing")
	//  TEnumAsByte<ETraceTypeQuery> CollisionChannel ;
	//
	// UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | Grabbing")
	// float SphereTraceRadius = 8.0f;
	//
	// UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | Grabbing")
	// float EndTraceValue = 0;
	//
	// UFUNCTION(BlueprintCallable)
	// void TryToGrabAndDrop(UGripMotionControllerComponent * CallingHand,bool bGrip,USphereComponent * GrabSphere,UGripMotionControllerComponent *OtherHand,bool bIsLeft	);
	//
	// UFUNCTION(BlueprintCallable)
	// void TryToDrop(UGripMotionControllerComponent * CallingHand,UObject * GrippedObject = NULL);

	//Variables
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing")
	TEnumAsByte<ETraceTypeQuery> CollisionChannelTraceTypeQuery = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Variables")
	float GripTraceLength = 0.1f;
	
	UPROPERTY(BlueprintReadOnly,Category="Ikarus Character | Grabbing | Variables")
	UObject * NearestOverlappingObjectValue = NULL;
	
	//Tags
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Variables")
	FGameplayTag DefaultSecondaryDropTag;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Variables")
	FGameplayTag DefaultSecondaryGripTag;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Variables")
	FGameplayTag DefaultDropTag;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Variables")
	FGameplayTag DefaultGripTag;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Variables")
	bool bForceOverlapOnlyGripChecks = true;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity")
	bool UseControllerVelocityOnRelease = true;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity")
	bool ScaleVelocityByMass = false;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity")
	float ThrowingMassMaximum = 50.0f;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity")
	float MassScalerMin = 0.3f; 

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity")
	float MaximumThrowingVelocity = 800.0f;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity")
	bool SampleGripVelocity  = false;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity", meta = (EditCondition = "SampleGripVelocity"))
	FBPLowPassPeakFilter LeftPeakVel;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity", meta = (EditCondition = "SampleGripVelocity"))
	FBPLowPassPeakFilter RightPeakVel;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Grabbing | Velocity")
	bool LimitMaxThrowVelocity = true;
	
	bool bIsSecondaryGrip = false;
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool TryToGrabObject(UObject * ObjectToTryToGrab,FTransform WorldTransform,UGripMotionControllerComponent * Hand,UGripMotionControllerComponent * OtherHand,bool bIsSlotGrip,FGameplayTag GripSecondaryTag,FName GripBoneName,FName SlotName,bool IsSecondaryGrip);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool TryToSecondaryGripObject(UGripMotionControllerComponent * Hand,UGripMotionControllerComponent * OtherHand,UObject * ObjecToTryToGrab,FGameplayTag GripSecondaryTag,bool ObjectImplementsInterface,FTransform RelativeSecondaryTransform,FName SlotName,bool bHadSlot);
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void GetNearestOverlappingObject(UPrimitiveComponent * OverlapComponent,UGripMotionControllerComponent * Hand,FGameplayTagContainer RelevantGameplayTags ,UObject *&NearestObject,bool &bImplementsInterface,FTransform &ObjectTransform,bool &bCanBeClimbed,FName &BoneName,FVector &ImpactLoc);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool HasValidGripCollision(UPrimitiveComponent * Component);
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void SelectObjectFromHitArray(TArray<FHitResult>Hits,FGameplayTagContainer RelevantGameplayTags,UGripMotionControllerComponent*Hand,bool &bShouldGrip,bool &ObjectImplementsInterface,UObject *&ObjectToGrip,FTransform &WorldTransform,UPrimitiveComponent * &FirstPrimitiveHit,FName &BoneName,FVector &ImpactPoint);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool ValidateGameplayTagContainer(FGameplayTag BaseTag,UObject * Object,FGameplayTag DefaultTag,FGameplayTagContainer GameplayTags);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	UObject * GetNearestOverlapOfHand(UGripMotionControllerComponent *Hand,UPrimitiveComponent *OverlapSphere);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void TryRemoveSecondaryAttachment(UGripMotionControllerComponent *CallingMotionController,UGripMotionControllerComponent*OtherController,FGameplayTagContainer GameplayTags,bool &DroppedSecondary,bool &HadSecondary);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool IsALocalGrip(EGripMovementReplicationSettings GripRepType);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool ValidateGameplayTag(FGameplayTag BaseTag,FGameplayTag GameplayTag,UObject * Object,FGameplayTag DefaultTag);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void DropSecondaryAttachment(UGripMotionControllerComponent * CallingMotionController,UGripMotionControllerComponent *OtherController,FGameplayTagContainer GameplayTags,bool &DroppedSecondary,bool &HadSecondary);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void ShouldSocketGrip(FBPActorGripInformation Grip,bool &ShouldSocket,USceneComponent * &SocketParent,FTransform_NetQuantize &RelativeTransform,FName & OptionalSocketName);
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void TryDropSingleClient(UGripMotionControllerComponent * Controlller,FBPActorGripInformation GripToDrop,FVector AngleVel,FVector LinearVel);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void TryDropSingle(UGripMotionControllerComponent *Controlller,FVector_NetQuantize AngleVel,FVector_NetQuantize LinearVel,int Hash);
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void CheckGripPriority(UObject *ObjectTsCheck,int PrioToCheckAgainst, bool CheckAgainstPrior, bool &HadHigherPriority,int &NewGripPrio);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool CheckIsValidForGripping(UObject * Object,FGameplayTagContainer RelevantGameplayTags); 

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void CanAttemptSecondaryGrabOnObject(UObject * ObjectToCheck,bool &CanAttemptSecondaryGrab,ESecondaryGripType &SecondaryGripType);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void ShouldGripComponent(UPrimitiveComponent * ComponentToCheck,int GripPrioToCheckAgainst,bool bCheckAgainstPrior,FName BoneName,FGameplayTagContainer RelevantGameplayTags,UGripMotionControllerComponent * CallingController,bool &ShouldGrip,UObject*&ObjectToGrip,bool &ObjectImplementsInterface,FTransform &ObjectsWorldTransform,int &GripPrio);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void CheckUseSecondaryAttachment(UGripMotionControllerComponent * CallingMotionController,UGripMotionControllerComponent * OtherController,bool ButtonPressed,bool &DroppedOrUsedSecondary,bool &HadSecondary);
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void CheckUseHeldItems(UGripMotionControllerComponent * Hand,bool ButtonState);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	FTransform RemoveControllerScale(FTransform SocketTransform,UGripMotionControllerComponent * GrippingController);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool CanAttemptGrabOnObject(UObject * ObjectToCheck);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool CanSecondaryGripObject(UGripMotionControllerComponent *Hand,UGripMotionControllerComponent *OtherHand,UObject * ObjectToTryToGrab,FGameplayTag GripSecondaryTag,bool HadSlot,ESecondaryGripType SecGripType);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	FTransform GetBoneTransform(UObject * Object,FName BoneName);
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void CallCorrectDropSingleEvent(UGripMotionControllerComponent*Hand,FBPActorGripInformation Grip);
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void GetThrowingVelocity(UGripMotionControllerComponent *ThrowingController,FBPActorGripInformation Grip,FVector AngularVel,FVector ObjectsLinearVel,FVector &OutAngleVel,FVector &OutLinearVel);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void TryGrabClient(UObject *ObjectToGrab,bool IsSlotGrip,FTransform_NetQuantize GripTransform,EControllerHand Hand,FGameplayTag GripSecondaryTag,FName GripBoneName,FName SlotName,bool IsSecondaryGrip);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void TryGrabServer(UObject *ObjectToGrab,bool IsSlotGrip,FTransform_NetQuantize GripTransform,EControllerHand Hand,FGameplayTag GripSecondaryTag,FName GripBoneName,FName SlotName,bool IsSecondaryGrip);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void CallCorrectGrabEvent(UObject *ObjectToGrip,EControllerHand Hand,bool IsSlotGrip,FTransform GripTransform,FGameplayTag GripSecondaryTag,FName OptionalBoneName,FName SlotName,bool IsSecondaryGrip);
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void DropItem(UGripMotionControllerComponent * Hand,FBPActorGripInformation GripInfo,FGameplayTagContainer GameplayTags);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void DropItems(UGripMotionControllerComponent *Hand,FGameplayTagContainer GameplayTags);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	FName GetCorrectPrimarySlotPrefix(UObject *ObjectToCheckForTag,EControllerHand Hand,FName NearestBoneName);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	bool GripOrDropObjectClean(UGripMotionControllerComponent * CallingMotionController,UGripMotionControllerComponent * OtherController,bool CanCheckClimb,UPrimitiveComponent * GrabSphere,FGameplayTagContainer RelevantGameplayTags);


	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void TriggerGripOrDrop(UGripMotionControllerComponent *CallingHand,UGripMotionControllerComponent*OtherHand,bool isGrip,UPrimitiveComponent * GrabSphere);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Grabbing | Functions")
	void RemoveSecondaryGrip(UGripMotionControllerComponent*Hand,UObject * GrippedActorToRemoveAttachment);

	/* *********************************************************************************  */
	
	//Overlapping
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | OverlappingEvents")
	void OnRightGrabSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | OverlappingEvents")
	void OnRightGrabSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | OverlappingEvents")
	void OnLeftGrabSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | OverlappingEvents")
	void OnLeftGrabSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Climbing :- (Currently Not Working in Cpp)
	// virtual void UpdateClimbingMovement(float DeltaTime) override;
	
	//Teleportation
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | InputSetup|Inputs | VR")
	void MapInput(UInputMappingContext * InputMapping, int32 Priority = 0);
	
	UFUNCTION(BlueprintCallable)
	void SpawnController();
	
	UFUNCTION(BlueprintCallable)
	void SetGripComponents(UPrimitiveComponent * LeftHand,UPrimitiveComponent* RightHand);
	
	UFUNCTION()
	void InitTeleportControllers();

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions  | Teleportation")
	void UpdateTeleportationRotations(FVector2D Input);

	UFUNCTION()
	void SetTeleportActive(EControllerHand Hand,bool Active);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions  | Teleportation")
	void ExecuteTeleportation(ATeleportController * MotionController,EControllerHand Hand);

	UFUNCTION()
	void GetCharacterRotatedPosition(FVector OriginalLocation,FRotator DeltaRotation,FVector PivotPoint,FVector &OutLocation,FRotator &OutRotation);

	UFUNCTION()
	FRotator GetCorrectRotation();
 
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | Teleportation")
	void NotifyTeleportActive(EControllerHand Hand,bool State);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | Teleportation")
	void MulticastTeleportActive(EControllerHand Hand,bool State);
	
	FTimerHandle TimerHandle;
	void DelayFunctionRunAfterFadeTime();
	// void StartDelayTimer(float time);
	
	//Snapping
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Snapping")
	bool bTurnModeIsSnap;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Snapping",meta = (EditCondition = "!bTurnModeIsSnap"))
	float TurningActivationThreshold = 0.7f;

	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Snapping", meta = (EditCondition = "!bTurnModeIsSnap"))
	float SmoothTurnSpeed = 50.0f;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement | Snapping", meta = (EditCondition = "bTurnModeIsSnap"))
	float SnapTurnAngle = 45.0f;
	
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions  | Snapping")
	void HandleTurnInput(float InputAxis);

	// Character Movement 
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | Character Movement")
	void HandleCurrentMovementInput(FVector2D MovementInput);
	
	UFUNCTION()
	void CalcPadRotationAndMagnitude(float YAxis,float XAxis,float OptMagnitudeScaler,float OptionalDeadzone,FRotator &OutRotation,float &OutMagnitude,bool &WasValid);

	UFUNCTION()
	void GetDPadMovementFacing(UGripMotionControllerComponent*CallingHand,FVector &OutForwardVector,FVector &OutRightVector);

	UFUNCTION()
	void MapThumbToWorld(UGripMotionControllerComponent*CallingHand,FRotator Rotation,FVector & OutDirection);

	UFUNCTION()
	USceneComponent *GetCorrectAimComp(UGripMotionControllerComponent*CallingHand);
	
	float ThumbX = 0.0f;
	float ThumbY = 0.f;
	float DPadVelocityScaler =  1.25f;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Properties | CharacterMovement")
	float SlidingMovementDeadZone = 0.0f;	

	UPROPERTY()
	FVector Direction = FVector(0,0,0);
	
	//Climbing
	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | Climbing")
	void InitializeClimbing(UPrimitiveComponent * NewClimbingGrip,UGripMotionControllerComponent*CallingHand);

	UFUNCTION(BlueprintCallable,Category="Ikarus Character | Functions | Climbing")
	void ExitClimbing();
	
	UPROPERTY(BlueprintReadOnly,Category="Ikarus Character | Climbing | Variables")
	FVector ClimbGripLocation;
	
	UPROPERTY(BlueprintReadOnly,Category="Ikarus Character | Climbing | Variables")
	UPrimitiveComponent * ClimbingGrip;

	UPROPERTY(BlueprintReadOnly,Category="Ikarus Character | Climbing | Variables")
	bool bIsClimbing = false;

	UPROPERTY(BlueprintReadOnly,Category="Climbing | Variables")
	UGripMotionControllerComponent * ClimbingHand;

	UPROPERTY(BlueprintReadOnly,Category="Climbing | Variables")
	UPrimitiveComponent * ClimbingWallGripRight;

	UPROPERTY(BlueprintReadOnly,Category="Climbing | Variables")
	UPrimitiveComponent * ClimbingWallGripLeft;
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Climbing | Variables",Meta=(ToolTip = "Make sure to apply the Same tag to the item you intend to ascend."))
	FName ClimbingTag = "climbable";
	
	UPROPERTY(EditDefaultsOnly,Category="Ikarus Character | Climbing | Variables",Meta=(ToolTip = "If Enabled will allow Snapping/Turning on Climbing."))
	bool bEnableSnappingWhileClimbing = false;
	
	
	// Print function.
	void Print(FString Message,int key = 1 ,FColor Color  =FColor::Red,float TimeToDisplay=3);
	void Print(int Message,int key = 1 ,FColor Color  =FColor::Red,float TimeToDisplay=3);
	void Print(FTransform Message,int key = 1 ,FColor Color  =FColor::Red,float TimeToDisplay=3,bool PrintOnlyLocation = true);
	
};

/* ********************************************************************** */