// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"

DECLARE_CYCLE_STAT(TEXT("EasyFM Net Move Broadcast"), STAT_MoveBroadcast, STATGROUP_EasyFM);
DECLARE_CYCLE_STAT(TEXT("EasyFM Net Move Receive"), STAT_MoveReceive, STATGROUP_EasyFM);
DECLARE_CYCLE_STAT(TEXT("EasyFM Net Move Smoothing"), STAT_MoveSmoothing, STATGROUP_EasyFM);
DECLARE_CYCLE_STAT(TEXT("EasyFM Net GetReplicatedProps"), STAT_GetProps, STATGROUP_EasyFM);
DECLARE_CYCLE_STAT(TEXT("EasyFM Net PreReplication"), STAT_PreRep, STATGROUP_EasyFM);


void UeasyFM::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	SCOPE_CYCLE_COUNTER(STAT_GetProps);

	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UeasyFM, Wind);

	DOREPLIFETIME(UeasyFM, AdditionalDrag);
	DOREPLIFETIME(UeasyFM, Buoyancy);

	DOREPLIFETIME(UeasyFM, AlphaLimiter);
	DOREPLIFETIME(UeasyFM, SlipLimiter);
	DOREPLIFETIME(UeasyFM, StabilityAugmentation);
	DOREPLIFETIME(UeasyFM, AutoTrim);
	DOREPLIFETIME(UeasyFM, AutoThrottle);
	DOREPLIFETIME(UeasyFM, AutoFlaps);

	DOREPLIFETIME(UeasyFM, Guided);

	DOREPLIFETIME_CONDITION(UeasyFM, ThrottleInput, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, PitchInput, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, RollInput, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, YawInput, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, FlapsInput, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, SpoilerInput, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, VTOLInput, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, ReverserInput, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, GuidedVector, COND_Custom);

	DOREPLIFETIME_CONDITION(UeasyFM, PitchTrimPos, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, YawTrimPos, COND_Custom);
	DOREPLIFETIME_CONDITION(UeasyFM, RollTrimPos, COND_Custom);

	DOREPLIFETIME_CONDITION(UeasyFM, GotServerPosition, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, ServerLocation, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, ServerRotation, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, ServerLinearVelocity, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, ServerAngularVelocity, COND_InitialOnly);

	DOREPLIFETIME_CONDITION(UeasyFM, PitchPos, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, YawPos, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, RollPos, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, ThrottlePos, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, VTOLPos, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, FlapsPos, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, SpoilerPos, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(UeasyFM, AutoThrottlePos, COND_InitialOnly);
}

void UeasyFM::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	SCOPE_CYCLE_COUNTER(STAT_PreRep);

	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, ThrottleInput, MulticastInputs);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, PitchInput, MulticastInputs);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, RollInput, MulticastInputs);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, YawInput, MulticastInputs);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, FlapsInput, MulticastInputs);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, SpoilerInput, MulticastInputs);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, VTOLInput, MulticastInputs);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, ReverserInput, MulticastInputs);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, GuidedVector, MulticastInputs);

	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, PitchTrimPos, MulticastTrim);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, YawTrimPos, MulticastTrim);
	DOREPLIFETIME_ACTIVE_OVERRIDE(UeasyFM, RollTrimPos, MulticastTrim);
}

void UeasyFM::MovementBroadcast(FBodyInstance* BodyInst) {
	SCOPE_CYCLE_COUNTER(STAT_MoveBroadcast);

	if (MovementReplicationFrequency <= 0.0f) return;

	if (TimeSincePosUpdate>=1.0f/MovementReplicationFrequency) {
		float overshoot = TimeSincePosUpdate - (1.0f/MovementReplicationFrequency);
		FTransform Transform = BodyInst->GetUnrealWorldTransform();
		
		FVector LocalLoc = Transform.GetLocation();
		FRotator LocalRot = FRotator(Transform.GetRotation());
		FVector LocalVel = BodyInst->GetUnrealWorldVelocity();
		FVector LocalAngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();

		//rebase
		LocalLoc = UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(), LocalLoc);

		if (HighPrecision) {
			PositionRep(LocalLoc + LocalVel*overshoot, LocalRot, LocalVel, LocalAngVel);
		}
		else {
			PositionRepLow(LocalLoc + LocalVel*overshoot, LocalRot, LocalVel, LocalAngVel);
		}

		TimeSincePosUpdate=FMath::Fmod(TimeSincePosUpdate, 1.0f / MovementReplicationFrequency);
	}
}

void UeasyFM::PositionRep_Implementation(FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity) {
	//do not receive updates if hosting, or parent not valid
	SCOPE_CYCLE_COUNTER(STAT_MoveReceive);

	UPrimitiveComponent* Parent = Cast<UPrimitiveComponent>(GetAttachParent());

	if ((GetOwner()->Role != ROLE_Authority) && (Parent != nullptr)) {
		FName ParentSocket = GetAttachSocketName();
		FBodyInstance* BodyInst = Parent->GetBodyInstance(ParentSocket);
		if (!BodyInst) { return; }

		//rebase
		NewLocation = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);

		//latency

		NewLocation += NewVelocity*AdditionalLatency;
		FQuat LagCompDelta = FQuat(NewAngularVelocity, AdditionalLatency * PI / 180.0f);
		NewRotation = (LagCompDelta*NewRotation.Quaternion()).Rotator();

		SoftSync(BodyInst, NewLocation, NewRotation, NewVelocity, NewAngularVelocity);
	}
}

void UeasyFM::PositionRepLow_Implementation(FVector_NetQuantize10 NewLocation, FRotator NewRotation, FVector_NetQuantize NewVelocity, FVector_NetQuantize10 NewAngularVelocity) {
	SCOPE_CYCLE_COUNTER(STAT_MoveReceive);

	UPrimitiveComponent* Parent = Cast<UPrimitiveComponent>(GetAttachParent());

	//do not receive updates if hosting, or parent not valid
	if ((GetOwner()->Role != ROLE_Authority) && (Parent != nullptr)) {

		FName ParentSocket = GetAttachSocketName();
		FBodyInstance* BodyInst = Parent->GetBodyInstance(ParentSocket);
		if (!BodyInst) { return; }

		//rebase
		NewLocation = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);

		//latency
		NewLocation += NewVelocity*AdditionalLatency;
		FQuat LagCompDelta = FQuat(NewAngularVelocity, AdditionalLatency * PI / 180.0f);
		NewRotation = (LagCompDelta*NewRotation.Quaternion()).Rotator();

		SoftSync(BodyInst, NewLocation, NewRotation, NewVelocity, NewAngularVelocity);
	}
}

void UeasyFM::MovementHardSync() {
	UPrimitiveComponent* Parent = Cast<UPrimitiveComponent>(GetAttachParent());
	if (!Parent) {return;}
	FName ParentSocket = GetAttachSocketName();
	FBodyInstance* BodyInst = Parent->GetBodyInstance(ParentSocket);
	if (!BodyInst) { return; }

	FTransform Transform = BodyInst->GetUnrealWorldTransform();

	FVector LocalLoc = Transform.GetLocation();
	FRotator LocalRot = FRotator(Transform.GetRotation());
	FVector LocalVel = BodyInst->GetUnrealWorldVelocity();
	FVector LocalAngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();

	//rebase
	LocalLoc = UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(), LocalLoc);

	PositionRepHard(LocalLoc, LocalRot, LocalVel, LocalAngVel);
}

void UeasyFM::PositionRepHard_Implementation(FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity) {
	SCOPE_CYCLE_COUNTER(STAT_MoveReceive);

	UPrimitiveComponent* Parent = Cast<UPrimitiveComponent>(GetAttachParent());

	//do not receive updates if hosting, or parent not valid
	if ((GetOwner()->Role != ROLE_Authority) && (Parent != nullptr)) {

		FName ParentSocket = GetAttachSocketName();
		FBodyInstance* BodyInst = Parent->GetBodyInstance(ParentSocket);

		if (!BodyInst) { return; }

		FTransform Transform = BodyInst->GetUnrealWorldTransform();

		//rebase
		NewLocation = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);

		//latency
		NewLocation += NewVelocity*AdditionalLatency;
		FQuat LagCompDelta = FQuat(NewAngularVelocity, AdditionalLatency * PI / 180.0f);
		NewRotation = (LagCompDelta*NewRotation.Quaternion()).Rotator();

		PreviousVelocity = NewVelocity;
		BodyInst->SetBodyTransform(FTransform(NewRotation,NewLocation,Transform.GetScale3D()), ETeleportType::TeleportPhysics);
		BodyInst->SetLinearVelocity(NewVelocity,false);
		BodyInst->SetAngularVelocityInRadians(NewAngularVelocity,false);

		LocError = FVector(0,0,0);
		RotError = FQuat::Identity;
		VelError = FVector(0, 0, 0);
		AngVelError = FVector(0, 0, 0);
	}
}


void UeasyFM::StoreServerPosition() {
	UPrimitiveComponent* Parent = Cast<UPrimitiveComponent>(GetAttachParent());
	if (!Parent) {
		GotServerPosition = false;
		return;
	}
	FName ParentSocket = GetAttachSocketName();
	FBodyInstance* BodyInst = Parent->GetBodyInstance(ParentSocket);

	if (!BodyInst) { 
		GotServerPosition = false;
		return; 
	}
	GotServerPosition = true;

	FTransform Transform = BodyInst->GetUnrealWorldTransform();

	//ServerLocation = LocalParent->GetComponentLocation();
	//rebase
	ServerLocation = UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(),Transform.GetLocation());
	ServerRotation = FRotator(Transform.GetRotation());
	ServerLinearVelocity = BodyInst->GetUnrealWorldVelocity();
	ServerAngularVelocity = BodyInst->GetUnrealWorldAngularVelocityInRadians();
}

void UeasyFM::MoveToServerPosition() {
	if (!GotServerPosition) { return; }

	UPrimitiveComponent* Parent = Cast<UPrimitiveComponent>(GetAttachParent());
	if (!Parent) { return; }
	FName ParentSocket = GetAttachSocketName();
	FBodyInstance* BodyInst = Parent->GetBodyInstance(ParentSocket);
	if (!BodyInst) { return; }

	FTransform Transform = BodyInst->GetUnrealWorldTransform();
	//rebase
	BodyInst->SetBodyTransform(FTransform(ServerRotation, UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), ServerLocation),Transform.GetScale3D()), ETeleportType::TeleportPhysics);
	BodyInst->SetLinearVelocity(ServerLinearVelocity,false);
	BodyInst->SetAngularVelocityInRadians(ServerAngularVelocity,false);
	PreviousVelocity = ServerLinearVelocity;
}

void UeasyFM::SoftSync(FBodyInstance* BodyInst, FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity) {

	FTransform Transform = BodyInst->GetUnrealWorldTransform();

	FVector LocalLoc = Transform.GetLocation();
	FRotator LocalRot = FRotator(Transform.GetRotation());
	FVector LocalVel = BodyInst->GetUnrealWorldVelocity();
	FVector LocalAngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();

	//jitter filter
	if (JitterFilter) {
		FVector FilteredLocation = FMath::ClosestPointOnSegment(NewLocation - (LocalVel * JitterFilterMaxTime), NewLocation + (LocalVel * JitterFilterMaxTime), LocalLoc);
		NewLocation = FMath::Lerp(FilteredLocation, FVector(NewLocation), JitterFilterCentering);
	}

	LocError = NewLocation - LocalLoc;
	RotError = FQuat(NewRotation.Quaternion()*LocalRot.Quaternion().Inverse());
	VelError = NewVelocity - LocalVel;
	AngVelError = NewAngularVelocity - LocalAngVel;

	if ((!LocationSmoothing) || (!RotationSmoothing)) {
		FVector Loc = Transform.GetLocation();
		FQuat Rot = Transform.GetRotation();

		if (!RotationSmoothing) {
			Loc = NewLocation;
		}

		if (!RotationSmoothing) {
			Rot = NewRotation.Quaternion();
		}

		BodyInst->SetBodyTransform(FTransform(Rot,Loc,Transform.GetScale3D()), ETeleportType::TeleportPhysics);
	}

	if (!VelocitySmoothing) {
		PreviousVelocity += VelError;
		BodyInst->SetLinearVelocity(NewVelocity,false);
	}

	if (!AngularVelocitySmoothing) {
		BodyInst->SetAngularVelocityInRadians(NewAngularVelocity,false);
	}

	if (LocError.Size() > TeleportThreshold) {
		LocError = FVector(0);
		RotError = FQuat::Identity;
		VelError = FVector(0);
		AngVelError = FVector(0);

		BodyInst->SetBodyTransform(FTransform(NewRotation, NewLocation, Transform.GetScale3D()), ETeleportType::TeleportPhysics);
		BodyInst->SetLinearVelocity(NewVelocity,false);
		BodyInst->SetAngularVelocityInRadians(NewAngularVelocity,false);

		PreviousVelocity = NewVelocity;
	}
}

void UeasyFM::ErrorCatchUp(FBodyInstance* BodyInst, float DeltaTime) {
	SCOPE_CYCLE_COUNTER(STAT_MoveSmoothing);

	if (LocationSmoothing || RotationSmoothing) {
		FTransform Transform = BodyInst->GetUnrealWorldTransform();
		FVector LocalLoc = Transform.GetLocation();
		FQuat LocalRot = Transform.GetRotation();
		FVector LocalScale = Transform.GetScale3D();

		if (LocationSmoothing) {
			float ErrorDelta = FMath::Clamp(DeltaTime / SmoothingTime, 0.0f, 1.0f);
			LocalLoc = LocalLoc + (LocError*ErrorDelta);
			LocError *= (1.0 - ErrorDelta);
		}
		if (RotationSmoothing) {
			float ErrorDelta = FMath::Clamp(DeltaTime / SmoothingTime, 0.0f, 1.0f);
			LocalRot = FQuat::Slerp(LocalRot, RotError*LocalRot, ErrorDelta);
			RotError = FQuat::Slerp(RotError, FQuat::Identity, ErrorDelta);
		}
		BodyInst->SetBodyTransform(FTransform(LocalRot, LocalLoc, LocalScale),ETeleportType::TeleportPhysics);
	}

	if (VelocitySmoothing) {
		float ErrorDelta = FMath::Clamp(DeltaTime / SmoothingTime, 0.0f, 1.0f);
		FVector LocalVel = BodyInst->GetUnrealWorldVelocity();
		BodyInst->SetLinearVelocity(VelError*ErrorDelta,true);
		PreviousVelocity += VelError*ErrorDelta;
		VelError *= (1.0 - ErrorDelta);
	}
	if (AngularVelocitySmoothing) {
		float ErrorDelta = FMath::Clamp(DeltaTime / SmoothingTime, 0.0f, 1.0f);
		FVector LocalAngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();
		BodyInst->SetAngularVelocityInRadians(AngVelError*ErrorDelta,true);
		AngVelError *= (1.0 - ErrorDelta);
	}
}