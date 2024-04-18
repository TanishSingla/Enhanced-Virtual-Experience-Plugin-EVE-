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

	UFUNCTION(BlueprintCallable, Category = Laser)
	void UpdateLaserBeam(const float& Deltatime);

	UFUNCTION(BlueprintCallable, Category = Laser)
	void CreateLaserSpline();

	UFUNCTION(BlueprintCallable)
	bool IfOverWidget_Use(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void InitController();

	UFUNCTION(BlueprintCallable)
	void ToggleTick();

	UFUNCTION(BlueprintCallable)
	void ClearLaserBeam();

	UFUNCTION(BlueprintCallable)
	void FilterGrabSpline(TArray<FVector>& Locations, FVector& Target);

	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void DisableWidgetActivation();

	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void GetTeleWorldLocAndForwardVector(FVector& WorldLoc, FVector& ForwardVecto, bool bUseRotOffset = true);

	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void ActivateTeleporter();

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	USplineMeshComponent* MySplineMeshComponent;
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void DeactivateTeleporter();
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void TraceTeleportDestination(bool& bSuccess, TArray<FVector>& TracePoints, FVector& NavMeshLocation, FVector& TraceLocation);
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void ClearArc();
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void UpdateArcSpline(bool bFoundValidLoc, TArray<FVector> SplinePoints);
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void UpdateArcEndpoint(const FVector& NewLoc, bool bValidLocFound);
	
	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void GetTeleportDestination(bool RelativeToHMD, FVector& Location, FRotator& Rotation);

	UFUNCTION(BlueprintCallable, Category = Teleportation)
	void RumbleController(float Intensity);
	
	/**
	 * Variables
	 */

	UPROPERTY(EditDefaultsOnly,Category = "LaserBeam")
	float LaserBeamTraceOffset;
	
	UPROPERTY(EditDefaultsOnly,Category = "LaserBeam")
	bool EnableDebugMode = false;

	UPROPERTY(BlueprintReadOnly,Category = "LaserBeam")
	FHitResult LaserBeamHitResult;
	
	UPROPERTY(EditDefaultsOnly,Category = "LaserBeam")
	TEnumAsByte<ETraceTypeQuery> LaserBeamTraceChannel;
	
	UPROPERTY(BlueprintReadWrite,Category = "Defaults")
	bool bLastFrameValidDestination = false;
	
	UPROPERTY(BlueprintReadWrite,Category = "Defaults")
	bool IsValidTeleportDestination;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = Components, meta = (ExposeOnSpawn = "True"))
	UGripMotionControllerComponent* OwningMotionController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	bool bIsLocal;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	bool bIsTeleporterActive;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FRotator TeleportRotation;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FRotator TeleportBaseRotation;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
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

	// UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = Components)
	// UPhysicsTossManager * PhysicsTossManager;
	
	/**
	 * Variables
	 */

	// Material Instances Variables :- 
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	UMaterialInstance * TeleportSplineStartingMaterial = NULL;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	UMaterialInstance * SmoothLaserBeamMaterial = NULL;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	float RumbleControllerIntensity = 0.0f;

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	UHapticFeedbackEffect_Base *HapticEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FVector TraceEndLocation = FVector(0,0,-200.0);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	bool bUseSmoothLaser = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	float LaserBeamMaxDistance = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	float LaserBeamRadius = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	int32 NumberOfLaserSplinePoints = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	float TeleportLaunchSpeed = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	float ProjectNavExtends = 500.f;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FHitResult LastLaserHitResult;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FRotator InitialControllerRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FRotator ControllerRotationOffset = FRotator(-60.f, 0.f, 0.f);

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FVector LastValidTeleportLocation;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	float ClosestDist;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	int32 ClosestIndex;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	TArray<USplineMeshComponent*> SplineMeshes;
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	FBPEuroLowPassFilter LowPassFilter ;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	TArray<USplineMeshComponent*> LaserSplineMeshes;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "True"), Category = "Defaults")
	int32 PointDiffNum;

	void Print(FString Message,int key = 1 ,FColor Color  =FColor::Red);
	void Print(int Message,int key = 1 ,FColor Color  = FColor::Red);
};