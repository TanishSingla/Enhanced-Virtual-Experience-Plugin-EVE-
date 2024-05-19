#pragma once

#include "CoreMinimal.h"
#include "VRBPDatatypes.h"
#include "GameFramework/Actor.h"
#include "TeleportController.generated.h"

class UPhysicsTossManager;
struct FBPEuroLowPassFilter;
class USplineComponent;
class USplineMeshComponent;
class UWidgetInteractionComponent;

UCLASS()
class VREXPANSIONPLUGIN_API ATeleportController : public AActor
{
	GENERATED_BODY()

public:

	ATeleportController();
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void ServerSideToss(UPrimitiveComponent * TargetObject);
	
	UFUNCTION()
	void OnTeleportControllerClassConstructed();

	UFUNCTION(BlueprintCallable, Category = Laser)
	void SetLaserBeamActive(bool bLaserBeamActive);

	UFUNCTION()
	void UpdateLaserBeam(const float& Deltatime);

	UFUNCTION()
	void CreateLaserSpline();

	UFUNCTION(BlueprintCallable)
	bool IfOverWidgetUse(bool bPressed,bool bIsHandInteracting=false);

	UFUNCTION()
	void InitController();

	UFUNCTION(BlueprintCallable)
	void ToggleTick();

	UFUNCTION()
	void ClearLaserBeam();

	UFUNCTION()
	void FilterGrabSpline(TArray<FVector>& Locations, FVector& Target);

	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void DisableWidgetActivation();

	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void GetTeleWorldLocAndForwardVector(FVector& WorldLoc, FVector& ForwardVecto, bool bUseRotOffset = true);

	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void ActivateTeleporter();
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void DeactivateTeleporter();
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void TraceTeleportDestination(bool& bSuccess, TArray<FVector>& TracePoints, FVector& NavMeshLocation, FVector& TraceLocation);
	
	UFUNCTION()
	void ClearArc();
	
	UFUNCTION()
	void UpdateArcSpline(bool bFoundValidLoc, TArray<FVector> SplinePoints);
	
	UFUNCTION()
	void UpdateArcEndpoint(const FVector& NewLoc, bool bValidLocFound);
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void GetTeleportDestination(bool RelativeToHMD, FVector& Location, FRotator& Rotation);

	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void RumbleController(UHapticFeedbackEffect_Base * HapticEff,float Intensity = 1.0f);

	UFUNCTION(BlueprintImplementableEvent)
	void GrabTrigger();
	
	/**
	 * Variables
	 */

	UPROPERTY()
	AActor * PreviousFrameHitActor = nullptr;

	UPROPERTY()
	AActor* CurrentFrameHitActor = nullptr;

	UPROPERTY(BlueprintReadOnly,Category = "Laser")
	FHitResult LaserBeamHitResult;
	
	UPROPERTY(BlueprintReadWrite,Category = "Defaults")
	bool bLastFrameValidDestination = false;
	
	UPROPERTY(BlueprintReadOnly,Category = "Defaults")
	bool IsValidTeleportDestination;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components, meta = (ExposeOnSpawn = "True"))
	UGripMotionControllerComponent* OwningMotionController;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	bool bIsLocal;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	bool bIsTeleporterActive;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FRotator TeleportRotation;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FRotator TeleportBaseRotation;
	
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	bool bIsLaserBeamActive = false;
	
	
protected:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	
	/**
	 * Components
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	USceneComponent* Scene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	USplineComponent* ArcSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	USplineComponent* LaserSpline;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UStaticMesh* BaseSplineMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UStaticMeshComponent* ArcEndPoint;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	USplineMeshComponent* MySplineMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UStaticMeshComponent* LaserBeamEndPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UStaticMeshComponent* TeleportCylinder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UStaticMeshComponent* Ring;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UStaticMeshComponent* Arrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UStaticMeshComponent* FinalFacingArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	USplineMeshComponent* LaserBeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UWidgetInteractionComponent* Wic;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = Components)
	UPrimitiveComponent* LaserHighlightingObject;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	UPrimitiveComponent* ActorBeingThrown;
	
	/**
	 * Variables
	 */

	// Material Instances Variables :- 
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Materials")
	UMaterialInstance * TeleportSplineStartingValidMaterial = NULL;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Materials")
	UMaterialInstance * TeleportSplineStartingInvalidMaterial = NULL;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Materials")
	UMaterialInstance * TeleportSplineValidMaterial = NULL;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Materials")
	UMaterialInstance * TeleportSplineInvalidMaterial = NULL;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Materials")
	UMaterialInstance * SmoothLaserBeamMaterial = NULL;


	// Haptics Variables :- 
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Haptics")
	float RumbleControllerIntensity = 1.0f;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Haptics")
	UHapticFeedbackEffect_Base * LaserHoverHaptic = nullptr;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Haptics")
	UHapticFeedbackEffect_Base * ValidTeleportHaptic = nullptr;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Haptics")
	UHapticFeedbackEffect_Base * InvalidTeleportHaptic = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Properties | Tracing")
	FVector TraceEndLocation = FVector(0,0,-200.0);

	UPROPERTY(EditDefaultsOnly,Category = "Properties | Tracing")
	TEnumAsByte<ECollisionChannel> TeleportTraceChannel = ECC_WorldStatic;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Tracing")
	FRotator ControllerRotationOffset = FRotator(-60.f, 0.f, 0.f);

	UPROPERTY(EditDefaultsOnly,Category = "Properties | Laser")
	TEnumAsByte<ETraceTypeQuery> LaserBeamTraceChannel;

	UPROPERTY(EditDefaultsOnly,Category = "Properties | Laser")
	float LaserBeamTraceOffset = 10.f;
	
	UPROPERTY(EditDefaultsOnly,Category = "Properties | Laser")
	bool bEnableDebugMode = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Properties | Laser")
	bool bUseSmoothLaser = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Laser")
	float LaserBeamMaxDistance = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Laser")
	float LaserBeamRadius = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Laser")
	int32 NumberOfLaserSplinePoints = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Teleport")
	float TeleportLaunchSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Teleport")
	float ProjectNavExtends = 500.f;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Properties | Tracing")
	FHitResult LastLaserHitResult;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Properties | Teleport")
	FVector LastValidTeleportLocation;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties | Laser")
	FBPEuroLowPassFilter LowPassFilter;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties")
	TArray<USplineMeshComponent*> SplineMeshes;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Properties")
	TArray<USplineMeshComponent*> LaserSplineMeshes;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Properties")
	FRotator InitialControllerRotation;

	UPROPERTY()
	float ClosestDist;

	UPROPERTY()
	int32 ClosestIndex;
	
	UPROPERTY()
	int32 PointDiffNum;

	void Print(FString Message,int key = 1 ,FColor Color  =FColor::Red);
	void Print(int Message,int key = 1 ,FColor Color  = FColor::Red);
};