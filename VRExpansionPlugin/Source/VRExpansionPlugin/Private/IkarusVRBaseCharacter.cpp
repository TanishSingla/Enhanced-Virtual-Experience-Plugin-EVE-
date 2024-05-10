#include "IkarusVRBaseCharacter.h"
#include "BlueprintGameplayTagLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GripMotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IXRTrackingSystem.h"
#include "VRExpansionFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "VRGripInterface.h"
#include "VRRootComponent.h"
#include "NativeGameplayTags.h"
#include "VRSpectator.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/SceneCaptureComponent2D.h"


/* Gameplay Tags Declaration */

UE_DEFINE_GAMEPLAY_TAG(DropType, "DropType");
UE_DEFINE_GAMEPLAY_TAG(DropType_OnAltGrip, "DropType.OnAltGrip");
UE_DEFINE_GAMEPLAY_TAG(DropType_OnAltGripRelease,"DropType.OnAltGripRelease");
UE_DEFINE_GAMEPLAY_TAG(DropType_OnPrimaryGrip,"DropType.OnPrimaryGrip");
UE_DEFINE_GAMEPLAY_TAG(DropType_OnPrimaryGripRelease,"DropType.OnPrimaryGripRelease");
UE_DEFINE_GAMEPLAY_TAG(DropType_Secondary,"DropType.Secondary");
UE_DEFINE_GAMEPLAY_TAG(DropType_Secondary_OnAltGrip,"DropType.Secondary.OnAltGrip");
UE_DEFINE_GAMEPLAY_TAG(DropType_Secondary_OnAltGrip_Release,"DropType.Secondary.OnAltGripRelease");
UE_DEFINE_GAMEPLAY_TAG(DropType_Secondary_OnPrimaryGrip,"DropType.Secondary.OnPrimaryGrip");
UE_DEFINE_GAMEPLAY_TAG(DropType_Secondary_OnPrimaryGripRelease,"DropType.Secondary.OnPrimaryGripRelease");

UE_DEFINE_GAMEPLAY_TAG(GripSockets_DontAttachHands, "GripSockets.DontAttachHand");
UE_DEFINE_GAMEPLAY_TAG(GripSockets_SeperateHandSocket, "GripSockets.SeperateHandSockets");

UE_DEFINE_GAMEPLAY_TAG(GripType, "GripType");
UE_DEFINE_GAMEPLAY_TAG(GripType_OnAltGrip, "GripType.OnAltGrip");
UE_DEFINE_GAMEPLAY_TAG(GripType_OnPrimaryGrip, "GripType.OnPrimaryGrip");
UE_DEFINE_GAMEPLAY_TAG(GripType_Secondary, "GripType.Secondary");
UE_DEFINE_GAMEPLAY_TAG(GripType_Secondary_OnAltGrip, "GripType.Secondary.OnAltGrip");
UE_DEFINE_GAMEPLAY_TAG(GripType_Secondary_OnPrimaryGrip, "GripType.Secondary.OnPrimaryGrip");

UE_DEFINE_GAMEPLAY_TAG(Interactible_PerBoneGripping, "Interactible.PerBoneGripping");

UE_DEFINE_GAMEPLAY_TAG(Weapon_DenyFreeGripping, "Weapon.DenyFreeGripping");
UE_DEFINE_GAMEPLAY_TAG(Weapon_Gun_HeldAtGrip, "Weapon.Gun.HeldAtGrip");
UE_DEFINE_GAMEPLAY_TAG(Weapon_Gun_HeldFreely, "Weapon.Gun.HeldFreely");

/* ********************************************************************** */


AIkarusVRBaseCharacter::AIkarusVRBaseCharacter()
{
	VRMovementReference->MaxWalkSpeed = 300.0;
	VRMovementReference->MaxWalkSpeedCrouched = 150.0;
	VRMovementReference->MinAnalogWalkSpeed = 0.0;
	
	PrimaryActorTick.bCanEverTick = true;
	RightGrabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RightGrabSphere"));
	RightGrabSphere->SetupAttachment(RightMotionController);
	RightGrabSphere->SetSphereRadius(4.f);
		
	LeftGrabSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LeftGrabSphere"));
	LeftGrabSphere->SetupAttachment(LeftMotionController);
	LeftGrabSphere->SetSphereRadius(4.f);

	AimRight = CreateDefaultSubobject<USceneComponent>(TEXT("AimRight"));
	AimRight->SetupAttachment(RightMotionController);
	AimRight->SetWorldRotation(FRotator(0,0,0));

	AimLeft = CreateDefaultSubobject<USceneComponent>(TEXT("AimLeft"));
	AimLeft->SetupAttachment(LeftMotionController);
	AimLeft->SetWorldRotation(FRotator(0,0,0));


	HandMeshRight = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandMeshRight"));
	HandMeshRight->SetupAttachment(RightMotionController);

	HandMeshLeft = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandMeshLeft"));
	HandMeshLeft->SetupAttachment(LeftMotionController);
		
	this->bTurnModeIsSnap = true;

	//Binding Overlapping events
	RightGrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AIkarusVRBaseCharacter::OnRightGrabSphereBeginOverlap);
	RightGrabSphere->OnComponentEndOverlap.AddDynamic(this, &AIkarusVRBaseCharacter::OnRightGrabSphereEndOverlap);
	LeftGrabSphere->OnComponentBeginOverlap.AddDynamic(this, &AIkarusVRBaseCharacter::OnLeftGrabSphereBeginOverlap);
	LeftGrabSphere->OnComponentEndOverlap.AddDynamic(this, &AIkarusVRBaseCharacter::OnLeftGrabSphereEndOverlap);

	VRRootReference->SetCollisionResponseToChannels(ECR_Block);
	VRRootReference->SetCollisionResponseToChannel(ECC_Visibility, ECR_Overlap);
	VRRootReference->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);
	VRRootReference->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	
	//Variables Initialization :- 
	DefaultDropTag = DropType_OnPrimaryGripRelease;
	DefaultGripTag =	GripType_OnPrimaryGrip;
	DefaultSecondaryGripTag = GripType_Secondary_OnPrimaryGrip;
	DefaultSecondaryDropTag =	DropType_Secondary_OnPrimaryGripRelease;
	LeftPeakVel.VelocitySamples = 30;	
	RightPeakVel.VelocitySamples = 30;
}

void AIkarusVRBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetGripComponents(LeftGrabSphere, RightGrabSphere);

	//Camera Fade
	APlayerCameraManager *CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(),0);
	CameraManager->StartCameraFade(1.f,0.f,1.0f,FColor::White,false,false);

	//Spawn Spectator if using
	if(bEnableSmoothSpectator)
	{

		FTimerHandle SpecSpawnTimer;
		GetWorldTimerManager().SetTimer(SpecSpawnTimer, this, &ThisClass::SpawnSpectator, 1.f);
	}

	//Inputs Mapping.
	MapInput(VRInputMapping,0);

	VRMovementReference->PhysCustom_ClimbingDelegate.AddDynamic(this, &ThisClass::UpdateClimbingMovement_Binding);
		
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(TrackingOrigin);
	
	SpawnController();

	if(CheckXRApi() == "OpenXR")
	{
		SpawnGraspingHands();
		bUsingGraspingHands = true;
	}
	else
	{
		EnableControllerAnimations();
		bUsingGraspingHands = false;
	}

	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AIkarusVRBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AIkarusVRBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		//Teleportation
		EnhancedInputComponent->BindAction(IA_LeftTeleport, ETriggerEvent::Started, this, &AIkarusVRBaseCharacter::LeftTeleportStarted);
		EnhancedInputComponent->BindAction(IA_LeftTeleport, ETriggerEvent::Completed, this, &AIkarusVRBaseCharacter::LeftTeleportCompleted);

		EnhancedInputComponent->BindAction(IA_RightTeleport, ETriggerEvent::Started, this, &AIkarusVRBaseCharacter::RightTeleportStarted);
		EnhancedInputComponent->BindAction(IA_RightTeleport, ETriggerEvent::Completed, this, &AIkarusVRBaseCharacter::RightTeleportCompleted);
		
		//Right and Left Grip
		EnhancedInputComponent->BindAction(IA_RightGrip, ETriggerEvent::Started, this, &AIkarusVRBaseCharacter::RightGripStarted);
		EnhancedInputComponent->BindAction(IA_RightGrip, ETriggerEvent::Completed, this, &AIkarusVRBaseCharacter::RightGripCompleted);
		
		EnhancedInputComponent->BindAction(IA_LeftGrip, ETriggerEvent::Started, this, &AIkarusVRBaseCharacter::LeftGripStarted);
		EnhancedInputComponent->BindAction(IA_LeftGrip, ETriggerEvent::Completed, this, &AIkarusVRBaseCharacter::LeftGripCompleted);

		//Right and Left Trigger
		EnhancedInputComponent->BindAction(IA_RightTriggerAxis,ETriggerEvent::Started,this,&AIkarusVRBaseCharacter::RightTriggerStarted);
		EnhancedInputComponent->BindAction(IA_RightTriggerAxis,ETriggerEvent::Completed,this,&AIkarusVRBaseCharacter::RightTriggeredCompleted);

		EnhancedInputComponent->BindAction(IA_LeftTriggerAxis,ETriggerEvent::Started,this,&AIkarusVRBaseCharacter::LeftTriggerStarted);
		EnhancedInputComponent->BindAction(IA_LeftTriggerAxis,ETriggerEvent::Completed,this,&AIkarusVRBaseCharacter::LeftTriggeredCompleted);
		
		//Turning
		EnhancedInputComponent->BindAction(IA_Turn,ETriggerEvent::Triggered,this,&AIkarusVRBaseCharacter::HandleTurn);
		EnhancedInputComponent->BindAction(IA_Turn,ETriggerEvent::Completed,this,&AIkarusVRBaseCharacter::HandleTurn);

		//Movement
		EnhancedInputComponent->BindAction(IA_Move,ETriggerEvent::Triggered,this,&AIkarusVRBaseCharacter::HandleMove);
		EnhancedInputComponent->BindAction(IA_Move,ETriggerEvent::Completed,this,&AIkarusVRBaseCharacter::HandleMove);

		EnhancedInputComponent->BindAction(IA_LeftThumbstick,ETriggerEvent::Triggered,this,&AIkarusVRBaseCharacter::HandleLeftTeleportedRotation);
		EnhancedInputComponent->BindAction(IA_RightThumbstick,ETriggerEvent::Triggered,this,&AIkarusVRBaseCharacter::HandleRightTeleportedRotation);

		//LaserBeam
		EnhancedInputComponent->BindAction(IA_LaserBeamRight,ETriggerEvent::Started,this,&AIkarusVRBaseCharacter::HandleLaserBeamRight);
		EnhancedInputComponent->BindAction(IA_LaserBeamLeft,ETriggerEvent::Started,this,&AIkarusVRBaseCharacter::HandleLaserBeamLeft);
	}
}

void AIkarusVRBaseCharacter::MapInput(UInputMappingContext * InputMapping,int32 Priority)
{
	if(const APlayerController * PlayerController = UGameplayStatics::GetPlayerController(GetWorld(),0))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping,Priority);
		}
	}
}

void AIkarusVRBaseCharacter::LeftTeleportStarted()
{
	// For left Controller
	if(!bIsClimbing && TeleportControllerClass && bEnableTeleportation && !bIsRightTeleportInputActive)
	{
		SetTeleportActive(EControllerHand::Left,true);
		bIsLeftTeleportInputActive = true;
	}
}

void AIkarusVRBaseCharacter::LeftTeleportCompleted()
{
	if(!bIsClimbing && TeleportControllerClass && bEnableTeleportation && !bIsRightTeleportInputActive)
	{
		ExecuteTeleportation(TeleportControllerLeft,EControllerHand::Left);
		bIsLeftTeleportInputActive = false;
	}
}

void AIkarusVRBaseCharacter::RightTeleportStarted()
{
	// For left Controller
	if(!bIsClimbing && TeleportControllerClass && bEnableTeleportation && !bIsLeftTeleportInputActive)
	{
		SetTeleportActive(EControllerHand::Right,true);
		bIsRightTeleportInputActive = true;
	}
}

void AIkarusVRBaseCharacter::RightTeleportCompleted()
{
	if(!bIsClimbing && TeleportControllerClass && bEnableTeleportation && !bIsLeftTeleportInputActive)
	{
		ExecuteTeleportation(TeleportControllerRight,EControllerHand::Right);
		bIsRightTeleportInputActive = false;
	}
}

void AIkarusVRBaseCharacter::RightGripStarted()
{
	if(IsValid(ClimbingWallGripRight))
	{
		if(IsValid(RightMotionController))
		{
			InitializeClimbing(ClimbingWallGripRight,RightMotionController);
		}
	}
	TriggerGripOrDrop(RightMotionController,LeftMotionController,true,RightGrabSphere);
	
}

void AIkarusVRBaseCharacter::RightGripCompleted()
{
	if(IsValid(RightMotionController) && IsValid(ClimbingHand) && (RightMotionController==ClimbingHand) && bIsClimbing)
	{
		ExitClimbing();
	}
	TriggerGripOrDrop(RightMotionController,LeftMotionController,false,RightGrabSphere);
	
}

void AIkarusVRBaseCharacter::LeftGripStarted()
{
	if(IsValid(ClimbingWallGripLeft))
	{
		if(IsValid(LeftMotionController))
		{
			InitializeClimbing(ClimbingWallGripLeft,LeftMotionController);
		}
	}
	TriggerGripOrDrop(LeftMotionController,RightMotionController,true,LeftHandGripComponent);
	
}

void AIkarusVRBaseCharacter::LeftGripCompleted()
{
	if(IsValid(LeftMotionController) && IsValid(ClimbingHand) && (LeftMotionController==ClimbingHand) && bIsClimbing)
	{
		ExitClimbing();
	}
	TriggerGripOrDrop(LeftMotionController,RightMotionController,false,LeftHandGripComponent);
	
}

void AIkarusVRBaseCharacter::HandleTurn(const FInputActionValue& InputAxis)
{
	if(bEnableCharacterSnapping && !TeleportControllerLeft->bIsTeleporterActive && !TeleportControllerRight->bIsTeleporterActive)
	{
		HandleTurnInput(InputAxis.GetMagnitude());
	}
}

void AIkarusVRBaseCharacter::HandleMove(const FInputActionValue& Input)
{
	if(bEnableCharacterSmoothMovement)
	{
		HandleCurrentMovementInput(Input.Get<FVector2d>());
	}
}

void AIkarusVRBaseCharacter::HandleLeftTeleportedRotation(const FInputActionValue& Input)
{
	if(TeleportControllerRight) UpdateTeleportationRotations(TeleportControllerRight, Input.Get<FVector2d>());
}

void AIkarusVRBaseCharacter::HandleRightTeleportedRotation(const FInputActionValue& Input)
{
	if(TeleportControllerLeft) UpdateTeleportationRotations(TeleportControllerLeft, Input.Get<FVector2d>());
}

void AIkarusVRBaseCharacter::HandleLaserBeamRight()
{
	if(bEnableLaserBeam && IsValid(TeleportControllerRight))
	{
		TeleportControllerRight->SetLaserBeamActive(!TeleportControllerRight->bIsLaserBeamActive);
	}
}

void AIkarusVRBaseCharacter::HandleLaserBeamLeft()
{
	if(bEnableLaserBeam && IsValid(TeleportControllerLeft))
	{
		TeleportControllerLeft->SetLaserBeamActive(!TeleportControllerLeft->bIsLaserBeamActive);
	}
}

void AIkarusVRBaseCharacter::RightTriggerStarted()
{
	const bool Check = IfOverWidgetUse(RightMotionController,true);
	if(!Check)
	{
		CheckUseHeldItems(RightMotionController,true);
		bool OutDropped = false;
		bool OutHadSecondary = false;
		CheckUseSecondaryAttachment(RightMotionController,LeftMotionController,true,OutDropped,OutHadSecondary);
	}

	if(TeleportControllerRight)
	{
		TeleportControllerRight->GrabTrigger();
	}
}

void AIkarusVRBaseCharacter::RightTriggeredCompleted()
{
	const bool Check = IfOverWidgetUse(RightMotionController,false);
	if(!Check)
	{
		CheckUseHeldItems(RightMotionController,false);
		bool OutDropped = false;
		bool OutHadSecondary = false;
		CheckUseSecondaryAttachment(RightMotionController,LeftMotionController,false,OutDropped,OutHadSecondary);
	}
}

void AIkarusVRBaseCharacter::LeftTriggerStarted()
{
	const bool Check = IfOverWidgetUse(LeftMotionController,true);
	if(!Check)
	{
		CheckUseHeldItems(LeftMotionController,true);
		bool OutDropped = false;
		bool OutHadSecondary = false;
		CheckUseSecondaryAttachment(LeftMotionController,RightMotionController,true,OutDropped,OutHadSecondary);
	}

	if(TeleportControllerLeft)
	{
		TeleportControllerLeft->GrabTrigger();
	}
}

void AIkarusVRBaseCharacter::LeftTriggeredCompleted()
{
	const bool Check = IfOverWidgetUse(LeftMotionController,false);
	if(!Check)
	{
		CheckUseHeldItems(LeftMotionController,false);
		bool OutDropped = false;
		bool OutHadSecondary = false;
		CheckUseSecondaryAttachment(LeftMotionController,RightMotionController,false,OutDropped,OutHadSecondary);
	}
}

bool AIkarusVRBaseCharacter::TryToGrabObject(UObject* ObjectToTryToGrab, FTransform WorldTransform,
                                             UGripMotionControllerComponent* Hand, UGripMotionControllerComponent* OtherHand, bool bIsSlotGrip,
                                             FGameplayTag GripSecondaryTag, FName GripBoneName, FName SlotName, bool IsSecondaryGrip)
{
	bool ImplementsInterface = false;
	if (ObjectToTryToGrab->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		ImplementsInterface = true;
	}

	if(Hand->GetIsObjectHeld(ObjectToTryToGrab))
	{
		// if already held by this hand.
		return false;
	}

	if(OtherHand->GetIsObjectHeld(ObjectToTryToGrab))
	{
		//if other hand is holding the object already.
		//then we will check...
		// If we are allowing multiple grips., then we will grip, else we will check for hand swap or secondary grip..
		if(const IVRGripInterface * AsVRGripInterface = Cast<IVRGripInterface>(ObjectToTryToGrab); !AsVRGripInterface)
		{
			//Cast failed
			if(!(bIsSlotGrip && !bIsSecondaryGrip))
			{
				if(TryToSecondaryGripObject(Hand,OtherHand,ObjectToTryToGrab,GripSecondaryTag,ImplementsInterface,WorldTransform,SlotName,bIsSlotGrip))
				{
					return true;
				}
			}
			//Drop Object
			OtherHand->DropObject(ObjectToTryToGrab);
		}else
		{
			//Cast successful
			
			if(!(AsVRGripInterface->Execute_AllowsMultipleGrips(ObjectToTryToGrab)))
			{
				// TODO 
				if((bIsSlotGrip && !bIsSecondaryGrip))
				{
					if(TryToSecondaryGripObject(Hand,OtherHand,ObjectToTryToGrab,GripSecondaryTag,ImplementsInterface,WorldTransform,SlotName,bIsSlotGrip))
						return true;
				}

				/*
				 if you are facing gripping related bugs then use the commented functions below and comment the rest. Good Luck!
				 */
				
				//Drop Object
				// OtherHand->DropObject(ObjectToTryToGrab);
				
				FGameplayTagContainer RelevantGameplayTagCont;
				RelevantGameplayTagCont.AddTag(DropType_OnPrimaryGripRelease);
				RelevantGameplayTagCont.AddTag(DropType_Secondary_OnPrimaryGripRelease);
				DropItems(OtherHand, RelevantGameplayTagCont);
			}
		}
	}else
	{
		if(bIsSecondaryGrip)
		{
			return false;
		}
		if(const IVRGripInterface * AsVRGripInterface = Cast<IVRGripInterface>(ObjectToTryToGrab))
		{
			bool bIsHeld;
			TArray<FBPGripPair>HoldingControllers;
			AsVRGripInterface->Execute_IsHeld(ObjectToTryToGrab,HoldingControllers,bIsHeld);
			if(bIsHeld)
			{
				if(!AsVRGripInterface->Execute_AllowsMultipleGrips(ObjectToTryToGrab))
				{
					return false;
				}
			}
		}
	}
	if(ImplementsInterface)
	{
		RumbleController(Hand,GrabHapticEffect,IntensityHapticEffect);

		EControllerHand HandType;
		Hand->GetHandType(HandType);
		switch (HandType)
		{
		case EControllerHand::Left:
			if(TeleportControllerLeft) TeleportControllerLeft->SetLaserBeamActive(false);

		case EControllerHand::Right:
			if(TeleportControllerRight) TeleportControllerRight->SetLaserBeamActive(false);
			
		default: ;
		}
		
		return (Hand->GripObjectByInterface(ObjectToTryToGrab,WorldTransform,true,GripBoneName,SlotName,bIsSlotGrip));
	}
		
	RumbleController(Hand,GrabHapticEffect,IntensityHapticEffect);

	EControllerHand HandType;
	Hand->GetHandType(HandType);
	switch (HandType)
	{
	case EControllerHand::Left:
		if(TeleportControllerLeft) TeleportControllerLeft->SetLaserBeamActive(false);

	case EControllerHand::Right:
		if(TeleportControllerRight) TeleportControllerRight->SetLaserBeamActive(false);
			
	default: ;
	}
	
	return (Hand->GripObject(ObjectToTryToGrab,WorldTransform,true,SlotName,GripBoneName,EGripCollisionType::InteractiveCollisionWithPhysics,EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping,EGripMovementReplicationSettings::ForceClientSideMovement,2250.0,140.0,bIsSlotGrip));
}

bool AIkarusVRBaseCharacter::TryToSecondaryGripObject(UGripMotionControllerComponent* Hand,
	UGripMotionControllerComponent* OtherHand, UObject* ObjectToTryToGrab, FGameplayTag GripSecondaryTag,
	bool ObjectImplementsInterface, FTransform RelativeSecondaryTransform, FName SlotName, bool bHadSlot)	
{
	if(!ObjectImplementsInterface)
	{
		return false;
	}
	if(const IVRGripInterface *GripInterface = Cast<IVRGripInterface>(ObjectToTryToGrab))
	{
		if(CanSecondaryGripObject(Hand,OtherHand,ObjectToTryToGrab,GripSecondaryTag,bHadSlot,GripInterface->Execute_SecondaryGripType(ObjectToTryToGrab)))
		{
			return OtherHand->AddSecondaryAttachmentPoint(ObjectToTryToGrab,Hand,RelativeSecondaryTransform,true,0.25,bHadSlot,SlotName);
		}
	}
	return false;
}

void AIkarusVRBaseCharacter::GetNearestOverlappingObject(UPrimitiveComponent* OverlapComponent,
	UGripMotionControllerComponent* Hand, FGameplayTagContainer RelevantGameplayTags, UObject*& NearestObject,
	bool& bImplementsInterface, FTransform& ObjectTransform, bool& bCanBeClimbed, FName &BoneName, FVector& ImpactLoc)
{
	if(!IsValid(OverlapComponent))
	{
		return;
	}
	
	UPrimitiveComponent * HitComponent = nullptr;
	int LastGripPrio = 0;
	UObject * NearestOverlappingObject =  nullptr;
	bool ImplementsVRGrip = false;
	FTransform WorldTransform;
	FName NearestBoneName = FName("None");
	FVector ImpactPoint = FVector(0,0,0);
	
	if(bForceOverlapOnlyGripChecks)
	{
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); 
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody)); 
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Vehicle)); 
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible)); 
	
		TArray<AActor*>ActorsToIgnore;
		ActorsToIgnore.Add(this);
		UClass * ComponentClassFilter = nullptr;
		TArray<UPrimitiveComponent*>OutComponents;

		if(UKismetSystemLibrary::ComponentOverlapComponents(OverlapComponent,OverlapComponent->GetComponentTransform(),ObjectTypes,ComponentClassFilter,ActorsToIgnore,OutComponents))
		{
			for(int i = 0;i<OutComponents.Num();i++)
			{
				if(i==0)HitComponent = OutComponents[i];
				if(HasValidGripCollision(OutComponents[i]))
				{
					bool bShouldGrip = false;
					UObject * ObjectToGrip = nullptr;
					bool ObjectImplementsInterface = false;
					FTransform ObjectsWorldTransform;
					int GripPrio = 0;
					ShouldGripComponent(OutComponents[i],LastGripPrio,(i>0),FName("None"),RelevantGameplayTags,Hand,bShouldGrip,ObjectToGrip,ObjectImplementsInterface,ObjectsWorldTransform,GripPrio);
					if(bShouldGrip)
					{
						NearestOverlappingObject = ObjectToGrip;
						ImplementsVRGrip = ObjectImplementsInterface;
						WorldTransform = ObjectsWorldTransform;
						ImpactPoint = OverlapComponent->GetComponentLocation();
						LastGripPrio = GripPrio;
					}
				}
			}
		}
	}else
	{
		TArray<AActor*>GrippedActorArray;
		Hand->GetGrippedActors(GrippedActorArray);

		FVector Origin;
		FVector BoxExtent;
		float Radius = 0.0f;
		UKismetSystemLibrary::GetComponentBounds(OverlapComponent,Origin,BoxExtent,Radius);

		TArray<FHitResult>  OutHit;
		FVector Calculation = UKismetMathLibrary::Multiply_VectorFloat(UKismetMathLibrary::GetForwardVector(GetCorrectAimComp(Hand)->GetComponentRotation()),GripTraceLength);
		if(UKismetSystemLibrary::SphereTraceMulti(GetWorld(),UKismetMathLibrary::Subtract_VectorVector(Origin,Calculation),UKismetMathLibrary::Add_VectorVector(Calculation,Origin),Radius,CollisionChannelTraceTypeQuery,true,GrippedActorArray,EDrawDebugTrace::None,OutHit,true))
		{
			bool bShouldGrip = false;
			bool bObjectImplementsInterface = false;
			UObject * OutObjectToGrip = nullptr;
			FTransform OutWorldTransform;
			UPrimitiveComponent * OutFirstPrimitiveHit = nullptr;
			FName OutBoneName;
			FVector OutImpactPoint;
			SelectObjectFromHitArray(OutHit,RelevantGameplayTags,Hand,bShouldGrip,bObjectImplementsInterface,OutObjectToGrip,OutWorldTransform,OutFirstPrimitiveHit,OutBoneName,OutImpactPoint);
				if(bShouldGrip)
				{
					NearestOverlappingObject = OutObjectToGrip;
					ImplementsVRGrip = bObjectImplementsInterface;
					WorldTransform = OutWorldTransform;
					ImpactPoint = OutImpactPoint;
					if(OutObjectToGrip->GetClass()->ImplementsInterface(UGameplayTagAssetInterface::StaticClass()))
					{
						IGameplayTagAssetInterface *AssetInt = Cast<IGameplayTagAssetInterface>(OutObjectToGrip);
						if(AssetInt && AssetInt->HasMatchingGameplayTag(Interactible_PerBoneGripping))
						{
							NearestBoneName = BoneName;
						}else
						{
							NearestBoneName = FName("None");
						}
					}else
					{
						NearestBoneName = BoneName;
					}
				}
				else
				{
					HitComponent = OutFirstPrimitiveHit;
				}
		}else
		{
			TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); 
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody)); 
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Vehicle)); 
			ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Destructible)); 
	
			TArray<AActor*>ActorsToIgnore;
			ActorsToIgnore.Add(this);
			UClass * ComponentClassFilter = nullptr;
			TArray<UPrimitiveComponent*>OutComponents;

			if(UKismetSystemLibrary::ComponentOverlapComponents(OverlapComponent,OverlapComponent->GetComponentTransform(),ObjectTypes,ComponentClassFilter,ActorsToIgnore,OutComponents))
			{
				for(int i = 0;i<OutComponents.Num();i++)
				{
					if(i==0)HitComponent = OutComponents[i];
					if(HasValidGripCollision(OutComponents[i]))
					{
						bool bShouldGrip = false;
						UObject * ObjectToGrip = nullptr;
						bool ObjectImplementsInterface = false;
						FTransform ObjectsWorldTransform;
						int GripPrio = 0;
						ShouldGripComponent(OutComponents[i],LastGripPrio,(i>0),FName("None"),RelevantGameplayTags,Hand,bShouldGrip,ObjectToGrip,ObjectImplementsInterface,ObjectsWorldTransform,GripPrio);
						if(bShouldGrip)
						{
							NearestOverlappingObject = ObjectToGrip;
							ImplementsVRGrip = ObjectImplementsInterface;
							WorldTransform = ObjectsWorldTransform;
							ImpactPoint = OverlapComponent->GetComponentLocation();
							LastGripPrio = GripPrio;
						}
					}
				}
			}
		}
	}
	NearestObject  = NearestOverlappingObject;
	NearestOverlappingObjectValue = NearestObject;
	bImplementsInterface = ImplementsVRGrip;
	ObjectTransform = WorldTransform;
	bCanBeClimbed = false;
	BoneName  = NearestBoneName;
	ImpactLoc = ImpactPoint;
}

bool AIkarusVRBaseCharacter::HasValidGripCollision(UPrimitiveComponent* Component)
{
	const ECollisionResponse CollisionResp = Component->GetCollisionResponseToChannel(CollisionChannel);
	if(CollisionResp == ECollisionResponse::ECR_Ignore)return false;
	return true;
}


void AIkarusVRBaseCharacter::SelectObjectFromHitArray(TArray<FHitResult> Hits,
	FGameplayTagContainer RelevantGameplayTags, UGripMotionControllerComponent* Hand, bool& bShouldGrip,
	bool& ObjectImplementsInterface, UObject*& ObjectToGrip, FTransform& WorldTransform,
	UPrimitiveComponent* &FirstPrimitiveHit, FName& BoneName, FVector& ImpactPoint)
{
	bool LShouldGrip = false;
	bool LObjectImplementsInterface = false;
	UObject * LOutObject = nullptr;
	FTransform LOutTransform;
	UPrimitiveComponent * OutFirstHitPrimitive = nullptr;
	FName LOutBoneName;
	FVector LImpactPoint;
	
	for(int i = 0;i<Hits.Num();i++)
	{
		if(IsValid(Hits[i].GetComponent()))
		{
			if(i==0)
			{
				OutFirstHitPrimitive = Hits[i].GetComponent();
			}

			bool TempShouldGrip = false;
			int TempBestGripPrio = 0;
			bool TempObjectImplementsInterface = false;
			UObject * TempOutObject = nullptr;
			FTransform TempOutTransform;
			ShouldGripComponent(Hits[i].GetComponent(),TempBestGripPrio,i>0,Hits[i].BoneName,RelevantGameplayTags,Hand,TempShouldGrip,TempOutObject,TempObjectImplementsInterface,TempOutTransform,TempBestGripPrio);
			if(TempShouldGrip)
			{
				const FName TempOutBoneName;
				LOutObject = TempOutObject;
				LOutTransform = TempOutTransform;
				LObjectImplementsInterface = TempObjectImplementsInterface;
				LShouldGrip = TempShouldGrip;
				LOutBoneName = TempOutBoneName;
				LImpactPoint = Hits[i].Location;
			}
		}
	}
	bShouldGrip = LShouldGrip;
	ObjectImplementsInterface = LObjectImplementsInterface;
	ObjectToGrip = LOutObject;
	WorldTransform  = LOutTransform;
	FirstPrimitiveHit = OutFirstHitPrimitive;
	BoneName = LOutBoneName;
	ImpactPoint = LImpactPoint;
}

bool AIkarusVRBaseCharacter::ValidateGameplayTagContainer(FGameplayTag BaseTag, UObject* Object,
	FGameplayTag DefaultTag, FGameplayTagContainer GameplayTags)
{
	if(const IGameplayTagAssetInterface * GameplayTagAssetInterface  = Cast<IGameplayTagAssetInterface>(Object))
	{
		if(GameplayTagAssetInterface->HasMatchingGameplayTag(BaseTag))
		{
			FGameplayTagContainer TagContainer;
			GameplayTagAssetInterface->GetOwnedGameplayTags(TagContainer);
			return UVRExpansionFunctionLibrary::MatchesAnyTagsWithDirectParentTag(BaseTag,GameplayTags,TagContainer);
		}
	}
	return UBlueprintGameplayTagLibrary::HasTag(GameplayTags,DefaultTag,true);
}

UObject* AIkarusVRBaseCharacter::GetNearestOverlapOfHand(UGripMotionControllerComponent* Hand,
	UPrimitiveComponent* OverlapSphere)
{
	TArray<UPrimitiveComponent*>OverlappingComponents;
	OverlapSphere->GetOverlappingComponents(OverlappingComponents);
	UObject * NearestOverlapObject = nullptr;
	float NearestOverlap = 100000000.0;

	for(const auto x : OverlappingComponents)
	{
		FVector Calc = UKismetMathLibrary::Subtract_VectorVector(x->GetComponentLocation(),OverlapSphere->GetComponentLocation());
		if(UKismetMathLibrary::Vector4_SizeSquared3(Calc) < NearestOverlap)
		{
			NearestOverlapObject = x;
			NearestOverlap = Calc.Length();
		}else
		{
			return NearestOverlapObject;
		}
	}
	return NearestOverlapObject;
}

void AIkarusVRBaseCharacter::TryRemoveSecondaryAttachment(UGripMotionControllerComponent* CallingMotionController,
	UGripMotionControllerComponent* OtherController, FGameplayTagContainer GameplayTags, bool& DroppedSecondary,
	bool& HadSecondary)
{
	FBPActorGripInformation OutGripInfo;
	if(OtherController->GetIsSecondaryAttachment(CallingMotionController,OutGripInfo))
	{
		DefaultSecondaryDropTag = DropType_Secondary_OnPrimaryGripRelease;
		if(ValidateGameplayTagContainer(DropType_Secondary,OutGripInfo.GrippedObject,DefaultSecondaryDropTag,GameplayTags))
		{
			OtherController->RemoveSecondaryAttachmentPoint(OutGripInfo.GrippedObject);
			DroppedSecondary = true;
			HadSecondary = true;
			return;
		}else
		{
			DroppedSecondary = false;
			HadSecondary = true;
			return;
		}
		// OutGripInfo.GrippedObject	
	}else
	{
		DroppedSecondary = false;
		HadSecondary = false;
		return;
	}
}

bool AIkarusVRBaseCharacter::IsALocalGrip(EGripMovementReplicationSettings GripRepType)
{
	return (GripRepType == EGripMovementReplicationSettings::ClientSide_Authoritive || GripRepType == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep);
}

bool AIkarusVRBaseCharacter::ValidateGameplayTag(FGameplayTag BaseTag, FGameplayTag GameplayTag, UObject* Object,
	FGameplayTag DefaultTag)
{
	if(UBlueprintGameplayTagLibrary::IsGameplayTagValid(GameplayTag))
	{
		if(const IGameplayTagAssetInterface * GameplayTagAssetInterface  = Cast<IGameplayTagAssetInterface>(Object))
		{
			if(GameplayTagAssetInterface->HasMatchingGameplayTag(BaseTag))
			{
				return GameplayTagAssetInterface->HasMatchingGameplayTag(GameplayTag);
			}
		}
	}
	return GameplayTag == DefaultTag;
}

void AIkarusVRBaseCharacter::DropSecondaryAttachment(UGripMotionControllerComponent* CallingMotionController,
	UGripMotionControllerComponent* OtherController, FGameplayTagContainer GameplayTags,bool &DroppedSecondary,bool &HadSecondary)
{
	FBPActorGripInformation OutGripInfo ;
	if(!(OtherController->GetIsSecondaryAttachment(CallingMotionController,OutGripInfo)))
	{
		DroppedSecondary = false;
		HadSecondary = false;
		return;
	}

	if(ValidateGameplayTagContainer(DropType_Secondary,OutGripInfo.GrippedObject,DefaultSecondaryDropTag,GameplayTags))
	{
		if(IsALocalGrip(OutGripInfo.GripMovementReplicationSetting))
		{
			OtherController->RemoveSecondaryAttachmentPoint(OutGripInfo.GrippedObject);
			HadSecondary = true;
			DroppedSecondary = true;
		}else
		{
			HadSecondary = true;
			DroppedSecondary = true;
			RemoveSecondaryGrip(OtherController,OutGripInfo.GrippedObject);
		}
	}else
	{
		DroppedSecondary = false;
		HadSecondary = true;
	}
}

void AIkarusVRBaseCharacter::ShouldSocketGrip(FBPActorGripInformation Grip, bool& ShouldSocket,
	USceneComponent*& SocketParent, FTransform_NetQuantize& RelativeTransform, FName& OptionalSocketName)
{
	if(const IVRGripInterface *GripInterface =  Cast<IVRGripInterface>(Grip.GrippedObject))
	{
		ShouldSocket = GripInterface->Execute_RequestsSocketing(Grip.GrippedObject,SocketParent,OptionalSocketName,RelativeTransform);
	}else
	{
		ShouldSocket = false;
		OptionalSocketName = "None";
	}
}

void AIkarusVRBaseCharacter::TryDropSingleClient(UGripMotionControllerComponent* Controlller,
                                                 FBPActorGripInformation GripToDrop, FVector AngleVel, FVector LinearVel){
	
	bool bShouldSocketGrip = false;
	USceneComponent * OutSocketParent = nullptr;
	FTransform_NetQuantize OutRelativeTransform;
	FName OutOptionSocketName;
	ShouldSocketGrip(GripToDrop,bShouldSocketGrip,OutSocketParent,OutRelativeTransform,OutOptionSocketName);

	if(bShouldSocketGrip)
	{
		Controlller->DropAndSocketGrip(GripToDrop,OutSocketParent,OutOptionSocketName,OutRelativeTransform,true);
	}
	else
	{
		if(IsValid(GripToDrop.GrippedObject))
		{
			if(GripToDrop.GrippedObject.GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
			{
				Controlller->DropObjectByInterface(nullptr,GripToDrop.GripID,AngleVel,LinearVel);
			}else
			{
				Controlller->DropGrip(GripToDrop,true,AngleVel,LinearVel);
			}
		}
	}
}

void AIkarusVRBaseCharacter::TryDropSingle(UGripMotionControllerComponent* Controlller, FVector_NetQuantize AngleVel,
	FVector_NetQuantize LinearVel, int Hash)
{
	FBPActorGripInformation Grip;
	EBPVRResultSwitch Result;
	Controlller->GetGripByID(Grip,Hash,Result);
	if (Result == EBPVRResultSwitch::OnSucceeded)
	{
		TryDropSingleClient(Controlller, Grip,AngleVel,LinearVel);
	}
}

void AIkarusVRBaseCharacter::CheckGripPriority(UObject* ObjectToCheck, int PrioToCheckAgainst, bool CheckAgainstPrior,
	bool& HadHigherPriority, int& NewGripPrio)
{
	const IVRGripInterface * GripInterface = Cast<IVRGripInterface>(ObjectToCheck);
	if(CheckAgainstPrior)
	{
		if(GripInterface)
		{
			const FBPAdvGripSettings Settings = GripInterface->Execute_AdvancedGripSettings(ObjectToCheck);
			HadHigherPriority = Settings.GripPriority>PrioToCheckAgainst;
			NewGripPrio = Settings.GripPriority;
		}
	}else
	{
		if(GripInterface)
		{
			const FBPAdvGripSettings Settings = GripInterface->Execute_AdvancedGripSettings(ObjectToCheck);
			HadHigherPriority = true;
			NewGripPrio = Settings.GripPriority;
		}
	}
}

bool AIkarusVRBaseCharacter::CheckIsValidForGripping(UObject* Object, FGameplayTagContainer RelevantGameplayTags)
{
	bool OutCanAttemptSecondaryOnGrab = false;
	ESecondaryGripType OutSecondaryGripType = ESecondaryGripType::SG_None;
	CanAttemptSecondaryGrabOnObject(Object,OutCanAttemptSecondaryOnGrab,OutSecondaryGripType);

	if(OutCanAttemptSecondaryOnGrab)
	{
		if(ValidateGameplayTagContainer(GripType_Secondary,Object,DefaultSecondaryGripTag,RelevantGameplayTags)){
			return true;
		}
	}
		DefaultSecondaryGripTag = GripType_Secondary_OnPrimaryGrip;
	if(ValidateGameplayTagContainer(GripType,Object,DefaultSecondaryGripTag,RelevantGameplayTags)){
		return true;
	}
	return false;
}

void AIkarusVRBaseCharacter::CanAttemptSecondaryGrabOnObject(UObject* ObjectToCheck, bool& CanAttemptSecondaryGrab,
	ESecondaryGripType &SecondaryGripType)
{
	if(const IVRGripInterface *GripInterface  = Cast<IVRGripInterface>(ObjectToCheck))
	{
		TArray<FBPGripPair> OutHoldingController;
		bool OutIsHeld = false;
		GripInterface->Execute_IsHeld(ObjectToCheck,OutHoldingController,OutIsHeld);

		if(!OutIsHeld)
		{
			CanAttemptSecondaryGrab = false;
			SecondaryGripType = ESecondaryGripType::SG_None;
			return;
		}
		SecondaryGripType = GripInterface->Execute_SecondaryGripType(ObjectToCheck);
		CanAttemptSecondaryGrab = OutHoldingController.Num()> 0 && SecondaryGripType != ESecondaryGripType::SG_None && (OutHoldingController[0].HoldingController)->GetOwner() == this;
	}
}

void AIkarusVRBaseCharacter::ShouldGripComponent(UPrimitiveComponent* ComponentToCheck, int GripPrioToCheckAgainst,
	bool bCheckAgainstPrior, FName BoneName, FGameplayTagContainer RelevantGameplayTags,
	UGripMotionControllerComponent* CallingController, bool& ShouldGrip, UObject* &ObjectToGrip,
	bool& ObjectImplementsInterface, FTransform& ObjectsWorldTransform, int& GripPrio)
{
	if(!HasValidGripCollision(ComponentToCheck))
	{
		ShouldGrip = false;
		ObjectImplementsInterface = false;
		return;
	}
	bool ImplementsInterface = ComponentToCheck->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass());

	const IVRGripInterface * GripInterface = Cast<IVRGripInterface>(ComponentToCheck);
	
	if(ImplementsInterface)
	{
		if(GripInterface)
		{
			if(GripInterface->Execute_SecondaryGripType(ComponentToCheck) == ESecondaryGripType::SG_None && GripInterface->Execute_DenyGripping(ComponentToCheck,CallingController))
			{
				ShouldGrip = false;
				ObjectImplementsInterface = false;
				return;
			}else
			{
				bool OutHadHigherPriority = false;
				int OutNewGripPrio;
				CheckGripPriority(ComponentToCheck,GripPrioToCheckAgainst,bCheckAgainstPrior,OutHadHigherPriority,OutNewGripPrio);
				if(OutHadHigherPriority)
				{
					if(CheckIsValidForGripping(ComponentToCheck,RelevantGameplayTags))
					{
						ShouldGrip = true;
						ObjectToGrip = ComponentToCheck;
						ObjectImplementsInterface = ImplementsInterface;
						ObjectsWorldTransform = ComponentToCheck->GetComponentTransform();
						GripPrio = OutNewGripPrio;
						return;
					}
				}
			}
		}
	}else
	{
		AActor * OwningActor = ComponentToCheck->GetOwner();

		if(!IsValid(OwningActor) || OwningActor == this)
		{
			ShouldGrip = false;
			ObjectImplementsInterface = false;
			return;
		}

		ImplementsInterface = OwningActor->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass());
		if(ImplementsInterface)
		{
			if(const IVRGripInterface * GripInterfaceOwningActor = Cast<IVRGripInterface>(OwningActor))
			{
				if(GripInterfaceOwningActor->Execute_SecondaryGripType(OwningActor) == ESecondaryGripType::SG_None && GripInterfaceOwningActor->Execute_DenyGripping(OwningActor,CallingController))
				{
					ShouldGrip = false;
					ObjectImplementsInterface = false;
					return;
				}
				bool OutHadHigherPriority  = false;
				int OutNewGripPrio = 0;
				CheckGripPriority(OwningActor,GripPrioToCheckAgainst,bCheckAgainstPrior,OutHadHigherPriority,OutNewGripPrio);
				if(!OutHadHigherPriority)
				{
					return;            
				}

				if(CheckIsValidForGripping(OwningActor,RelevantGameplayTags))
				{
					ShouldGrip = true;
					ObjectToGrip = OwningActor;
					ObjectImplementsInterface = ImplementsInterface;
					ObjectsWorldTransform = OwningActor->GetActorTransform();
					GripPrio = OutNewGripPrio;
				}
			}
		}else
		{
			if(bCheckAgainstPrior)return;

			if(ComponentToCheck->IsSimulatingPhysics(BoneName))
			{
				ShouldGrip = true;
				ObjectToGrip = ComponentToCheck;
				ObjectImplementsInterface = ImplementsInterface;
				ObjectsWorldTransform = ComponentToCheck->GetComponentTransform();
				GripPrio = 0;
			}
		}
	}
}

void AIkarusVRBaseCharacter::CheckUseSecondaryAttachment(UGripMotionControllerComponent* CallingMotionController,
	UGripMotionControllerComponent* OtherController, bool ButtonPressed, bool& DroppedOrUsedSecondary,
	bool& HadSecondary)
{
	FBPActorGripInformation OutGripInfo;
	if(OtherController->GetIsSecondaryAttachment(CallingMotionController,OutGripInfo))
	{
		if(ButtonPressed)
		{
			// If button is pressed
			if(const IVRGripInterface *GripInterface = Cast<IVRGripInterface>(OutGripInfo.GrippedObject))
			{
				GripInterface->Execute_OnSecondaryUsed(OutGripInfo.GrippedObject);
				HadSecondary = true;
				DroppedOrUsedSecondary = true;
			}else
			{
				DroppedOrUsedSecondary = false;
				HadSecondary = true;
			}
		}else
		{
			if(const IVRGripInterface *AsGripInterface = Cast<IVRGripInterface>(OutGripInfo.GrippedObject))
			{
				AsGripInterface->Execute_OnEndSecondaryUsed(OutGripInfo.GrippedObject);
				DroppedOrUsedSecondary = true;
				HadSecondary = true;
			}
			else
			{
				HadSecondary = true;
				DroppedOrUsedSecondary = false;
			}
		}
	}
}

void AIkarusVRBaseCharacter::CheckUseHeldItems(UGripMotionControllerComponent* Hand, bool ButtonState)
{
	TArray<FBPActorGripInformation>OutGripArray;
	Hand->GetAllGrips(OutGripArray);

	//Reverse for loop
	for(int i = OutGripArray.Num()-1; i >= 0; i--)
	{
		if(IsValid(OutGripArray[i].GrippedObject))
		{
			const IVRGripInterface* AsGripInterface = Cast<IVRGripInterface>(OutGripArray[i].GrippedObject);
			if(ButtonState)
			{
				if(AsGripInterface)
				{
					AsGripInterface->Execute_OnUsed(OutGripArray[i].GrippedObject);
				}
			}else
			{
				if(AsGripInterface)
				{
					AsGripInterface->Execute_OnEndUsed(OutGripArray[i].GrippedObject);
				}
			}
		}
	}
}

FTransform AIkarusVRBaseCharacter::RemoveControllerScale(FTransform SocketTransform,
	UGripMotionControllerComponent* GrippingController)
{
	FTransform Out;
	Out.SetScale3D(UKismetMathLibrary::Divide_VectorVector(FVector(1.0,1.0,1.0),GrippingController->GetPivotTransform().GetScale3D()));
	return Out;
}

bool AIkarusVRBaseCharacter::CanAttemptGrabOnObject(UObject* ObjectToCheck)
{
	if(const IVRGripInterface *GripInterface = Cast<IVRGripInterface>(ObjectToCheck))
	{
		TArray<FBPGripPair>OutHoldingController;
		bool OutIsHeld = false;
		GripInterface->Execute_IsHeld(ObjectToCheck,OutHoldingController,OutIsHeld);
		if(!GripInterface->Execute_AllowsMultipleGrips(ObjectToCheck) && OutIsHeld)
		{
			return OutHoldingController[0].HoldingController->GetOwner() == this;
		}
	}
	return true;
}

bool AIkarusVRBaseCharacter::CanSecondaryGripObject(UGripMotionControllerComponent* Hand,
	UGripMotionControllerComponent* OtherHand, UObject* ObjectToTryToGrab, FGameplayTag GripSecondaryTag, bool HadSlot,
	ESecondaryGripType SecGripType)
{
	if(SecGripType == ESecondaryGripType::SG_None)
	{
		return false;
	}

	if(!ValidateGameplayTag(GripType_Secondary,GripSecondaryTag,ObjectToTryToGrab,DefaultSecondaryGripTag) || !(ObjectToTryToGrab->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass())))
	{
		return false;
	}

	if(HadSlot)return true;

	if(SecGripType == ESecondaryGripType::SG_ScalingOnly || SecGripType == ESecondaryGripType::SG_Custom || SecGripType == ESecondaryGripType::SG_FreeWithScaling_Retain || SecGripType == ESecondaryGripType::SG_Free_Retain || SecGripType == ESecondaryGripType::SG_Free)
	{
		return true;
	}

	return false;
}

FTransform AIkarusVRBaseCharacter::GetBoneTransform(UObject* Object, FName BoneName)
{
	const FTransform Output;
	if(BoneName != FName("None"))
	{
		if(const USceneComponent *SceneComp = Cast<USceneComponent>(Object))
		{
			return SceneComp->GetSocketTransform(BoneName);
		}else
		{
			//Cast Unsuccessful
			//Cast to skeletal Mesh actor
			if(const ASkeletalMeshActor *SkeletalMeshActor = Cast<ASkeletalMeshActor>(Object))
			{
				return SkeletalMeshActor->GetSkeletalMeshComponent()->GetSocketTransform(BoneName);
			}
		}
	}
	return Output;
}

void AIkarusVRBaseCharacter::CallCorrectDropSingleEvent(UGripMotionControllerComponent* Hand,
	FBPActorGripInformation Grip)
{
	if(const IVRGripInterface *GripInterface = Cast<IVRGripInterface>(Grip.GrippedObject))
	{
		FVector OutCurrAngleVel;
		FVector OutCurrLinearVel;
		Hand->GetPhysicsVelocities(Grip,OutCurrAngleVel,OutCurrLinearVel);
		FVector OutThrowingAngleVel;
		FVector OutThrowingLinearVel;
		GetThrowingVelocity(Hand,Grip,OutCurrAngleVel,OutCurrLinearVel,OutThrowingAngleVel,OutThrowingLinearVel);
		
		if(IsALocalGrip(GripInterface->Execute_GripMovementReplicationType(Grip.GrippedObject)))
		{
			TryDropSingleClient(Hand,Grip,OutThrowingAngleVel,OutThrowingLinearVel);
		}else
		{
			TryDropSingle(Hand,OutThrowingAngleVel,OutThrowingLinearVel,Grip.GripID);
		}
	}
}

void AIkarusVRBaseCharacter::GetThrowingVelocity(UGripMotionControllerComponent* ThrowingController,
	FBPActorGripInformation Grip, FVector AngularVel, FVector ObjectsLinearVel, FVector& OutAngleVel,
	FVector& OutLinearVel)
{
	FVector LocalVel;
	if(Grip.GrippedObject->GetClass()->ImplementsInterface(UVRGripInterface::StaticClass()))
	{
		if(const IVRGripInterface *GripInt = Cast<IVRGripInterface>(Grip.GrippedObject))
		{
			TArray<FBPGripPair> OutHoldingController;
			bool bIsHeld = false;
			GripInt->Execute_IsHeld(Grip.GrippedObject,OutHoldingController,bIsHeld);
			if(OutHoldingController.Num()>1)
			{
				// if more than one hand is holding the object, then we will not set values.
				OutAngleVel = FVector(0,0,0);
				OutLinearVel = FVector(0,0,0);
				return;
			}
		}
	}
	if(SampleGripVelocity)
	{
		EControllerHand Hand = EControllerHand::Left;
		ThrowingController->GetHandType(Hand);
		if(Hand == EControllerHand::Left)
		{
			LocalVel = UVRExpansionFunctionLibrary::GetPeak_PeakLowPassFilter(LeftPeakVel);
		}else
		{
			LocalVel = UVRExpansionFunctionLibrary::GetPeak_PeakLowPassFilter(RightPeakVel);
		}
	}else
	{
		if(UseControllerVelocityOnRelease)
		{
			LocalVel = ThrowingController->GetComponentVelocity();
		}else
		{
			LocalVel = ObjectsLinearVel;	
		}
	}
	if(ScaleVelocityByMass)
	{
		float Mass = 0;
		ThrowingController->GetGripMass(Grip,Mass);
		LocalVel =	LocalVel * UKismetMathLibrary::Clamp(ThrowingMassMaximum/Mass,MassScalerMin,1.0);
	}else
	{
		if(!LimitMaxThrowVelocity)
		{
			OutAngleVel  = AngularVel;
			OutLinearVel = LocalVel;
			return;
		}
	}

	if((UKismetMathLibrary::Vector4_SizeSquared(LocalVel) > (MaximumThrowingVelocity*MaximumThrowingVelocity)))
	{
		UKismetMathLibrary::Vector_Normalize(LocalVel,0.0001);
		OutLinearVel = UKismetMathLibrary::Multiply_VectorFloat(LocalVel,MaximumThrowingVelocity);
	}else
	{
		OutLinearVel = LocalVel;
	}
	OutAngleVel = AngularVel;
}

void AIkarusVRBaseCharacter::TryGrabClient(UObject*ObjectToGrab, bool IsSlotGrip, FTransform_NetQuantize GripTransform,
	EControllerHand Hand, FGameplayTag GripSecondaryTag, FName GripBoneName, FName SlotName, bool IsSecondaryGrip)
{
	if(Hand == EControllerHand::Left)
	{
		TryToGrabObject(ObjectToGrab,GripTransform,LeftMotionController,RightMotionController,IsSlotGrip,GripSecondaryTag,GripBoneName,SlotName,IsSecondaryGrip);
	}else
	{
		TryToGrabObject(ObjectToGrab,GripTransform,RightMotionController,LeftMotionController,IsSlotGrip,GripSecondaryTag,GripBoneName,SlotName,IsSecondaryGrip);
	}
}

void AIkarusVRBaseCharacter::TryGrabServer(UObject* ObjectToGrab, bool IsSlotGrip, FTransform_NetQuantize GripTransform,
	EControllerHand Hand, FGameplayTag GripSecondaryTag, FName GripBoneName, FName SlotName, bool IsSecondaryGrip)
{
	if(Hand == EControllerHand::Left)
	{
		TryToGrabObject(ObjectToGrab,GripTransform,LeftMotionController,RightMotionController,IsSlotGrip,GripSecondaryTag,GripBoneName,SlotName,IsSecondaryGrip);
	}else
	{
		TryToGrabObject(ObjectToGrab,GripTransform,RightMotionController,LeftMotionController,IsSlotGrip,GripSecondaryTag,GripBoneName,SlotName,IsSecondaryGrip);
	}
}

void AIkarusVRBaseCharacter::CallCorrectGrabEvent(UObject* ObjectToGrip, EControllerHand Hand, bool IsSlotGrip,
	FTransform GripTransform, FGameplayTag GripSecondaryTag, FName OptionalBoneName, FName SlotName,
	bool IsSecondaryGrip)
{
	if(const IVRGripInterface * GripInterface = Cast<IVRGripInterface>(ObjectToGrip))
	{
		if(IsALocalGrip(GripInterface->Execute_GripMovementReplicationType(ObjectToGrip)))
		{
			TryGrabClient(ObjectToGrip,IsSlotGrip,GripTransform,Hand,GripSecondaryTag,OptionalBoneName,SlotName,IsSecondaryGrip);
		}else
		{
			TryGrabServer(ObjectToGrip,IsSlotGrip,GripTransform,Hand,GripSecondaryTag,OptionalBoneName,SlotName,IsSecondaryGrip);
		}
	}
}

void AIkarusVRBaseCharacter::DropItem(UGripMotionControllerComponent* Hand, FBPActorGripInformation GripInfo,
	FGameplayTagContainer GameplayTags)
{
	if(IsValid(GripInfo.GrippedObject))
	{
		if(ValidateGameplayTagContainer(DropType,GripInfo.GrippedObject,DefaultDropTag,GameplayTags))
		{
			CallCorrectDropSingleEvent(Hand,GripInfo);
		}
	}else
	{
		CallCorrectDropSingleEvent(Hand,GripInfo);
	}
}

void AIkarusVRBaseCharacter::DropItems(UGripMotionControllerComponent* Hand, FGameplayTagContainer GameplayTags)
{
	TArray<FBPActorGripInformation>GripArray;
	Hand->GetAllGrips(GripArray);
	for(int i = GripArray.Num()-1;i>=0;i--)
	{
		DropItem(Hand,GripArray[i],GameplayTags);
	}
}

FName AIkarusVRBaseCharacter::GetCorrectPrimarySlotPrefix(UObject* ObjectToCheckForTag, EControllerHand Hand,
	FName NearestBoneName)
{
	bool HasPerHandSocket = false;
	FName LocalBasePrefix;
	
	if(NearestBoneName != FName("None"))
	{
		LocalBasePrefix = NearestBoneName;
	}
	if(const IGameplayTagAssetInterface *TagAssetInterface = Cast<IGameplayTagAssetInterface>(ObjectToCheckForTag))
	{
		HasPerHandSocket = TagAssetInterface->HasMatchingGameplayTag(GripSockets_SeperateHandSocket);
	}

	if(HasPerHandSocket)
	{
		if(Hand == EControllerHand::Left)
		{
			return FName((LocalBasePrefix.ToString() + "VRGripLP"));
		}else if(Hand == EControllerHand::Right)
		{
			return FName((LocalBasePrefix.ToString() + "VRGripRP"));
		}	
	}
	if(NearestBoneName != FName("None"))
	{
		return FName((LocalBasePrefix.ToString() + "VRGripP"));
	}
	
	return FName("None");
}

bool AIkarusVRBaseCharacter::GripOrDropObjectClean(UGripMotionControllerComponent* CallingMotionController,
	UGripMotionControllerComponent* OtherController, bool CanCheckClimb, UPrimitiveComponent* GrabSphere,
	FGameplayTagContainer RelevantGameplayTags)
{
	
	UObject *NearestObject = nullptr;
	FName NearestBoneName = FName("None");
	FTransform ObjectTransform;
	ESecondaryGripType SecondaryGripType=ESecondaryGripType::SG_None;
	FVector ImpactPoint;

	if(CallingMotionController->HasGrippedObjects())
	{
		// if calling controller already has an gripped object, We will drop the object.
		DropItems(CallingMotionController,RelevantGameplayTags);
		return true;
	}
	bool OutDroppedSecondary = false;
	bool OutHadSecondary = false;
	DropSecondaryAttachment(CallingMotionController,OtherController,RelevantGameplayTags,OutDroppedSecondary,OutHadSecondary);

	if(OutHadSecondary)
	{
		return true;
	}
	bool OutImpInterface  = false;
	bool OutCanBeClimbed  = false;
	GetNearestOverlappingObject(GrabSphere,CallingMotionController,RelevantGameplayTags,NearestObject,OutImpInterface,ObjectTransform,OutCanBeClimbed,NearestBoneName,ImpactPoint);
	
	if(!IsValid(NearestObject))return false;
	if(OutImpInterface)
	{
		FName SlotName;
		bool bHadSlot = false;
		if(!CanAttemptGrabOnObject(NearestObject))
		{
			return false;
		}
		/*   -------Checking For Secondary Grab-------     */
		
		bool bCanAttemptSecondaryGrab = false;
		ESecondaryGripType OutSecGripType = ESecondaryGripType::SG_None ;
		CanAttemptSecondaryGrabOnObject(NearestObject,bCanAttemptSecondaryGrab,OutSecGripType);
		if(bCanAttemptSecondaryGrab)
		if(bCanAttemptSecondaryGrab)
		{
			//checks if secondary grip is valid or not
			SecondaryGripType = OutSecGripType;
			if(ValidateGameplayTagContainer(GripType_Secondary,NearestObject,DefaultSecondaryGripTag,RelevantGameplayTags))
			{
				if(IVRGripInterface *GripInt = Cast<IVRGripInterface>(NearestObject))
				{
					FTransform OutSlotWorldTransform;
					GripInt->Execute_ClosestGripSlotInRange(NearestObject,ImpactPoint,true,bHadSlot,OutSlotWorldTransform,SlotName,CallingMotionController,FName("None"));
					FGameplayTag OutFoundTag;
					UVRExpansionFunctionLibrary::GetFirstGameplayTagWithExactParent(GripType_Secondary,RelevantGameplayTags,OutFoundTag);
					if(CanSecondaryGripObject(CallingMotionController,OtherController,NearestObject,OutFoundTag,bHadSlot,SecondaryGripType))
					{
						FGameplayTag TagFound;
						UVRExpansionFunctionLibrary::GetFirstGameplayTagWithExactParent(GripType_Secondary,RelevantGameplayTags,TagFound);
						FTransform GripTrans;
						if(!bHadSlot)
						{
							GripTrans = UGripMotionControllerComponent::ConvertToGripRelativeTransform(ObjectTransform,CallingMotionController->GetPivotTransform());
						}else
						{
							GripTrans = UGripMotionControllerComponent::ConvertToGripRelativeTransform(ObjectTransform,OutSlotWorldTransform);
						}
						EControllerHand OutHand;
						CallingMotionController->GetHandType(OutHand);
						
						CallCorrectGrabEvent(NearestObject,OutHand,bHadSlot,GripTrans,TagFound,FName("None"),SlotName,true);
						return true;
					}
				}
			}
		}
		
		// if can not attempt secondary grab on object, then we will try for primary grab next.
		if(IVRGripInterface *GripInter = Cast<IVRGripInterface>(NearestObject))
		{
			if(!GripInter->Execute_DenyGripping(NearestObject,CallingMotionController))
			{
				if(!ValidateGameplayTagContainer(GripType,NearestObject,DefaultGripTag,RelevantGameplayTags))return false;
				FTransform OutSlotWorldTransform;
				FName OutSlotName;
				EControllerHand Hand;
				CallingMotionController->GetHandType(Hand);
				GripInter->Execute_ClosestGripSlotInRange(NearestObject,ImpactPoint,false,bHadSlot,OutSlotWorldTransform,OutSlotName,CallingMotionController,GetCorrectPrimarySlotPrefix(NearestObject,Hand,NearestBoneName));
				
				if(!bHadSlot)
				{
					if(IGameplayTagAssetInterface *GameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(NearestObject))
					{
						if(GameplayTagAssetInterface->HasMatchingGameplayTag(Weapon_DenyFreeGripping))
						{
							return false;
						}
					}
				}
				SlotName = OutSlotName;
				EControllerHand HandType;
				CallingMotionController->GetHandType(HandType);
				FTransform Res;
				FTransform Temp;
					if(NearestBoneName == FName("None"))
					{
						Temp = ObjectTransform;
					}else
					{
						Temp = GetBoneTransform(NearestObject,NearestBoneName);
					}
				if(bHadSlot)
				{
					//Remove additional controller scale if the controller is scaled from 1.0
					// Can use this function if you want objects to scale up to scaled controllers with
					// snap grips.
					// RemoveControllerScale();
					Res = UKismetMathLibrary::MakeRelativeTransform(Temp,OutSlotWorldTransform);
				}else
				{
					Res = CallingMotionController->ConvertToControllerRelativeTransform(Temp);
				}
				FGameplayTag TempSecondaryTag;
				CallCorrectGrabEvent(NearestObject,HandType,bHadSlot,Res,TempSecondaryTag,NearestBoneName,SlotName,false);
				return true;
			}
		}
	}
	//if dont implements Interface
	//then allow gripping non interface objects.
	if(!UBlueprintGameplayTagLibrary::HasTag(RelevantGameplayTags,DefaultGripTag,true))return false;
	FTransform Temp;
	if(NearestBoneName == FName("None"))
	{
		Temp = ObjectTransform;
	}else
	{
		Temp = GetBoneTransform(NearestObject,NearestBoneName);
	}
	
	EControllerHand HandType;
	CallingMotionController->GetHandType(HandType);
	FGameplayTag GripTag;
	TryGrabServer(NearestObject,false,CallingMotionController->ConvertToControllerRelativeTransform(Temp),HandType,GripTag,NearestBoneName,FName("None"),false);
	return true;
}

void AIkarusVRBaseCharacter::TriggerGripOrDrop(UGripMotionControllerComponent* CallingHand,
	UGripMotionControllerComponent* OtherHand, bool isGrip, UPrimitiveComponent* GrabSphere)
{
	if(isGrip)
	{
		FGameplayTagContainer RelevantGameplayTagContainer;
		RelevantGameplayTagContainer.AddTag(DropType_OnPrimaryGrip);
		RelevantGameplayTagContainer.AddTag(DropType_Secondary_OnPrimaryGrip);
		RelevantGameplayTagContainer.AddTag(GripType_OnPrimaryGrip);
		RelevantGameplayTagContainer.AddTag(GripType_Secondary_OnPrimaryGrip);
		GripOrDropObjectClean(CallingHand,OtherHand,true,GrabSphere,RelevantGameplayTagContainer);
	}else
	{
		if(ClimbingHand == CallingHand && bIsClimbing)
		{
			ExitClimbing();
		}
			FGameplayTagContainer RelevantGameplayTagCont;
			RelevantGameplayTagCont.AddTag(DropType_OnPrimaryGripRelease);
			RelevantGameplayTagCont.AddTag(DropType_Secondary_OnPrimaryGripRelease);
			GripOrDropObjectClean(CallingHand,OtherHand,false,GrabSphere,RelevantGameplayTagCont);
	}
}

void AIkarusVRBaseCharacter::RemoveSecondaryGrip(UGripMotionControllerComponent* Hand,
	UObject* GrippedActorToRemoveAttachment)
{
	Hand->RemoveSecondaryAttachmentPoint(GrippedActorToRemoveAttachment,0.25);
}

void AIkarusVRBaseCharacter::RumbleController(UGripMotionControllerComponent * Hand,UHapticFeedbackEffect_Base* HapticEff, float Intensity)
{
	if(IsValid(Hand))
	{
		EControllerHand currentHand;
		Hand->GetHandType(currentHand);
		(UGameplayStatics::GetPlayerController(GetWorld(),0))->PlayHapticEffect(HapticEff,currentHand,Intensity,false);
	}
}

void AIkarusVRBaseCharacter::OnRightGrabSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherComp->ComponentHasTag(ClimbingTag))
	{
		ClimbingWallGripRight = OtherComp;
	}
}

void AIkarusVRBaseCharacter::OnRightGrabSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ClimbingWallGripRight = nullptr;
}

void AIkarusVRBaseCharacter::OnLeftGrabSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherComp->ComponentHasTag(ClimbingTag))
	{
		ClimbingWallGripLeft = OtherComp;
	}
}

void AIkarusVRBaseCharacter::OnLeftGrabSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ClimbingWallGripLeft = nullptr;
}

void AIkarusVRBaseCharacter::NotifyServerOfTossRequest(bool LeftHand, UPrimitiveComponent* TargetPrim)
{
	if(LeftHand)
	{
		TeleportControllerLeft->ServerSideToss(TargetPrim);
	}else
	{
		TeleportControllerRight->ServerSideToss(TargetPrim);
	}
}

void AIkarusVRBaseCharacter::UpdateClimbingMovement_Binding()
{
	if(bIsClimbing)
	{
		if(ClimbingHand && ClimbingGrip)
		{
			const FVector MovementVector = UKismetMathLibrary::NegateVector(
				ClimbingHand->GetComponentLocation() -
				UKismetMathLibrary::TransformLocation(ClimbingGrip->GetComponentTransform(), ClimbGripLocation));
			
			VRMovementReference->AddCustomReplicatedMovement(MovementVector);
		}
	}
}

void AIkarusVRBaseCharacter::SpawnController()
	{
	
		if(TeleportControllerClass)
		{
			const FVector SpawnLocation = FVector(0.0f, 0.0f, 0.0f);
			const FRotator SpawnRotation = FRotator(0.0f, 0.0f, 0.0f);
				FActorSpawnParameters SpawnParams;

				//For Left Motion Controller.
				// TeleportControllerLeft = GetWorld()->SpawnActorDeferred<ATeleportController>(TeleportControllerClass,FTransform(),this,NULL);
				TeleportControllerLeft = GetWorld()->SpawnActor<ATeleportController>(TeleportControllerClass,SpawnLocation,SpawnRotation);

			if(TeleportControllerLeft)
			{
				TeleportControllerLeft->OwningMotionController = LeftMotionController;
				if(IsLocallyControlled())
				{
					TeleportControllerLeft->bIsLocal = true;
				}
			
			TeleportControllerLeft->AttachToComponent(GetCapsuleComponent(),
				FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					   EAttachmentRule::SnapToTarget,false));
			}

			//For Right Motion Controller.
			TeleportControllerRight =  GetWorld()->SpawnActor<ATeleportController>(TeleportControllerClass,SpawnLocation,SpawnRotation);

			if(TeleportControllerRight)
			{
				
			TeleportControllerRight->AttachToComponent(GetCapsuleComponent(),
				FAttachmentTransformRules(
					EAttachmentRule::SnapToTarget,
					EAttachmentRule::SnapToTarget,
					   EAttachmentRule::SnapToTarget,false));
			TeleportControllerRight->OwningMotionController = RightMotionController;
				
				if(IsLocallyControlled())
				{
					TeleportControllerRight->bIsLocal = true;
				}
			}
		}
		if(TeleportControllerClass)
		{
			InitTeleportControllers();
			if(TeleportControllerLeft)
				TeleportControllerLeft->OnTeleportControllerClassConstructed();
			if(TeleportControllerRight)
				TeleportControllerRight->OnTeleportControllerClassConstructed();
		}
	}

void AIkarusVRBaseCharacter::SetGripComponents(UPrimitiveComponent * LeftHand,UPrimitiveComponent* RightHand)
{
	if(IsValid(LeftHand))
	{
		LeftHandGripComponent = LeftHand;
		LeftMotionController->SetCustomPivotComponent(LeftHandGripComponent);
	}else
	{
		Print("INVALID GRIP COMPONENT SET LEFT");
	}
	if(IsValid(RightHand))
	{
		RightHandGripComponent = RightHand;
		RightMotionController->SetCustomPivotComponent(RightHandGripComponent);
	}else
	{
		Print("INVALID GRIP COMPONENT SET RIGHT");
	}
}

void AIkarusVRBaseCharacter::InitTeleportControllers()
{
	// Call Init Controller function from TeleportController class.
	TeleportControllerRight->InitController();
	TeleportControllerLeft->InitController();
}

void AIkarusVRBaseCharacter::UpdateTeleportationRotations(ATeleportController* TeleportController, FVector2D Input)
{
	//For Right Controller :
	const float XVal = Input.X;
	const float YVal = Input.Y;

	if(IsValid(TeleportController))
	{
		if(TeleportController->bIsTeleporterActive)
		{
			if(bTeleportUsesThumbRotation)
			{
				FRotator OutRotation;
				float OutMagnitude;
				bool OutWasValid;
				
				CalcPadRotationAndMagnitude(YVal,XVal,1.0,TeleportThumbDeadZone,OutRotation,OutMagnitude,OutWasValid);
				if(OutWasValid)
				{
					TeleportController->TeleportRotation = OutRotation;
				}
			}else
			{
				TeleportController->TeleportRotation = FRotator(0,0,0);
			}
			TeleportController->TeleportBaseRotation = GetVRRotation();
		}
	}
}

void AIkarusVRBaseCharacter::SetTeleportActive(EControllerHand Hand, bool Active)
{
	if (Hand == EControllerHand::Left)
	{
		if (TeleportControllerLeft && Active)
		{
			TeleportControllerLeft->ActivateTeleporter();
		}
		else
		{
			TeleportControllerLeft->DeactivateTeleporter();
		}
	}
	else if (Hand == EControllerHand::Right)
	{
		if (TeleportControllerRight && Active)
		{
			TeleportControllerRight->ActivateTeleporter();
		}
		else
		{
			TeleportControllerRight->DeactivateTeleporter();
		}
	}
	NotifyTeleportActive(Hand, Active);
}

void AIkarusVRBaseCharacter::ExecuteTeleportation(ATeleportController * MotionController, EControllerHand Hand)
{
	if (!IsTeleporting)
	{
		VRMovementReference->StopMovementImmediately();

		if (MotionController->IsValidTeleportDestination)
		{
			IsTeleporting = true;
			APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
			CameraManager->StartCameraFade(0.f, 1.f, FadeOutDuration, TeleportFadeColor, false, true);


			FVector TeleportLocation;
			FRotator TeleportRotation;
			MotionController->GetTeleportDestination(false, TeleportLocation, TeleportRotation);

			FVector OutLocation;
			FRotator OutRotation;
			GetCharacterRotatedPosition(GetTeleportLocation(TeleportLocation), TeleportRotation, GetVRLocation(), OutLocation, OutRotation);

			FTimerHandle FadeTimer;
			
			TeleportHandForFade = Hand;
			TeleportLocationForFade = OutLocation;
			TeleportRotationForFade = OutRotation;
			
			GetWorld()->GetTimerManager().SetTimer(FadeTimer, this, &AIkarusVRBaseCharacter::DelayFunctionRunAfterFadeTime, FadeOutDuration, false);
		}
		else
		{
			SetTeleportActive(Hand, false);
		}
	}
}

void AIkarusVRBaseCharacter::GetCharacterRotatedPosition(const FVector& OriginalLocation, const FRotator& DeltaRotation, FVector PivotPoint, FVector & OutLocation, FRotator & OutRotation)
{
	PivotPoint = UKismetMathLibrary::InverseTransformLocation(GetActorTransform(), PivotPoint);
	UVRExpansionFunctionLibrary::RotateAroundPivot(DeltaRotation, OriginalLocation, GetCorrectRotation(), PivotPoint, OutLocation, OutRotation, true);
}

FRotator AIkarusVRBaseCharacter::GetCorrectRotation()
{
	return bUseControllerRotationYaw == true ? GetControlRotation() : GetActorRotation();
}

void AIkarusVRBaseCharacter::NotifyTeleportActive(EControllerHand  Hand,bool State)
{
	
	MulticastTeleportActive(Hand,State);
}

void AIkarusVRBaseCharacter::MulticastTeleportActive(EControllerHand  Hand,bool State)
{
	if(!IsLocallyControlled())
	{
		if(Hand == EControllerHand::Left)
		{
			if(State)TeleportControllerLeft->ActivateTeleporter();
			else TeleportControllerLeft->DeactivateTeleporter();
		}
		else if(Hand == EControllerHand::Right)
		{
			if(State)TeleportControllerRight->ActivateTeleporter();
			else TeleportControllerRight->DeactivateTeleporter();
		}
	}
}

void AIkarusVRBaseCharacter::HandleTurnInput(float InputAxis)
{
	if (!bIsClimbing || (bIsClimbing && bEnableSnapTurnWhileClimbing))
	{
		if (InputAxis != 0)
		{

			InputValue = InputAxis;
			if (bTurnModeIsSnap)
			{
				// Snap Turn
				if (bTurningFlag)
				{
					if ((abs(InputValue) < TurningActivationThreshold))
					{
						bTurningFlag = false;
					}
				}
				else
				{
					if ((abs(InputValue) > TurningActivationThreshold))
					{
						bTurningFlag = true;
						VRMovementReference->PerformMoveAction_SnapTurn(UKismetMathLibrary::SignOfFloat(InputValue) * SnapTurnAngle, EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn, true, true, true);
					}
				}
			}
			else
			{
				//Smooth Turn
				if ((abs(InputValue) > TurningActivationThreshold))
				{
					VRMovementReference->PerformMoveAction_SnapTurn((UKismetMathLibrary::SignOfFloat(InputValue) * SmoothTurnSpeed) * (UGameplayStatics::GetWorldDeltaSeconds(GetWorld())), EVRMoveActionVelocityRetention::VRMOVEACTION_Velocity_Turn, false, false, true);
				}
			}
		}
	}
}

void AIkarusVRBaseCharacter::HandleCurrentMovementInput(FVector2D MovementInput)
{

	if (TeleportControllerLeft && !TeleportControllerLeft->bIsTeleporterActive && !TeleportControllerRight->bIsTeleporterActive) {
		UGripMotionControllerComponent* CallingHand;
		if (bIsRightHand)
		{
			CallingHand = RightMotionController;
		}
		else
		{
			CallingHand = LeftMotionController;
		}
		const FVector2D NormalizedInput = MovementInput.GetSafeNormal();
		ThumbY = NormalizedInput.Y;
		ThumbX = NormalizedInput.X;

		FRotator Rotation;
		float Magnitude = 0.0f;
		bool WasValid = false;

		CalcPadRotationAndMagnitude(ThumbY, ThumbX, DPadVelocityScaler, SlidingMovementDeadZone, Rotation, Magnitude, WasValid);

		FVector FwrdVect;
		FVector OutRightVec;

		GetDPadMovementFacing(CallingHand, FwrdVect, OutRightVec);

		if (WasValid)
		{
			if (bCharacterMovementAccordingToController)
			{
				MapThumbToWorld(CallingHand, Rotation, Direction);
			}
			else
			{
				Direction = Rotation.RotateVector(FwrdVect);
			}
		}
		else
		{
			Direction = FwrdVect;
		}
		AddMovementInput(Direction, Magnitude, false);
	}
}

void AIkarusVRBaseCharacter::CalcPadRotationAndMagnitude(float YAxis, float XAxis, float OptMagnitudeScaler,
	float OptionalDeadzone, FRotator& OutRotation, float& OutMagnitude, bool& OutWasValid)
{
	//Output Rotation
	OutRotation = UKismetMathLibrary::MakeRotFromX(FVector(YAxis, XAxis, 0));

	//Output Magnitude
	const double MaxEle = UKismetMathLibrary::GetMaxElement(FVector(abs(YAxis * OptMagnitudeScaler), abs(XAxis * OptMagnitudeScaler), 0.0));
	// OutMagnitude = UKismetMathLibrary::Clamp(MaxEle,0.00000f,1.0000f);
	OutMagnitude = MaxEle;

	//Output WasValid
	OutWasValid = (abs(YAxis) + abs(XAxis)) > OptionalDeadzone;

}

void AIkarusVRBaseCharacter::GetDPadMovementFacing(const UGripMotionControllerComponent* CallingHand, FVector& OutForwardVector, FVector& OutRightVector)
{
	if (bCharacterMovementAccordingToController)
	{
		OutForwardVector = UKismetMathLibrary::Normal(UKismetMathLibrary::ProjectVectorOnToPlane(GetCorrectAimComp(CallingHand)->GetForwardVector(), GetVRUpVector()));
		OutRightVector = UKismetMathLibrary::Normal(UKismetMathLibrary::ProjectVectorOnToPlane(GetCorrectAimComp(CallingHand)->GetRightVector(), GetVRUpVector()));
	}
	else
	{
		OutForwardVector = GetVRForwardVector();
		OutRightVector = GetVRRightVector();
	}
}

void AIkarusVRBaseCharacter::MapThumbToWorld(const UGripMotionControllerComponent* CallingHand, const FRotator& Rotation, FVector& OutDirection)
{
	// OutDirection = UKismetMathLibrary::Normal(UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::ComposeRotators(Rotation,CallingHand->GetComponentRotation())),GetVRUpVector()));

	const USceneComponent* AimComp = GetCorrectAimComp(CallingHand);
	OutDirection = UKismetMathLibrary::Normal(UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::ComposeRotators(Rotation, AimComp->GetComponentRotation())), GetVRUpVector()));
}

USceneComponent* AIkarusVRBaseCharacter::GetCorrectAimComp(const UGripMotionControllerComponent* CallingHand)
{
	if (CallingHand == LeftMotionController)return AimLeft;
	return AimRight;
}

void AIkarusVRBaseCharacter::DelayFunctionRunAfterFadeTime()
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	
	SetTeleportActive(TeleportHandForFade, false);
	VRMovementReference->PerformMoveAction_Teleport(TeleportLocationForFade, TeleportRotationForFade);

	CameraManager->StartCameraFade(1.f, 0.f, FadeOutDuration, TeleportFadeColor, false, false);
	IsTeleporting = false;
}

void AIkarusVRBaseCharacter::InitializeClimbing(UPrimitiveComponent * NewClimbingGrip,UGripMotionControllerComponent*CallingHand)
{
	ClimbGripLocation =  UKismetMathLibrary::InverseTransformLocation( NewClimbingGrip->GetComponentTransform(),CallingHand->GetComponentLocation());
	ClimbingGrip = NewClimbingGrip;
	bIsClimbing = true;
	ClimbingHand = CallingHand;
	VRMovementReference->SetClimbingMode(true);
}

void AIkarusVRBaseCharacter::ExitClimbing()
{
	if(bIsClimbing)
	{
		VRMovementReference->SetClimbingMode(false);
		bIsClimbing = false;
		ClimbingHand = nullptr;
		ClimbGripLocation = FVector(0,0,0);
		ClimbingGrip = nullptr;
	}
}

FString AIkarusVRBaseCharacter::CheckXRApi()
{
	if (GEngine)
	{
		const auto XRSystem = GEngine->XRSystem;
		if (XRSystem.IsValid() && XRSystem->GetSystemName().ToString().Contains("OpenXR"))
		{
			return TEXT("OpenXR");
		}
		else if (XRSystem.IsValid() && XRSystem->GetSystemName().ToString().Contains("Oculus"))
		{
			return TEXT("Meta XR");
		}
	}
	return TEXT("Unknown");
}

void AIkarusVRBaseCharacter::SpawnSpectator()
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
		
	if(AVRSpectator* Spectator = GetWorld()->SpawnActor<AVRSpectator>(SpawnParameters))
	{
		Spectator->SceneCapture->TextureTarget = RenderTarget;
		Spectator->FieldOfView = SpectatorFOV;
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::Texture);
		UTexture* Texture = Cast<UTexture>(Spectator->SceneCapture->TextureTarget);
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenTexture(Texture);
	}
}

bool AIkarusVRBaseCharacter::IfOverWidgetUse(UGripMotionControllerComponent * CallingHand,bool Pressed)
{
	EControllerHand Hand;
	CallingHand->GetHandType(Hand);
	if(Hand == EControllerHand::Left)
	{
		if(IsValid(TeleportControllerLeft))
		{
			return TeleportControllerLeft->IfOverWidgetUse(Pressed);
		}
	}
	if(Hand == EControllerHand::Right)
	{
		if(IsValid(TeleportControllerRight))
		{
			return TeleportControllerRight->IfOverWidgetUse(Pressed);
		}
	}
	return false;
}


// Print Functions...
void AIkarusVRBaseCharacter::Print(const FString& Message,int key,FColor Color,float TimeToDisplay)
	{
		if(GEngine)
		GEngine->AddOnScreenDebugMessage(key, TimeToDisplay, Color, Message);
	}

void AIkarusVRBaseCharacter::Print(int Message, int key, FColor Color,float TimeToDisplay)
{
	const FString NumAsString = FString::FromInt(Message);
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(key, TimeToDisplay, Color, NumAsString);
}

void AIkarusVRBaseCharacter::Print(const FTransform& Message, int key, FColor Color, float TimeToDisplay,bool PrintOnlyLocation )
{
	const FString Loc = Message.GetLocation().ToString();
	const FString Rot = Message.GetRotation().ToString();
	
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(key, TimeToDisplay, Color, Loc);
		if(!PrintOnlyLocation)
		{
			GEngine->AddOnScreenDebugMessage(key+1, TimeToDisplay, Color, Rot);
		}
	}
}

/************************************************************************/  