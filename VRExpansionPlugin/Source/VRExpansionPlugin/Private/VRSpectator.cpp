
#include "VRSpectator.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


AVRSpectator::AVRSpectator()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SetRootComponent(SceneCapture);
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = SCS_FinalColorLDR;
}

void AVRSpectator::BeginPlay()
{
	Super::BeginPlay();

	DisableIfMobile();
	EnableSpectator();
}

void AVRSpectator::EnableSpectator()
{
	SetActorTickEnabled(true);
	
	SceneCapture->bCaptureEveryFrame = true;

	if(UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::Texture);
		UTexture* Texture = Cast<UTexture>(SceneCapture->TextureTarget);
		UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenTexture(Texture);
	}
	else
	{
		UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetViewTargetWithBlend(this, 0.f,VTBlend_Linear);
	}
}

void AVRSpectator::DisableSpectator()
{
	SetActorTickEnabled(false);
	UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::SingleEyeCroppedToFill);
	SceneCapture->bCaptureEveryFrame = false;
}

void AVRSpectator::DisableIfMobile()
{
	if(UGameplayStatics::GetPlatformName() == "Android")
	{
		this->Destroy();
	}
}

void AVRSpectator::HandleSpectatorFPV()
{
	const FVector SmoothLoc = UKismetMathLibrary::VInterpTo(GetActorLocation(), UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation(),
		GetWorld()->GetDeltaSeconds(), 5.f);
	const FRotator SmoothRot =UKismetMathLibrary::RInterpTo(GetActorRotation(), UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraRotation(),
		GetWorld()->GetDeltaSeconds(), 5.f);
	
	SetActorLocationAndRotation(SmoothLoc, SmoothRot);
}

void AVRSpectator::SetSpectatorMode()
{
	TArray<UCameraComponent*> CameraComps;
	UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetComponents(CameraComps);

	if(CameraComps.Num() > 0)
	{
		TArray<USceneComponent*> ChildrenComps;
		CameraComps[0]->GetChildrenComponents(true, ChildrenComps);

		if(ChildrenComps.Num() > 0)
		{
			for(const auto Comp : ChildrenComps)
			{
				if(UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Comp))
				{
					Comp->SetVisibility(false);
				}
			}
		}
	}
	
}

void AVRSpectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleSpectatorFPV();
}