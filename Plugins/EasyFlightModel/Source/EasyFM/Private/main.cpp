// Copyright 2016 Mookie. All Rights Reserved.

// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"
#include "PhysicsEngine/PhysicsSettings.h"

//DECLARE_CYCLE_STAT(TEXT("EasyFM Tick"), STAT_Tick, STATGROUP_EasyFM);

UeasyFM::UeasyFM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...

	bReplicates = true;
	OnCalculateCustomPhysics.BindUObject(this, &UeasyFM::CustomPhysics);

	SetTickGroup(ETickingGroup::TG_PrePhysics);
}

// Called when the game starts
void UeasyFM::BeginPlay()
{
	Super::BeginPlay();

	//replicate initial position
	if(ReplicateMovement){
		if (GetOwner()->Role == ROLE_Authority) {
			StoreServerPosition();
		}
		else {
			MoveToServerPosition();
		}
	}
}

//physics substep
void UeasyFM::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	Simulate(BodyInstance, DeltaTime, true, false);
}


// Called every frame
void UeasyFM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	//SCOPE_CYCLE_COUNTER(STAT_Tick);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!Enabled) return;

	UPrimitiveComponent* Parent = Cast<UPrimitiveComponent>(GetAttachParent());
	FName ParentSocket = GetAttachSocketName();

	//idiot-proofing
	if (!Parent) {
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("easyFM ERROR - not attached to UPrimitiveComponent"));
		return;
	}
	if (!Parent->IsSimulatingPhysics()) {
		GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("easyFM ERROR - parent object doesn't simulate physics"));
		return;
	}

	FBodyInstance *bodyInst = Parent->GetBodyInstance(ParentSocket);

	//multiplayer
	if (ReplicateMovement) {
		if (GetOwner()->Role == ROLE_Authority) {
			StoreServerPosition();

			TimeSincePosUpdate += DeltaTime;

			MovementBroadcast(bodyInst);
		}
		else {
			ErrorCatchUp(bodyInst, DeltaTime);
		}
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
	}else{
		Simulate(bodyInst, DeltaTime,true,false);
	}

#ifdef WITH_EDITOR
	if (DrawDebug) {
		Simulate(bodyInst, DeltaTime, false, true);
	}
#endif
}

