#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VR_Spectator.generated.h"

UCLASS()
class ENHANCEDVIRTUALEXPERIENCE_API AVR_Spectator : public AActor
{
	GENERATED_BODY()

public:
	AVR_Spectator();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	USceneCaptureComponent2D* SceneCapture;

	UPROPERTY(VisibleDefaultsOnly, Category = Components)
	UTextureRenderTarget2D* RenderTarget;

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
