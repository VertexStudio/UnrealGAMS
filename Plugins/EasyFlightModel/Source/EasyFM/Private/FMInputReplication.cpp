// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"
#include "Net/UnrealNetwork.h"

//DECLARE_CYCLE_STAT(TEXT("EasyFM Net InputBroadcast"), STAT_InputBroadcast, STATGROUP_EasyFM);

float UeasyFM::SetPitchInput(float Input) {
	ClientPitchInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		PitchInput = Input;
	}
	return Input;
}

float UeasyFM::SetYawInput(float Input) {
	ClientYawInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		YawInput = Input;
	}
	return Input;
}

float UeasyFM::SetRollInput(float Input) {
	ClientRollInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		RollInput = Input;
	}
	return Input;
}

float UeasyFM::SetThrottleInput(float Input) {
	ClientThrottleInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		ThrottleInput = Input;
	}
	return Input;
}

float UeasyFM::SetFlapsInput(float Input) {
	ClientFlapsInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		FlapsInput = Input;
	}
	return Input;
}

float UeasyFM::SetSpoilerInput(float Input) {
	ClientSpoilerInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		SpoilerInput = Input;
	}
	return Input;
}

float UeasyFM::SetVTOLInput(float Input) {
	ClientVTOLInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		VTOLInput = Input;
	}
	return Input;
}

float UeasyFM::SetReverserInput(float Input) {
	ClientReverserInput = Input;
	if (GetOwner()->Role == ROLE_Authority) {
		ReverserInput = Input;
	}
	return Input;
}


void UeasyFM::InputReplicate() {
	//SCOPE_CYCLE_COUNTER(STAT_InputBroadcast);

	if (InputReplicationFrequency <= 0.0f) return;

	if (TimeSinceInputUpdate >= 1.0f / InputReplicationFrequency) {

		if((ClientPitchInput!=PitchInput)||
		(ClientYawInput != YawInput) ||
		(ClientRollInput != RollInput) ||
		(ClientThrottleInput != ThrottleInput)){
			InputRep(ClientPitchInput, ClientYawInput, ClientRollInput, ClientThrottleInput);
			
			TimeSinceInputUpdate = 0.0f;
		}

		if ((ClientFlapsInput != FlapsInput) ||
		(ClientSpoilerInput != SpoilerInput) ||
		(ClientVTOLInput != VTOLInput) ||
		(ClientReverserInput != ReverserInput)) {
			SecInputRep(ClientFlapsInput, ClientSpoilerInput, ClientVTOLInput, ClientReverserInput);

			TimeSinceInputUpdate = 0.0f;
		}
	}
}

void UeasyFM::InputRep_Implementation(float Pitch, float Yaw, float Roll, float Throttle) {
	PitchInput = Pitch;
	YawInput = Yaw;
	RollInput = Roll;
	ThrottleInput = Throttle;
}

bool UeasyFM::InputRep_Validate(float Pitch, float Yaw, float Roll, float Throttle) {
	return true;
}

void UeasyFM::SecInputRep_Implementation(float Flaps, float Spoiler, float VTOL, float Reverser) {
	FlapsInput = Flaps;
	SpoilerInput = Spoiler;
	VTOLInput = VTOL;
	ReverserInput = Reverser;
}

bool UeasyFM::SecInputRep_Validate(float Flaps, float Spoiler, float VTOL, float Reverser) {
	return true;
}