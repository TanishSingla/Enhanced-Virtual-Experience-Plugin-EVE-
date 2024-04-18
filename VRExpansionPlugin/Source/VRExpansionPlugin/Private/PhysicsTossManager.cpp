#include "PhysicsTossManager.h"
#include "GripMotionControllerComponent.h"
#include "VRGripInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


UPhysicsTossManager::UPhysicsTossManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPhysicsTossManager::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	RunThrowing();
}

void UPhysicsTossManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	CancelToss();
}

void UPhysicsTossManager::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UPhysicsTossManager::CalledAfterSomeTime()
{
	// GEngine->AddOnScreenDebugMessage(0, 4, FColor::Red, "asdkjsadhjikasdhiasbfahjldsbfadwhjlfbaedwghjdbaeshjl");
	CancelThrowing(ObjectBeingThrown);
}

void UPhysicsTossManager::ToggleTick()
{
	SetComponentTickEnabled(IsValid(ObjectBeingThrown));
}

void UPhysicsTossManager::RunToss(bool bIsHeld)
{
	const float CurrentDistanceSq = UKismetMathLibrary::VSizeSquared(
		OwningMotionController->GetPivotLocation() - ObjectBeingThrown->GetComponentLocation());

	const FVector NormalVector = UKismetMathLibrary::Normal(	
		OwningMotionController->GetPivotLocation() - ObjectBeingThrown->GetComponentLocation());

	FVector LaunchVelocity;
	
	if((CurrentDistanceSq > (CancelDistance*CancelDistance)) && (!bIsHeld) )
	{
		if(UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetWorld(),
			LaunchVelocity, ObjectBeingThrown->GetComponentLocation(),
			OwningMotionController->GetPivotLocation(),0.0,0.8))
		{
			const float SlowDownRange =  UKismetMathLibrary::FClamp(
				1.0 - ((DistanceToStartSlowing*DistanceToStartSlowing)/CurrentDistanceSq),
				0.8, 1.0);

			const FVector FinalVelocity = SlowDownRange * (LaunchVelocity +
				(NormalVector * UKismetMathLibrary::FClamp(
					(MinimumSpeed*MinimumSpeed) - UKismetMathLibrary::VSizeSquared(
						UKismetMathLibrary::Vector_ProjectOnToNormal(LaunchVelocity, NormalVector)),
						0.f, MinimumSpeed)));
			ObjectBeingThrown->SetPhysicsLinearVelocity(FinalVelocity);
		}
		else
		{
			CancelThrowing(ObjectBeingThrown);
		}
	}
	else
	{
		CancelThrowing(ObjectBeingThrown);
	}
	
	// GetWorld()->GetTimerManager().SetTimer(Timer, this, &UPhysicsTossManager::CalledAfterSomeTime, 3.0f, false);
}

void UPhysicsTossManager::CancelToss()
{
	CancelThrowing(ObjectBeingThrown);
}

bool UPhysicsTossManager::IsThrowing()
{
	if(ObjectBeingThrown)
	{
		return true;
	}

	return false;
}

void UPhysicsTossManager::CancelThrowing(UPrimitiveComponent* Object)
{
	if(IsValid(Object))
	{
		//Unbind event
		Object->OnComponentHit.RemoveDynamic(this,&UPhysicsTossManager::CancelThrow);
	}
	// if(ObjectBeingThrown!=nullptr)
	// {
	// 	ObjectBeingThrown->SetNotifyRigidBodyCollision(bGenerateHitEvents);
	// }
	ObjectBeingThrown = nullptr;
	ToggleTick();
}

void UPhysicsTossManager::RunThrowing()
{
	if(IsValid(ObjectBeingThrown))
	{
		if(const IVRGripInterface * GripInterface = Cast<IVRGripInterface>(ObjectBeingThrown))
		{
			TArray<FBPGripPair> HoldingControllers;
			bool bIsHeld;
			
			GripInterface->Execute_IsHeld(ObjectBeingThrown,HoldingControllers,bIsHeld);
			RunToss(bIsHeld);
		}
	}else
	{
		CancelThrowing(ObjectBeingThrown);
	}
}

void UPhysicsTossManager::ServersideToss(UPrimitiveComponent* TargetObject,
	UGripMotionControllerComponent* TargetMotionController)
{
	if(!IsValid(ObjectBeingThrown))
	{
		if(TargetObject)
		{
			TSubclassOf<UVRGripInterface>Interface =  UVRGripInterface::StaticClass();
			if(UKismetSystemLibrary::DoesImplementInterface(TargetObject->GetOwner(),Interface))	
			{
				// GEngine->AddOnScreenDebugMessage(10, 4, FColor::Green, "Server Side Tossssssss");
				
				bool IsHeld = false;
				IVRGripInterface * GripInterface = Cast<IVRGripInterface>(TargetObject->GetOwner());
				TArray<FBPGripPair> HoldingControllers;
				if(GripInterface)
				{
					// GEngine->AddOnScreenDebugMessage(3, 4, FColor::Green, "Called");
					GripInterface->Execute_IsHeld(TargetObject->GetOwner(),HoldingControllers,IsHeld);
				}
				
				if(!IsHeld)
				{
					OwningMotionController = TargetMotionController;
					ObjectBeingThrown = TargetObject;
				}
					
				// ObjectBeingThrown->SetNotifyRigidBodyCollision(true);
					
					
					//Bind Event
					ObjectBeingThrown->OnComponentHit.AddDynamic(this,&UPhysicsTossManager::CancelThrow);
					ObjectBeingThrown->SetSimulatePhysics(true);
					
					RunToss(IsHeld);
					ToggleTick();
				}
			}
		}
	}		


void UPhysicsTossManager::CancelThrow(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector Impulse, const FHitResult& Hit)
{
	CancelThrowing(HitComponent);
}	

