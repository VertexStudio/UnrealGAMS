// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"

/*inline*/ float UeasyFM::BlendInput(float low, float high) const {
	return FMath::Lerp(FMath::Clamp(low, -1.0f, 1.0f), FMath::Sign(high), FMath::Min(FMath::Abs(high), 1.0f));
};

/*inline*/ float UeasyFM::Limiter(float input, float limitedVar, float min, float max, float sensitivity) const {
	return FMath::Clamp(input + (limitedVar - FMath::Clamp(limitedVar, min, max))*sensitivity, -1.0f, 1.0f);
};

/*inline*/ float UeasyFM::Transit(float input, float target, float upspeed, float downspeed, float DeltaTime) const {
	float shift = FMath::Clamp(target - input, -1.0f / downspeed*DeltaTime, 1.0f / upspeed*DeltaTime);
	return input + shift;
};
