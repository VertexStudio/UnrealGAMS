// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "easygearcomponent.h"
#include "Net/UnrealNetwork.h"

void UEasyGear::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEasyGear, Enabled);
	DOREPLIFETIME(UEasyGear, NoseEnabled);
	DOREPLIFETIME(UEasyGear, MainLeftEnabled);
	DOREPLIFETIME(UEasyGear, MainRightEnabled);

	DOREPLIFETIME(UEasyGear, SteerToVector);

	DOREPLIFETIME(UEasyGear, NoseSteering);

	DOREPLIFETIME(UEasyGear, SteeringInput);
	DOREPLIFETIME(UEasyGear, LeftBrakeInput);
	DOREPLIFETIME(UEasyGear, RightBrakeInput);
	DOREPLIFETIME(UEasyGear, GuidedVector);
};

float UEasyGear::SetSteeringInput(float Input) {
	ClientSteeringInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		SteeringInput = Input;
	}
	return Input;
}

float UEasyGear::SetLeftBrakeInput(float Input) {
	ClientLeftBrakeInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		LeftBrakeInput = Input;
	}
	return Input;
}

float UEasyGear::SetRightBrakeInput(float Input) {
	ClientRightBrakeInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		RightBrakeInput = Input;
	}
	return Input;
}

void UEasyGear::InputReplicate() {
	if (TimeSinceInputUpdate >= 1.0f / InputReplicationFrequency) {
		if ((ClientSteeringInput != SteeringInput) ||
			(ClientLeftBrakeInput != LeftBrakeInput) ||
			(ClientRightBrakeInput != RightBrakeInput)) {
			InputRep(ClientSteeringInput, ClientLeftBrakeInput, ClientRightBrakeInput);

			TimeSinceInputUpdate = 0.0f;
		}
	}
}

void UEasyGear::InputRep_Implementation(float Steering, float LeftBrake, float RightBrake) {
	SteeringInput = Steering;
	LeftBrakeInput = LeftBrake;
	RightBrakeInput = RightBrake;
}

bool UEasyGear::InputRep_Validate(float Steering, float LeftBrake, float RightBrake) {
	return true;
}