#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VRSpectator.generated.h"

UCLASS()
class VREXPANSIONPLUGIN_API AVRSpectator : public AActor
{
	GENERATED_BODY()

public:
	AVRSpectator();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(EditAnywhere, Category = "Spectator Properties")
	float FieldOfView = 90.f;

protected:
	virtual void BeginPlay() override;

private:
	
	/**
	 * Private Functions
	 */
	void EnableSpectator();
	void DisableSpectator();
	void DisableIfMobile();
	void HandleSpectatorFPV();
	void SetSpectatorMode();
};
