// Copyright 2016 Mookie. All Rights Reserved.

#include "easygearcomponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "DrawDebugHelpers.h"

//Actual simulation
void UEasyGear::Simulate(FBodyInstance* bodyInst, float DeltaTime, bool apply, bool debug) {
	FTransform transform;

	if (UseThisComponentTransform) {
		transform = this->GetComponentTransform();
	}
	else {
		transform = bodyInst->GetUnrealWorldTransform();
	};

	FVector location = transform.GetLocation();
	FVector downvec = -transform.GetUnitAxis(EAxis::Z);
	FVector fwdvec = transform.GetUnitAxis(EAxis::X);
	FVector rightvec = transform.GetUnitAxis(EAxis::Y);

	float multiplier = 1.0f;
	if (ScaleWithMass) {
		multiplier = bodyInst->GetBodyMass();
	}

	float LeftBrake, RightBrake;

	if (LocalInputPrediction&&GetOwnerRole() == ROLE_AutonomousProxy) {
		LeftBrake = ClientLeftBrakeInput;
		RightBrake = ClientRightBrakeInput;
	}else {
		LeftBrake = LeftBrakeInput;
		RightBrake = RightBrakeInput;
	}
	LeftBrake = FMath::Clamp(LeftBrake, 0.0f, 1.0f);
	RightBrake = FMath::Clamp(RightBrake, 0.0f, 1.0f);

	if (NoseSteering) {
		if (SteerToVector) {
			SteeringPosition = FVector::DotProduct(rightvec, GuidedVector)*SteerToVectorSensitivity;
			SteeringPosition = FMath::Clamp(SteeringPosition, -1.0f, 1.0f);
		}
		else {
			if (LocalInputPrediction&&GetOwnerRole() == ROLE_AutonomousProxy) {
				SteeringPosition = FMath::Clamp(ClientSteeringInput, -1.0f, 1.0f);
			}
			else {
				SteeringPosition = FMath::Clamp(SteeringInput, -1.0f, 1.0f);
			}
		}
	}
	else {
		SteeringPosition = 0;
	}

	//nose wheel
	if (NoseEnabled) {
		FVector offset = transform.GetRotation()*FVector(NoseLocationLong, 0, NoseLocationVert);
		NoseOnGround = SimulateWheel(bodyInst, DeltaTime, location + offset, downvec, fwdvec, rightvec, NoseStrutLength, NoseWheelRadius, NoseSpring, NoseShockAbsorber, NoseGrip, 0, 0, NoseGripLimit, SteeringPosition, multiplier, NoseStrutCompression, NoseSkid, NoseLocation, apply, debug);
	}

	//mains
	if (MainLeftEnabled) {
		FVector offset = transform.GetRotation()*FVector(MainLocationLong, -MainLocationLat, MainLocationVert);
		MainLeftOnGround = SimulateWheel(bodyInst, DeltaTime, location + offset, downvec, fwdvec, rightvec, MainStrutLength, MainWheelRadius, MainSpring, MainShockAbsorber, MainGrip, MainBrakePower, LeftBrake, MainGripLimit, 0, multiplier, MainLeftStrutCompression, MainLeftSkid, MainLeftLocation, apply, debug);
	}

	if (MainRightEnabled) {
		FVector offset = transform.GetRotation()*FVector(MainLocationLong, MainLocationLat, MainLocationVert);
		MainRightOnGround = SimulateWheel(bodyInst, DeltaTime, location + offset, downvec, fwdvec, rightvec, MainStrutLength, MainWheelRadius, MainSpring, MainShockAbsorber, MainGrip, MainBrakePower, RightBrake, MainGripLimit, 0, multiplier, MainRightStrutCompression, MainRightSkid, MainRightLocation, apply, debug);
	}
}

bool UEasyGear::SimulateWheel(FBodyInstance* bodyInst, float DeltaTime, FVector loc, FVector direction, FVector fwdvec, FVector rightvec,
	float length, float radius, float spring, float shock, float grip, float brakepower, float brakeinput, float maxgrip, float steering, float multiplier, float &compress, bool &skid, FVector &outLocation, bool apply, bool debug) {
	compress = 0.0f;
	FHitResult result;
	FCollisionObjectQueryParams objParams;

	for (TEnumAsByte<ECollisionChannel> col : CollisionChannels) {
		objParams.AddObjectTypesToQuery(col);
	}

	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this->GetOwner());

	if (UseMaterialFriction) {
		queryParams.bReturnPhysicalMaterial = true;
	}

	fwdvec = fwdvec.RotateAngleAxis(-steering*NoseSteeringMaxAngle, direction);
	rightvec = rightvec.RotateAngleAxis(-steering*NoseSteeringMaxAngle, direction);

	bool hit = GetWorld()->SweepSingleByObjectType(result, loc, loc + direction*length, FQuat(1, 0, 0, 1), objParams, FCollisionShape::MakeSphere(radius),queryParams);
	if (hit) {

		compress = 1.0f - result.Time;
		FVector hitloc = result.Location;
		FVector norm = result.ImpactNormal;
		FVector velocity = bodyInst->GetUnrealWorldVelocityAtPoint(hitloc);

		FBodyInstance *other = result.GetComponent()->GetBodyInstance();
		if (other != nullptr) {
			FVector otherVelocity = other->GetUnrealWorldVelocityAtPoint(hitloc);
			velocity -= otherVelocity;
		}

		FVector springForce = (norm*compress*spring);
		FVector shockForce = norm*-FMath::Min(FVector::DotProduct(velocity, norm), 0.0f)*shock;

		FVector sidevec = FVector::CrossProduct(norm, fwdvec);
		FVector brakevec = FVector::CrossProduct(norm, rightvec);

		float sidegrip = -FVector::DotProduct(sidevec, velocity)*grip;
		float brakegrip;

		float limit;
		if (ScaleGripLimitWithStrutCompression) {
			limit = maxgrip*compress;
		}
		else {
			limit = maxgrip;
		}

		if (UseMaterialFriction && result.PhysMaterial.IsValid()) {
			limit*=result.PhysMaterial.Get()->Friction;
		}
		
		if (MainBrakeAntiLock) {
			brakegrip= FMath::Clamp(-FVector::DotProduct(brakevec, velocity)*brakepower, -limit, limit)*brakeinput;
		}
		else {
			brakegrip = -FVector::DotProduct(brakevec, velocity)*brakepower*brakeinput;
		}

		FVector totalgripunclamped = ((sidevec*sidegrip) + (brakevec*brakegrip)).GetClampedToMaxSize(limit);
		FVector totalgrip = totalgripunclamped.GetClampedToMaxSize(limit);	

		skid = (totalgripunclamped.Size() > totalgrip.Size());

		if (apply) {
			bodyInst->AddForceAtPosition((springForce + shockForce + totalgrip)*multiplier, hitloc,false);
			if (other != nullptr) {
				other->AddForceAtPosition(-(springForce + shockForce + totalgrip)*multiplier, hitloc,false);
			}
		}

		outLocation = hitloc;
	}
	else {
		skid = false;
		outLocation = loc + direction*length;
	}

#ifdef WITH_EDITOR
	if (debug) {
		FVector lagcomp = FVector(0, 0, 0);
		if(DebugLagCompensation){
			lagcomp = bodyInst->GetUnrealWorldVelocityAtPoint(loc)*DeltaTime;
		}

		if (hit) {
			DrawDebugLine(GetWorld(), loc+lagcomp, loc + lagcomp + direction*length, FColor::Green);
			DrawDebugSphere(GetWorld(), lagcomp + result.Location, radius, 8, FColor::Green);
		}
		else {
			DrawDebugLine(GetWorld(), loc+lagcomp, loc +lagcomp + direction*length, FColor::Red);
			DrawDebugSphere(GetWorld(), loc + lagcomp + (direction*length), radius, 8, FColor::Red);
		}
	}

	return hit;
#endif
}