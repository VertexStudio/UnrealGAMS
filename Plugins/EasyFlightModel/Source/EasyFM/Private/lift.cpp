// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"

float UeasyFM::CalculateLift(float alpha, float speed, float mach) const {
	float lift = 0;
	if (LiftMultiplier != 0) {
		lift = LiftMultiplier*GetCurveValue(LiftCurve,alpha,0);
	};

	if (ScaleFlapsWithAlpha) {
		lift += (FlapsLift*FlapsPos*GetCurveValue(FlapsAlphaCurve,alpha));
	}
	else {
		lift += (FlapsLift*FlapsPos);
	};

	lift *= FMath::Lerp(1.0f, SpoilerLiftMulti, SpoilerPos);
	lift -= SpoilerDownForce*SpoilerPos;

	if (MachCurves) {
		lift *= GetCurveValue(MachLiftCurve,mach);
	};

	lift *= speed;

	lift *= FMath::Lerp(1.0f,GroundEffectLiftMultiplier,GroundEffect);

	return lift;
};

float UeasyFM::CalculateSideLift(float slip, float speed, float mach) const {
	float sidelift = 0;
	if (SideLiftMultiplier != 0) {
		sidelift = SideLiftMultiplier*GetCurveValue(SideLiftCurve,slip,0);
	};

	if (MachCurves) {
		sidelift *= GetCurveValue(MachLiftCurve, mach);
	};

	sidelift *= speed;

	return sidelift;
};

float UeasyFM::CalculateDrag(float alpha, float slip, float speed, float mach) const
{
	float drag = 0;
	if (DragMultiplier != 0) {
		drag = DragMultiplier*GetCurveValue(DragCurve,alpha,0);
	};
	if (SideDragMultiplier != 0) {
		drag += SideDragMultiplier*GetCurveValue(SideDragCurve,slip,0);
	};

	drag += (FlapsDrag*FlapsPos*GetCurveValue(FlapsAlphaCurve, alpha));
	drag += (SpoilerDrag*SpoilerPos);
	drag += AdditionalDrag;

	if (MachCurves) {
		drag *= GetCurveValue(MachDragCurve, mach);
	};

	drag *= speed;

	drag *= FMath::Lerp(1.0f, GroundEffectDragMultiplier, GroundEffect);

	return drag;
};

FVector UeasyFM::AeroForces(const FTransform &transform, FVector velocity, float soundv) const {
	float lift, sidelift, drag;

	FVector fwdvec, upvec, rightvec;
	GetAxisVectors(transform, fwdvec, upvec, rightvec);

	float totalspeed = velocity.Size();
	float fwdspeed = FVector::DotProduct(velocity, fwdvec);
	float upspeed = FVector::DotProduct(velocity, upvec);
	float rightspeed = FVector::DotProduct(velocity, rightvec);

	float mach = totalspeed / soundv;

	float alpha = GetAlpha(transform, velocity);
	float slip = GetSlip(transform, velocity);

	float streamspeed;
	float sidestreamspeed;
	float totalstreamspeed;

	if (LinearLift) {
		streamspeed = FMath::Abs(upspeed) + FMath::Abs(fwdspeed);
		sidestreamspeed = FMath::Abs(rightspeed) + FMath::Abs(fwdspeed);
		totalstreamspeed = FMath::Abs(totalspeed);
	}
	else {
		streamspeed = FMath::Pow(upspeed, 2.0f) + FMath::Pow(fwdspeed, 2.0f);
		sidestreamspeed = FMath::Pow(rightspeed, 2.0f) + FMath::Pow(fwdspeed, 2.0f);
		totalstreamspeed = FMath::Pow(totalspeed, 2.0f);
	};

	float AlphaWFlaps = alpha + (FlapsAlpha*FlapsPos) + (SpoilerAlpha*SpoilerPos);

	lift = CalculateLift(AlphaWFlaps, streamspeed, mach);
	sidelift = CalculateSideLift(slip, sidestreamspeed, mach);
	drag = CalculateDrag(AlphaWFlaps, slip, totalstreamspeed, mach);

	return FVector(lift, sidelift, drag);
};