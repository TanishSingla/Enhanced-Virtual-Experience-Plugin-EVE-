
#include "VR_Spectator.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


AVR_Spectator::AVR_Spectator()
{
	PrimaryActorTick.bCanEverTick = true;
	
	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SetRootComponent(SceneCapture);
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->CaptureSource = SCS_FinalColorLDR;
}

void AVR_Spectator::BeginPlay()
{
	Super::BeginPlay();

	DisableIfMobile();
	EnableSpectator();
}

void AVR_Spectator::EnableSpectator()
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

void AVR_Spectator::DisableSpectator()
{
	SetActorTickEnabled(false);
	UHeadMountedDisplayFunctionLibrary::SetSpectatorScreenMode(ESpectatorScreenMode::SingleEyeCroppedToFill);
	SceneCapture->bCaptureEveryFrame = false;
}

void AVR_Spectator::DisableIfMobile()
{
	if(UGameplayStatics::GetPlatformName() == "Android")
	{
		this->Destroy();
	}
}

void AVR_Spectator::HandleSpectatorFPV()
{
	const FVector SmoothLoc = UKismetMathLibrary::VInterpTo(GetActorLocation(), UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation(),
		GetWorld()->GetDeltaSeconds(), 5.f);
	const FRotator SmoothRot =UKismetMathLibrary::RInterpTo(GetActorRotation(), UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraRotation(),
		GetWorld()->GetDeltaSeconds(), 5.f);
	
	SetActorLocationAndRotation(SmoothLoc, SmoothRot);
}

void AVR_Spectator::SetSpectatorMode()
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

void AVR_Spectator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleSpectatorFPV();
}