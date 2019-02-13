// Copyright 2016-2017 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"

float UeasyFM::EnginePower(float TotalSpeed, float Mach, float AirDensity) const {
	float Thrust = 0.0f;

	if (EngineCurves) {
		Thrust = GetCurveValue(DensityPowerCurve, AirDensity)
		* GetCurveValue(SpeedPowerCurve, TotalSpeed*FMath::Sqrt(AirDensity / SeaLevelAirDensity))
		* GetCurveValue(MachPowerCurve, Mach);
	}
	else { Thrust = 1.0f; };

	//idle thrust
	float idle = FMath::Clamp(EngineIdlePower / EngineFullPower, 0.0f, 1.0f);
	Thrust = FMath::Lerp(idle * Thrust, Thrust, ThrottlePos);
	Thrust = FMath::Lerp(Thrust, -Thrust*ReverserThrustPortion, ReverserPos);

	return Thrust;
}