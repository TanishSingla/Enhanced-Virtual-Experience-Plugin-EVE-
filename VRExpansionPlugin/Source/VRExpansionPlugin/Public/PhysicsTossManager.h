// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VRGripInterface.h"
#include "Components/ActorComponent.h"
#include "PhysicsTossManager.generated.h"


class UGripMotionControllerComponent;


UCLASS(Blueprintable, ClassGroup=(VRE))
class VREXPANSIONPLUGIN_API UPhysicsTossManager : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UPhysicsTossManager();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginPlay() override;

	FTimerHandle Timer;
	void CalledAfterSomeTime();
	
	UFUNCTION(BlueprintCallable, Category = TossManager)
	void ToggleTick();
	
	UFUNCTION( BlueprintCallable,Category = TossManager)
	void RunToss(bool bIsHeld);
	
	UFUNCTION( BlueprintCallable,Category = TossManager)
	void CancelToss();
	
	UFUNCTION(BlueprintCallable,Category = TossManager)
	bool IsThrowing();

	UFUNCTION(  BlueprintCallable,Category = TossManager)
	void CancelThrowing(UPrimitiveComponent* Object);
	
	UFUNCTION( BlueprintCallable,Category = TossManager)
	void RunThrowing();

	UFUNCTION( BlueprintCallable,Category = TossManager)
	void ServersideToss(UPrimitiveComponent * TargetObject,UGripMotionControllerComponent * TargetMotionController);

	// UFUNCTION(BlueprintImplementableEvent)
	// void TempFun(UPrimitiveComponent * Obj);
	
	
	UFUNCTION(BlueprintCallable,Category= TossManager)
	void CancelThrow(UPrimitiveComponent * HitComponent,AActor * OtherActor,UPrimitiveComponent * OtherComp,FVector Impulse,const FHitResult &Hit);
	

	UPROPERTY(EditDefaultsOnly)
	float CancelDistance = 20.f;

	UPROPERTY(EditDefaultsOnly)
	float DistanceToStartSlowing = 100.f;

	UPROPERTY(EditDefaultsOnly)
	float MinimumSpeed = 40.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	UPrimitiveComponent* ObjectBeingThrown;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "True"))
	UGripMotionControllerComponent* OwningMotionController;

	bool bGenerateHitEvents = false;
};
