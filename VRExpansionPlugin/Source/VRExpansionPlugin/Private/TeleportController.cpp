
#include "TeleportController.h"
#include "GripMotionControllerComponent.h"
#include "VRExpansionFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IkarusVRBaseCharacter.h"
#include "PhysicsTossManager.h"


ATeleportController::ATeleportController()
{
	PrimaryActorTick.bCanEverTick = true;

	LaserBeamTraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	Scene->SetMobility(EComponentMobility::Movable);
	SetRootComponent(Scene);

	ArcEndPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArcEndPoint"));
	ArcEndPoint->SetupAttachment(Scene);
	ArcEndPoint->SetMobility(EComponentMobility::Movable);
	ArcEndPoint->SetGenerateOverlapEvents(false);
	ArcEndPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArcEndPoint->SetVisibility(false);

	ArcSpline = CreateDefaultSubobject<USplineComponent>(TEXT("ArcSpline"));
	ArcSpline->SetupAttachment(Scene);
	ArcSpline->SetMobility(EComponentMobility::Movable);

	LaserSpline = CreateDefaultSubobject<USplineComponent>(TEXT("LaserSpline"));
	LaserSpline->SetupAttachment(Scene);
	LaserSpline->SetMobility(EComponentMobility::Movable);

	LaserBeamEndPoint = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserBeamEndPoint"));
	LaserBeamEndPoint->SetupAttachment(Scene);
	LaserBeamEndPoint->SetGenerateOverlapEvents(false);
	LaserBeamEndPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LaserBeamEndPoint->SetVisibility(false);
	LaserBeamEndPoint->SetMobility(EComponentMobility::Movable);

	TeleportCylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TeleportCylinder"));
	TeleportCylinder->SetupAttachment(Scene);
	TeleportCylinder->SetGenerateOverlapEvents(false);
	TeleportCylinder->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TeleportCylinder->SetVisibility(false);
	TeleportCylinder->SetMobility(EComponentMobility::Movable);

	Ring = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ring"));
	Ring->SetupAttachment(TeleportCylinder);
	Ring->SetGenerateOverlapEvents(false);
	Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ring->SetMobility(EComponentMobility::Movable);

	Arrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(TeleportCylinder);
	Arrow->SetGenerateOverlapEvents(false);
	Arrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Arrow->SetMobility(EComponentMobility::Movable);

	FinalFacingArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FinalFacingArrow"));
	FinalFacingArrow->SetupAttachment(Arrow);
	FinalFacingArrow->SetGenerateOverlapEvents(false);
	FinalFacingArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FinalFacingArrow->SetHiddenInGame(true);
	FinalFacingArrow->SetMobility(EComponentMobility::Movable);

	LaserBeam = CreateDefaultSubobject<USplineMeshComponent>(TEXT("LaserBeam"));
	LaserBeam->SetupAttachment(Scene);
	LaserBeam->SetGenerateOverlapEvents(false);
	LaserBeam->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LaserBeam->SetHiddenInGame(true);
	LaserBeam->SetStartTangent(FVector(1.f, 0.f, 0.f));
	LaserBeam->SetEndPosition(FVector(1.f, 0.f, 0.f));
	LaserBeam->SetEndTangent(FVector(1.f, 0.f, 0.f));
	LaserBeam->SetMobility(EComponentMobility::Movable);

	Wic = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WIC"));
	Wic->SetupAttachment(LaserBeam);
	Wic->InteractionSource = EWidgetInteractionSource::Custom;
	Wic->SetAutoActivate(false);
	Wic->SetMobility(EComponentMobility::Movable);

	// PhysicsTossManager = CreateDefaultSubobject<UPhysicsTossManager>(TEXT("PhysicsTossManager"));
	
	LowPassFilter.MinCutoff = 1.2f;
	LowPassFilter.DeltaCutoff = 0.001f;
	LowPassFilter.CutoffSlope = 0.001f;
}

void ATeleportController::BeginPlay()
{
	Super::BeginPlay();

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void ATeleportController::OnTeleportControllerClassConstructed()
{
	/*  */
	
	TeleportCylinder->SetVisibility(false,true);

	if(IsValid(OwningMotionController))
	{
		FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		
		LaserSpline->AttachToComponent(OwningMotionController,AttachmentRules);

		LaserBeamEndPoint->AttachToComponent(OwningMotionController,AttachmentRules);
		
		AttachmentRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepRelative, EAttachmentRule::SnapToTarget, true);
		LaserBeam->AttachToComponent(OwningMotionController,AttachmentRules);

		LaserBeamEndPoint->SetRelativeScale3D(FVector(0.02,0.02,0.02));
	
		if(OwningMotionController->IsLocallyControlled())
		{
			APlayerController * PlayerController = UGameplayStatics::GetPlayerController(GetWorld(),0);
			EnableInput(PlayerController);

			AIkarusVRBaseCharacter * IkarusBaseChar = Cast<AIkarusVRBaseCharacter>(OwningMotionController->GetOwner());
			if(IsValid(IkarusBaseChar))
			{
				// IkarusBaseChar->OnCharacterTeleported_Bind;

				SetOwner(OwningMotionController->GetOwner());
			}
		}
	}
}

void ATeleportController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	ClearArc();
	ClearLaserBeam();
	DisableWidgetActivation();
	
}

void ATeleportController::GetTeleWorldLocAndForwardVector(FVector& WorldLoc, FVector& ForwardVector, bool bUseRotOffset)
{
	WorldLoc = OwningMotionController->GetComponentLocation();
	ForwardVector = bUseRotOffset ? 
		UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::ComposeRotators(ControllerRotationOffset,
		OwningMotionController->GetComponentRotation())) : 
		UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::ComposeRotators(UKismetMathLibrary::NegateRotator(ControllerRotationOffset),
		OwningMotionController->GetComponentRotation()));
}

void ATeleportController::ActivateTeleporter()
{
	bIsTeleporterActive = true;
	TeleportCylinder->SetVisibility(true, true);
	
	if(IsValid(OwningMotionController))
	{
		InitialControllerRotation = OwningMotionController->GetComponentRotation();
		ToggleTick();
	}
}

void ATeleportController::DeactivateTeleporter()
{
	if(bIsTeleporterActive)
	{
		bIsTeleporterActive = false;
		TeleportCylinder->SetVisibility(false, true);
		ArcEndPoint->SetVisibility(false, false);
		ToggleTick();
		ClearArc();
	}
}

void ATeleportController::TraceTeleportDestination(bool& bSuccess, TArray<FVector>& TracePoints,
	FVector& NavMeshLocation, FVector& TraceLocation)
{
	FVector TeleWorldLoc;
	FVector TeleForwardVec;
	GetTeleWorldLocAndForwardVector(TeleWorldLoc, TeleForwardVec);
	TeleWorldLoc = TeleWorldLoc + (TeleForwardVec * LaserBeamTraceOffset);
	
	FHitResult TraceResult;
	FVector LastOutTraceDest;
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjects;
	TraceObjects.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
	TArray<AActor*> ActorsToIgnore;
	
	bool WasHit = UGameplayStatics::Blueprint_PredictProjectilePath_ByObjectType(GetWorld(), TraceResult, TracePoints,
		LastOutTraceDest, TeleWorldLoc, TeleForwardVec*TeleportLaunchSpeed, true, 0.f, TraceObjects, false,
		ActorsToIgnore, EDrawDebugTrace::None, 0,30.0,2.0,0.0);
	
	TraceLocation = TraceResult.Location;
	if(UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation ProjectedNavLocation;
		
		NavSystem->ProjectPointToNavigation(TraceResult.Location, ProjectedNavLocation, FVector(ProjectNavExtends));
		NavMeshLocation = ProjectedNavLocation.Location;
		bSuccess = UKismetMathLibrary::NotEqual_VectorVector(TraceResult.Location, NavMeshLocation) &&
			UKismetMathLibrary::NotEqual_VectorVector(NavMeshLocation, FVector::ZeroVector) && WasHit;
	}
	
}

void ATeleportController::ClearArc()
{
	for(const auto Mesh : SplineMeshes)
	{
		Mesh->DestroyComponent();
	}
	SplineMeshes.Empty();
	ArcSpline->ClearSplinePoints(true);
}

void ATeleportController::UpdateArcSpline(bool bFoundValidLoc,  TArray<FVector> SplinePoints)
{
	ArcSpline->ClearSplinePoints(true);
	if(!bFoundValidLoc)
	{
		SplinePoints.Empty();

		FVector TeleWorldLoc;
		FVector TeleForwardVec;

		GetTeleWorldLocAndForwardVector(TeleWorldLoc, TeleForwardVec);
		SplinePoints.Add(TeleWorldLoc);
		SplinePoints.Add((TeleWorldLoc + (TeleForwardVec * 20.f)));
	}
	
	for (FVector Loc : SplinePoints)
	{
		ArcSpline->AddSplinePoint(Loc, ESplineCoordinateSpace::Local);
	}

	if (SplinePoints.Num() > 0)
	{
		ArcSpline->SetSplinePointType(SplinePoints.Num() - 1, ESplinePointType::CurveClamped);
	}

	PointDiffNum = (ArcSpline->GetNumberOfSplinePoints() - 1) - SplineMeshes.Num();

	if(SplineMeshes.Num() > 0)
	{
		SplineMeshes[0]->SetMaterial(0,TeleportSplineStartingMaterial);
	}
	
	if (SplineMeshes.Num() < ArcSpline->GetNumberOfSplinePoints())
	{
		
		for (int32 i = 0; i <= PointDiffNum; i++)
		{
			MySplineMeshComponent = NewObject<USplineMeshComponent>(this);
			MySplineMeshComponent->SetStaticMesh(BaseSplineMesh);
			MySplineMeshComponent->RegisterComponent();
			MySplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			MySplineMeshComponent->SetGenerateOverlapEvents(false);
			SplineMeshes.Add(MySplineMeshComponent);
		}

		int32 ArrayIndex = 0;
		for (auto Mesh : SplineMeshes)
		{

			if (ArrayIndex < ArcSpline->GetNumberOfSplinePoints() - 1)
			{
				SplineMeshes[ArrayIndex]->SetVisibility(true);

				SplineMeshes[ArrayIndex]->SetStartAndEnd(SplinePoints[ArrayIndex],
					ArcSpline->GetTangentAtSplinePoint(ArrayIndex, ESplineCoordinateSpace::Local),
					SplinePoints[ArrayIndex + 1],
					ArcSpline->GetTangentAtSplinePoint(ArrayIndex + 1, ESplineCoordinateSpace::Local));
			}
			else
			{
				SplineMeshes[ArrayIndex]->SetVisibility(false);
			}
			ArrayIndex++;
		}
	}
	else
	{
		int32 ArrayIndex = 0;
		for (auto Mesh : SplineMeshes)
		{
			if (ArrayIndex < ArcSpline->GetNumberOfSplinePoints() - 1)
			{
				SplineMeshes[ArrayIndex]->SetVisibility(true);

				SplineMeshes[ArrayIndex]->SetStartAndEnd(SplinePoints[ArrayIndex],
					ArcSpline->GetTangentAtSplinePoint(ArrayIndex, ESplineCoordinateSpace::Local),
					SplinePoints[ArrayIndex + 1],
					ArcSpline->GetTangentAtSplinePoint(ArrayIndex + 1, ESplineCoordinateSpace::Local));
			}
			else
			{
				SplineMeshes[ArrayIndex]->SetVisibility(false);
			}
			ArrayIndex++;
		}
	}
}

void ATeleportController::UpdateArcEndpoint(const FVector& NewLoc, bool bValidLocFound)
{
	ArcEndPoint->SetVisibility(bIsTeleporterActive && bValidLocFound);
	ArcEndPoint->SetWorldLocation(NewLoc,false, nullptr, ETeleportType::TeleportPhysics);
	Arrow->SetWorldRotation(UKismetMathLibrary::ComposeRotators(TeleportRotation,TeleportBaseRotation));
}

void ATeleportController::GetTeleportDestination(bool RelativeToHMD, FVector& Location, FRotator& Rotation)
{
	if(RelativeToHMD)
	{
		FVector HMDPos;
		FRotator HMDRot;
		UHeadMountedDisplayFunctionLibrary::GetOrientationAndPosition(HMDRot, HMDPos);
		HMDPos = HMDPos * FVector(1.f, 1.f, 0.f);

		Location = LastValidTeleportLocation - UKismetMathLibrary::Quat_RotateVector(FQuat(TeleportRotation), HMDPos);
		Rotation = TeleportRotation;
		
	}
	else
	{
		Location = LastValidTeleportLocation;
		Rotation = TeleportRotation;
	}
}

void ATeleportController::RumbleController(float Intensity)
{
	if(IsValid(OwningMotionController))
	{
		EControllerHand Hand;
		OwningMotionController->GetHandType(Hand);
		(UGameplayStatics::GetPlayerController(GetWorld(),0))->PlayHapticEffect(HapticEffect,Hand,Intensity,false);
	}
}

void ATeleportController::OnLaserBeamActiveTriggerAxis()
{
	// if((!PhysicsTossManager->IsThrowing()) && (bIsLaserBeamActive) && ( (UKismetMathLibrary::BooleanNOR(Wic->IsOverInteractableWidget(),Wic->IsOverFocusableWidget())) ))
	// {
	// 	if(IsValid(LaserHighlightingObject))
	// 	{
	// 		AIkarusVRBaseCharacter * IkarusCharacter = Cast<AIkarusVRBaseCharacter>(OwningMotionController->GetOwner());
	// 		if(IsValid(IkarusCharacter))
	// 		{
	// 			IkarusCharacter->NotifyServerOfTossRequest(false,LaserHighlightingObject);
	// 		}
	// 	}
	// }
}

void ATeleportController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateLaserBeam(DeltaTime);

	bool bSuccess = false;
	TArray<FVector>TracePoints;
	FVector NavMeshLocation;
	FVector TraceLocation;
	
	if(bIsTeleporterActive)
	{
		TraceTeleportDestination(bSuccess,TracePoints,NavMeshLocation,TraceLocation);
		IsValidTeleportDestination = bSuccess;

		// sequence 
		TeleportCylinder->SetVisibility(IsValidTeleportDestination,true);
		if(IsValidTeleportDestination)
		{
			const TArray<TEnumAsByte<EObjectTypeQuery>>ObjectTypes;
			TArray<AActor*> ActorsToIgnore;
			FHitResult OutHit;
			UKismetSystemLibrary::LineTraceSingleForObjects(GetWorld(),NavMeshLocation,UKismetMathLibrary::Add_VectorVector(NavMeshLocation , TraceEndLocation),ObjectTypes,false,ActorsToIgnore,EDrawDebugTrace::None,OutHit,true);

			TeleportCylinder->SetWorldLocation(UKismetMathLibrary::SelectVector(OutHit.ImpactPoint,NavMeshLocation,OutHit.bBlockingHit),false,nullptr,ETeleportType::TeleportPhysics);
			LastValidTeleportLocation = TeleportCylinder->GetComponentLocation();
		}

		//Sequence 1
		//Rumble Controller when valid teleport controller found.
		if((IsValidTeleportDestination && !bLastFrameValidDestination) || (!IsValidTeleportDestination && bLastFrameValidDestination))
		{
			RumbleController(RumbleControllerIntensity);
		}

		//Sequence 2
		bLastFrameValidDestination = bSuccess;

		//Sequence 3
		UpdateArcSpline(bSuccess,TracePoints);

		//Sequence 4
		UpdateArcEndpoint(TraceLocation,bSuccess);
	}
	
}

void ATeleportController::ServerSideToss(UPrimitiveComponent* TargetObject)
{
	// PhysicsTossManager->ServersideToss(TargetObject,OwningMotionController);
}


void ATeleportController::SetLaserBeamActive(bool bLaserBeamActive)
{
	if(bLaserBeamActive != bIsLaserBeamActive)
	{
		bIsLaserBeamActive = bLaserBeamActive;

		if(bIsLaserBeamActive)
		{
			LaserBeam->SetHiddenInGame(bUseSmoothLaser);
			LaserBeamEndPoint->SetHiddenInGame(bUseSmoothLaser);
			
			UVRExpansionFunctionLibrary::ResetEuroSmoothingFilter(LowPassFilter);
			CreateLaserSpline();
			if(bIsLocal)
			{
				Wic->Activate(false);
			}
			ToggleTick();
		}
		else
		{
			LaserBeam->SetHiddenInGame(true);
			LaserBeamEndPoint->SetHiddenInGame(true);
			
			ClearLaserBeam();
			const FHitResult HitResult;
			Wic->SetCustomHitResult(HitResult);
			if(bIsLocal)
			{
				DisableWidgetActivation();
			}
			ToggleTick();
		}
	}
}

void ATeleportController::UpdateLaserBeam(const float& Deltatime)
{
	if(bIsLaserBeamActive)
	{
		LaserHighlightingObject = nullptr;
		Wic->InteractionDistance = LaserBeamMaxDistance;

		FVector TeleWorldLoc;
		FVector TeleForwardVec;
		GetTeleWorldLocAndForwardVector(TeleWorldLoc, TeleForwardVec);
		TeleWorldLoc = TeleWorldLoc + (TeleForwardVec * LaserBeamTraceOffset);

		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(OwningMotionController->GetOwner());
			
		const bool bWasHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), TeleWorldLoc,
			(TeleForwardVec * LaserBeamMaxDistance) + TeleWorldLoc,
			LaserBeamTraceChannel, false, ActorsToIgnore, EnableDebugMode ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None,
			LastLaserHitResult, true);

		// Smooth Laser Logic
		if(bUseSmoothLaser)
		{
			const FVector InRawValue = TeleWorldLoc + (TeleForwardVec * LastLaserHitResult.Time * LaserBeamMaxDistance);
			FVector SmoothedLoc;
			UVRExpansionFunctionLibrary::RunEuroSmoothingFilter(LowPassFilter, InRawValue, Deltatime, SmoothedLoc);

			UVRExpansionFunctionLibrary::SmoothUpdateLaserSpline(LaserSpline, LaserSplineMeshes,
				TeleWorldLoc, SmoothedLoc, TeleForwardVec, LaserBeamRadius);

			FHitResult TraceResult;
			
			const FVector TraceEndLoc = SmoothedLoc + (UKismetMathLibrary::Normal((SmoothedLoc - TeleWorldLoc),0.0001) * 100.0f);
			const bool bIsHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), TeleWorldLoc, TraceEndLoc,
			LaserBeamTraceChannel, false, ActorsToIgnore,EnableDebugMode ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, TraceResult, true);

			LaserBeamHitResult = TraceResult;
			if(bIsHit)
			{
				LastLaserHitResult = TraceResult;
				Wic->SetCustomHitResult(LastLaserHitResult);
				LaserHighlightingObject = TraceResult.Component.Get();
			}
			else
			{
				FHitResult TempHitResult;
				Wic->SetCustomHitResult(TempHitResult);
			}
		}

		// Straight Laser Logic
		else
		{
			LaserBeam->SetWorldLocation(TeleWorldLoc);
			LaserBeam->SetRelativeRotation(ControllerRotationOffset);
			if(bWasHit)
			{
				FVector Scale = FVector(LastLaserHitResult.Time * LaserBeamMaxDistance, 1.f, 1.f);
				LaserBeam->SetWorldScale3D(Scale);
				Wic->SetCustomHitResult(LastLaserHitResult);
				FVector LaserLoc = LastLaserHitResult.TraceStart + (UKismetMathLibrary::Normal(LastLaserHitResult.TraceStart - LastLaserHitResult.TraceEnd)) * LastLaserHitResult.Time * LaserBeamMaxDistance;
				LaserBeamEndPoint->SetWorldLocation(LaserLoc);
				LaserBeamEndPoint->SetHiddenInGame(false);
			}
			else
			{
				FVector Scale = FVector(LastLaserHitResult.Time * LaserBeamMaxDistance, 1.f, 1.f);
				LaserBeam->SetWorldScale3D(Scale);
				LaserBeamEndPoint->SetHiddenInGame(true);
			}
		}
	}
}

void ATeleportController::CreateLaserSpline()
{
	if(bUseSmoothLaser)
	{
		const FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget,
			EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, true);
		UMaterialInstanceDynamic* MID = NULL;
		for(int32 i = 0; i <= NumberOfLaserSplinePoints-1 ; i++)
		{
			USplineMeshComponent* NewMesh = NewObject<USplineMeshComponent>(this);
			
			NewMesh->SetStaticMesh(BaseSplineMesh);
			NewMesh->RegisterComponent();
			MID = NewMesh->CreateDynamicMaterialInstance(0);
			float Time = 1;
			if(i==0)Time = 0;
			if(i==NumberOfLaserSplinePoints-1)Time = -1;
			
			MID->SetScalarParameterValue(FName("TimeValue"), Time);
			
			NewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			NewMesh->SetGenerateOverlapEvents(false);
			
			NewMesh->AttachToComponent(LaserSpline, AttachRules);
			LaserSplineMeshes.Add(NewMesh);
		}
	}
}

bool ATeleportController::IfOverWidget_Use(bool bPressed)
{
	if(bIsLaserBeamActive)
	{
		if(Wic->IsOverInteractableWidget() || Wic->IsOverFocusableWidget())
		{
			if(bPressed)
			{
				Wic->PressPointerKey(EKeys::LeftMouseButton);
				return UKismetMathLibrary::BooleanOR(Wic->IsOverInteractableWidget(), Wic->IsOverFocusableWidget());
			}
			else
			{
				Wic->ReleasePointerKey(EKeys::LeftMouseButton);
				return UKismetMathLibrary::BooleanOR(Wic->IsOverInteractableWidget(), Wic->IsOverFocusableWidget());
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void ATeleportController::InitController()
{
	if(bIsLocal)
	{
		EControllerHand ControllerHand;
		OwningMotionController->GetHandType(ControllerHand);
		switch (ControllerHand)
		{
		case EControllerHand::Left:
			Wic->VirtualUserIndex = 0;
			Wic->PointerIndex = 0;
			break;
			
		case EControllerHand::Right:
			Wic->VirtualUserIndex = 0;
			Wic->PointerIndex = 1;
			break;
			
		default:
			break;
		}
	}
}

void ATeleportController::ToggleTick()
{
	SetActorTickEnabled(bIsLaserBeamActive || bIsTeleporterActive || IsValid(ActorBeingThrown));
}

void ATeleportController::ClearLaserBeam()
{
	for(const auto Mesh : LaserSplineMeshes)
	{
		Mesh->DestroyComponent();
	}
	LaserSplineMeshes.Empty();
	LaserSpline->ClearSplinePoints(true);
}

void ATeleportController::FilterGrabSpline(TArray<FVector>& Locations, FVector& Target)
{
	if(Locations.Num() > 1)
	{
		int32 ArrayIndex = 0;
		for(FVector Loc : Locations)
		{
			ArrayIndex++;
			if(UKismetMathLibrary::BooleanOR(UKismetMathLibrary::VSizeSquared(Loc - Target) < ClosestDist,
				ClosestDist == 0.f))
			{
				ClosestIndex = ArrayIndex;
				ClosestDist = UKismetMathLibrary::VSizeSquared(Loc - Target);
			}
		}
		for(int32 i = Locations.Num() - 1; i >= 0; i--)
		{
			if(i > ClosestIndex)
			{
				Locations.RemoveAt(ArrayIndex);
			}
		}
		Locations[ClosestIndex] = Target;
	}
}

void ATeleportController::DisableWidgetActivation()
{
	FHitResult HitResult;
	Wic->SetCustomHitResult(HitResult);
	Wic->Deactivate();	
}


void ATeleportController::Print(FString Message,int key,FColor Color)
{
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(key, 2, Color, Message);
}

void ATeleportController::Print(int Message, int key, FColor Color)
{
	FString NumAsString = FString::FromInt(Message);
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(key, 4, Color, NumAsString);
}