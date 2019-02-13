// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodelcomponent.h"

FVector UeasyFM::Damping(float pitchrate,float rollrate,float yawrate, float totalspeed, float air) const {
	FVector damptorque;
	damptorque.X = -rollrate*RollDamping;
	damptorque.Y = -pitchrate*PitchDamping;
	damptorque.Z = -yawrate*YawDamping;

	if (!LinearDamping) {
		return damptorque*(pow(totalspeed, 2.0f))*air;
	}
	else { return damptorque*totalspeed*air; };
}

FVector UeasyFM::Control(const FTransform &Transform, FVector velocity, float air) const {

	float pitchSensMultiplier = 1.0;
	float yawSensMultiplier = 1.0;
	float rollSensMultiplier = 1.0;

	float TotalSpeed = velocity.Size();

	//scale sensitivity
	if (ScaleSensitivityWithAlpha) {
		float alpha = GetAlpha(Transform, velocity);
		float slip = GetSlip(Transform, velocity);

		pitchSensMultiplier *= GetCurveValue(AlphaSensitivityCurve.Pitch, alpha);
		yawSensMultiplier *= GetCurveValue(AlphaSensitivityCurve.Yaw, alpha);
		rollSensMultiplier *= GetCurveValue(AlphaSensitivityCurve.Roll, alpha);

		pitchSensMultiplier *= GetCurveValue(SlipSensitivityCurve.Pitch, slip);
		yawSensMultiplier *= GetCurveValue(SlipSensitivityCurve.Yaw, slip);
		rollSensMultiplier *= GetCurveValue(SlipSensitivityCurve.Roll, slip);
	}

	FVector controltorque;
	controltorque.X = -RollPos*RollSensitivity*rollSensMultiplier;
	controltorque.Y = PitchPos*PitchSensitivity*pitchSensMultiplier;
	controltorque.Z = YawPos*YawSensitivity*yawSensMultiplier;

	if (!LinearControlSensitivity) {
		return controltorque*(pow(TotalSpeed, 2.0f))*air;
	}
	else { return controltorque*TotalSpeed*air; };
}

FVector UeasyFM::Stability(const FTransform &transform, FVector velocity, float air, float soundv) const {

	//scale stability
	float pitchStabMultiplier = 1.0;
	float yawStabMultiplier = 1.0;
	float rollStabMultiplier = 1.0;

	float TotalSpeed = velocity.Size();

	float alpha = GetAlpha(transform, velocity);
	float slip = GetSlip(transform, velocity);

	if (ScaleStabilityWithAlpha) {

		pitchStabMultiplier *= GetCurveValue(AlphaStabilityCurve.Pitch, alpha);
		yawStabMultiplier *= GetCurveValue(AlphaStabilityCurve.Yaw, alpha);
		rollStabMultiplier *= GetCurveValue(AlphaStabilityCurve.Roll, alpha);

		pitchStabMultiplier *= GetCurveValue(SlipStabilityCurve.Pitch, slip);
		yawStabMultiplier *= GetCurveValue(SlipStabilityCurve.Yaw, slip);
		rollStabMultiplier *= GetCurveValue(SlipStabilityCurve.Roll, slip);
	}

	//mach stability
	if (MachCurves) {
		float mach = TotalSpeed / soundv;

		float mscurveval = GetCurveValue(MachStabilityCurve, mach);
		pitchStabMultiplier *= mscurveval;
		yawStabMultiplier *= mscurveval;
		rollStabMultiplier *= mscurveval;
	}

	pitchStabMultiplier *= FMath::Lerp(1.0f, SpoilerStabilityMulti, SpoilerPos);
	yawStabMultiplier *= FMath::Lerp(1.0f, SpoilerStabilityMulti, SpoilerPos);
	rollStabMultiplier *= FMath::Lerp(1.0f, SpoilerStabilityMulti, SpoilerPos);

	float AlphaWFlaps = alpha + (FlapsAlpha*FlapsPos) + (SpoilerAlpha*SpoilerPos);

	FVector stabtorque;
	stabtorque.X = FMath::Sin((-slip)*PI / 180.0f)*RollStability*rollStabMultiplier;
	stabtorque.Y = FMath::Sin((AlphaWFlaps - FrameTrimPitch)*PI / 180.0f)*PitchStability*pitchStabMultiplier;
	stabtorque.Z = FMath::Sin((-slip - FrameTrimYaw)*PI / 180.0f)*YawStability*yawStabMultiplier;

	if (!LinearStability) {
		return stabtorque * FMath::Pow(TotalSpeed, 2.0f) * air;
	}
	else { return stabtorque * TotalSpeed * air; };

}

FVector UeasyFM::RCSTorque(float ThrustTemp) const {
	if (RCSEnabled) {

		float ThrustMultiplier;
		if (RCSScaleWithEnginePower) {
			ThrustMultiplier = ThrustTemp;
		}
		else { ThrustMultiplier = 1.0f; };

		FVector RCStorque;
		RCStorque.X = -RollPos*RCSRollSensitivity*ThrustMultiplier;
		RCStorque.Y = PitchPos*RCSPitchSensitivity*ThrustMultiplier;
		RCStorque.Z = YawPos*RCSYawSensitivity*ThrustMultiplier;
		return RCStorque;
	}
	else {
		return FVector(0, 0, 0);
	}
}