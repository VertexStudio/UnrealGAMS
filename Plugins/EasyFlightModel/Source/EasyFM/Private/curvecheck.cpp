// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"

/*inline*/ float UeasyFM::GetCurveValue(const UCurveFloat* curve, float in, float deflt) const {
	if (curve == nullptr) return deflt;
	return curve->GetFloatValue(in);
};