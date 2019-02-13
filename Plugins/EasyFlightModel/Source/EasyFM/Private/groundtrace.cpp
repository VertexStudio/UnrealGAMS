// Copyright 2016 Mookie. All Rights Reserved.

#include "flightmodel.h"
#include "flightmodelcomponent.h"

float UeasyFM::GetGroundEffect(const FTransform &transform) {
	FVector upvec = transform.GetUnitAxis(EAxis::Z);
	return FMath::Pow(GroundTrace(transform.GetLocation(), -upvec, GroundTraceLength), GroundEffectFalloffExponent);
};

float UeasyFM::GroundTrace(FVector start, FVector direction, float length)
{
	FHitResult result;
	FCollisionObjectQueryParams params;

	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this->GetOwner());

	for (TEnumAsByte<ECollisionChannel> col : GroundTraceChannels) {
		params.AddObjectTypesToQuery(col);
	};

	bool hit = GetWorld()->LineTraceSingleByObjectType(result, start, start + (direction*length),params,queryParams);
	if (hit) {
		return 1.0f - result.Time;
	}
	else {
		return 0.0f;
	};
};