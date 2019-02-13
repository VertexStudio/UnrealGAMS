// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"

void UeasyFM::Extract(
	FBodyInstance* BodyInst,
	FTransform &Transform,
	FVector &rawVelocity,
	FVector &Velocity,
	FVector &AngularVelocity,
	FVector &fwdvec,
	FVector &upvec,
	FVector &rightvec,
	float &TotalSpeed,
	float &AirDensity,
	float &SpeedOfSound,
	float &Mach,
	float &pitchrate,
	float &yawrate,
	float &rollrate
	)const {

	if (UseThisComponentTransform) {
		Transform = this->GetComponentTransform();
	}
	else {
		Transform = BodyInst->GetUnrealWorldTransform();
	}

	rawVelocity = BodyInst->GetUnrealWorldVelocity();
	Velocity = (rawVelocity - GetWind(Transform.GetLocation()));
	AngularVelocity = BodyInst->GetUnrealWorldAngularVelocityInRadians() / PI*180.0f;

	AirDensity = GetAirDensity(Transform.GetLocation());
	SpeedOfSound = GetSpeedOfSound(Transform.GetLocation());

	TotalSpeed = Velocity.Size();

	Mach = TotalSpeed / SpeedOfSound;

	GetAxisVectors(Transform, fwdvec, upvec, rightvec);

	pitchrate = FVector::DotProduct(rightvec, AngularVelocity);
	rollrate = FVector::DotProduct(fwdvec, AngularVelocity);
	yawrate = FVector::DotProduct(upvec, AngularVelocity);
};

void UeasyFM::GetData(EGetDataFormat Units, bool UseGravity, FVector &Velocity, FVector &TrueVelocity, float &Altitude, float &IndicatedAirSpeed, float &TrueAirSpeed, float &GroundSpeed, float &Mach, 
	float &Alpha, float &Slip, float &PitchRate, float &YawRate, float &RollRate, float &AccelX, float &AccelY, float &AccelZ) const {
	UPrimitiveComponent *LocalParent = Cast<UPrimitiveComponent>(GetAttachParent());
	FBodyInstance *BodyInst = LocalParent->GetBodyInstance();

	if (BodyInst == nullptr) { return; }

	//multipliers
	float AltitudeMultiplier;
	float SpeedMultiplier;
	float AccelerationMultiplier;

	switch (Units) {
	case EGetDataFormat::GDF_UU:
		AltitudeMultiplier = 1.0f;
		SpeedMultiplier = 1.0f;
		AccelerationMultiplier = 1.0f;
		break;
	case EGetDataFormat::GDF_Metric:
		AltitudeMultiplier = 0.01f;
		SpeedMultiplier = 0.036f;
		AccelerationMultiplier = 1.0/980.0f;
		break;
	case EGetDataFormat::GDF_Imperial:
		AltitudeMultiplier = 0.0328084f;
		SpeedMultiplier = 0.0223694f;
		AccelerationMultiplier = 1.0/980.0f;
		break;
	case EGetDataFormat::GDF_Nautical:
		AltitudeMultiplier = 0.0328084f;
		SpeedMultiplier = 0.0194384f;
		AccelerationMultiplier = 1.0/980.0f;
		break;
	default:
		AltitudeMultiplier = 1.0f;
		SpeedMultiplier = 1.0f;
		AccelerationMultiplier = 1.0/980.0f;
	};

	AltitudeMultiplier /= WorldScale;
	SpeedMultiplier /= WorldScale;
	AccelerationMultiplier /= WorldScale;

	FTransform Transform;
	FVector AngularVelocity;
	FVector fwdvec, upvec, rightvec;
	float TotalSpeed;
	float AirDensity, SpeedOfSound;

	Extract(BodyInst, Transform, TrueVelocity, Velocity, AngularVelocity, fwdvec, upvec, rightvec, TotalSpeed, AirDensity, SpeedOfSound, Mach, PitchRate, YawRate, RollRate);

	float fwdspeed = FVector::DotProduct(Velocity, fwdvec);
	IndicatedAirSpeed = FMath::Sqrt((AirDensity / SeaLevelAirDensity)*FMath::Pow(fwdspeed, 2.0f))*SpeedMultiplier;
	TrueAirSpeed = fwdspeed*SpeedMultiplier;
	GroundSpeed = TrueVelocity.Size()*SpeedMultiplier;

	FVector DistanceFromOrigin = (Transform.GetLocation()-WorldCenterLocation + FVector(GetWorld()->OriginLocation));
	if(SphericalAltitude)
	{
		Altitude = (DistanceFromOrigin.Size() - SeaLevelRadius) * AltitudeMultiplier;
	}
	else {
		Altitude = DistanceFromOrigin.Z*AltitudeMultiplier;
	}

	FVector newAcceleration = acceleration;
	if (UseGravity) {
		newAcceleration -= FVector(0.0f, 0.0f, GetWorld()->GetGravityZ());
	};

	AccelX = FVector::DotProduct(newAcceleration, fwdvec)*AccelerationMultiplier;
	AccelY = FVector::DotProduct(newAcceleration, rightvec)*AccelerationMultiplier;
	AccelZ = FVector::DotProduct(newAcceleration, upvec)*AccelerationMultiplier;

	Alpha = GetAlpha(Transform, Velocity);
	Slip = GetSlip(Transform, Velocity);
}

void UeasyFM::GetAtmosphereData(FVector & OutWind, float & OutDensity, float & OutPressure, float & OutTemperature) const
{
	UPrimitiveComponent *LocalParent = Cast<UPrimitiveComponent>(GetAttachParent());
	FBodyInstance *BodyInst = LocalParent->GetBodyInstance();

	if (BodyInst == nullptr) { return; }
	FTransform Transform;


	if (UseThisComponentTransform) {
		Transform = this->GetComponentTransform();
	}
	else {
		Transform = BodyInst->GetUnrealWorldTransform();
	}
	FVector Location = Transform.GetLocation();
	OutWind = GetWind(Location);
	float AltitudeM;

	FVector DistanceFromOrigin = (Transform.GetLocation() - WorldCenterLocation + FVector(GetWorld()->OriginLocation));
	if (SphericalAltitude)
	{
		AltitudeM = (DistanceFromOrigin.Size() - SeaLevelRadius) / 100.0f;
	}
	else {
		AltitudeM = DistanceFromOrigin.Z / 100.0f;
	}


	OutDensity = GetAltitudeDensity(AltitudeM);
	OutPressure = GetAltitudePressure(AltitudeM);
	OutTemperature = GetAltitudeTemperature(AltitudeM);
}

/*inline*/ float UeasyFM::GetAlpha(const FTransform &Transform, FVector velocity) const {
	FVector fwdvec, upvec, rightvec;
	GetAxisVectors(Transform, fwdvec, upvec, rightvec);

	float fwdspeed = FVector::DotProduct(velocity, fwdvec);
	float upspeed = FVector::DotProduct(velocity, upvec);

	return -FMath::Atan2(upspeed*FMath::Lerp(1.0f, GroundEffectAlphaMultiplier, GroundEffect), fwdspeed) * 180.0f / PI;
};

/*inline*/ float UeasyFM::GetSlip(const FTransform &Transform, FVector velocity) const {
	FVector fwdvec, upvec, rightvec;
	GetAxisVectors(Transform, fwdvec, upvec, rightvec);

	float fwdspeed = FVector::DotProduct(velocity, fwdvec);
	float rightspeed = FVector::DotProduct(velocity, rightvec);

	return -FMath::Atan2(rightspeed, fwdspeed) * 180.0f / PI;
};

/*inline*/ void UeasyFM::GetAxisVectors(const FTransform &Transform, FVector &fwdvec, FVector &upvec, FVector &rightvec) const {
	fwdvec = Transform.GetUnitAxis(EAxis::X);
	upvec = Transform.GetUnitAxis(EAxis::Z);
	rightvec = Transform.GetUnitAxis(EAxis::Y);
}