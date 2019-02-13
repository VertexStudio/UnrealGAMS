// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "easygearcomponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/Engine.h"

UEasyGear::UEasyGear()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bReplicates = true;
	OnCalculateCustomPhysics.BindUObject(this, &UEasyGear::CustomPhysics);

	SetTickGroup(ETickingGroup::TG_PrePhysics);
	// ...
}

// Called when the game starts
void UEasyGear::BeginPlay()
{
	Super::BeginPlay();
};

//physics substep
void UEasyGear::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	Simulate(BodyInstance, DeltaTime, true, false);
};

void UEasyGear::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Enabled) return;

	UPrimitiveComponent *Parent = Cast<UPrimitiveComponent>(GetAttachParent());
	FName ParentSocket = GetAttachSocketName();

	//idiot-proofing
	if (!Parent) {
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("EASYGEAR ERROR - not attached to UPrimitiveComponent"));
		return;
	};
	if (!Parent->IsSimulatingPhysics()) {
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("EASYGEAR ERROR - parent object doesn't simulate physics"));
		return;
	};

	FBodyInstance *bodyInst = Parent->GetBodyInstance(ParentSocket);
	if (!bodyInst) {
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("EASYGEAR ERROR - invalid body instance"));
		return;
	}

	if (ReplicateInputs) {
		if (GetOwner()->Role == ROLE_AutonomousProxy) {
			TimeSinceInputUpdate += DeltaTime;
			InputReplicate();
		}
	}

	//substep
	bool Substep;

	const UPhysicsSettings* Settings = GetDefault<UPhysicsSettings>();
	if (Settings) {
		Substep = Settings->bSubstepping;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Project settings inaccessible"));
		Substep = false;
	}

	if (Substep) {
		bodyInst->AddCustomPhysics(OnCalculateCustomPhysics);
	}
	else {
		Simulate(bodyInst, DeltaTime, true, false);
	};

#ifdef WITH_EDITOR
	if (DrawDebug) {
		Simulate(bodyInst, DeltaTime, false, true);
	};
#endif
};