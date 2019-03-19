#include "AerodynamicsComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

/////////////////////////////////////////////////////////
// blends
/////////////////////////////////////////////////////////

float UAerodynamicsComponent::BlendInput(float low, float high) const
{
    return FMath::Lerp(FMath::Clamp(low, -1.0f, 1.0f), FMath::Sign(high), FMath::Min(FMath::Abs(high), 1.0f));
};

float UAerodynamicsComponent::Limiter(float input, float limitedVar, float min, float max, float sensitivity) const
{
    return FMath::Clamp(input + (limitedVar - FMath::Clamp(limitedVar, min, max)) * sensitivity, -1.0f, 1.0f);
};

float UAerodynamicsComponent::Transit(float input, float target, float upspeed, float downspeed, float DeltaTime) const
{
    float shift = FMath::Clamp(target - input, -1.0f / downspeed * DeltaTime, 1.0f / upspeed * DeltaTime);
    return input + shift;
};

/////////////////////////////////////////////////////////
// control
/////////////////////////////////////////////////////////

void UAerodynamicsComponent::SimulateControls(float DeltaTime, float alpha, float slip, float pitchrate, float yawrate, float rollrate, float speed, const FTransform &Transform, FVector location, FVector rawVelocity, float air)
{
    speed = FMath::Sqrt(FMath::Pow(speed, 2.0f) * air);

    FVector fwdvec, upvec, rightvec;
    GetAxisVectors(Transform, fwdvec, upvec, rightvec);

    float Pitch, Yaw, Roll, Flaps, Spoiler, Throttle, Vtol, Reverser;

    if (LocalInputPrediction && GetOwnerRole() == ROLE_AutonomousProxy)
    {
        Pitch = ClientPitchInput;
        Yaw = ClientYawInput;
        Roll = ClientRollInput;
        Flaps = ClientFlapsInput;
        Spoiler = ClientSpoilerInput;
        Throttle = ClientThrottleInput;
        Vtol = ClientVTOLInput;
        Reverser = ClientReverserInput;
    }
    else
    {
        Pitch = PitchInput;
        Yaw = YawInput;
        Roll = RollInput;
        Flaps = FlapsInput;
        Spoiler = SpoilerInput;
        Throttle = ThrottleInput;
        Vtol = VTOLInput;
        Reverser = ReverserInput;
    };

    Pitch = FMath::Clamp(Pitch, -1.0f, 1.0f);
    Yaw = FMath::Clamp(Yaw, -1.0f, 1.0f);
    Roll = FMath::Clamp(Roll, -1.0f, 1.0f);
    Flaps = FMath::Clamp(Flaps, 0.0f, 1.0f);
    Spoiler = FMath::Clamp(Spoiler, 0.0f, 1.0f);
    Throttle = FMath::Clamp(Throttle, -1.0f, 1.0f);
    Vtol = FMath::Clamp(Vtol, -1.0f, 1.0f);
    Reverser = FMath::Clamp(Reverser, -1.0f, 1.0f);

    float augScale;
    if (DampingScaleWithSpeed)
    {
        augScale = 1.0f - FMath::Clamp(FMath::Pow(speed, 2.0f) / FMath::Pow(DampingShutoffSpeed, 2.0f), 0.0f, 1.0f);
    }
    else
    {
        augScale = 1.0f;
    }

    //guidance
    if (Guided)
    {

        FVector fwdvecComp;
        FVector upvecComp;
        FVector rightvecComp;
        FVector targetVec;

        if (GuidedToLocation)
        {
            targetVec = (GuidedVector - location).GetSafeNormal();
        }
        else
        {
            targetVec = GuidedVector.GetSafeNormal();
        }

        if (GuidedCompensateAlpha)
        {
            fwdvecComp = rawVelocity.GetSafeNormal();
            upvecComp = fwdvecComp.RotateAngleAxis(-90.0f, rightvec);
            rightvecComp = fwdvecComp.RotateAngleAxis(90.0f, upvec);
        }
        else
        {
            fwdvecComp = fwdvec;
            upvecComp = upvec;
            rightvecComp = rightvec;
        }

        float guideFwd = FVector::DotProduct(fwdvecComp, targetVec);
        float guideUp = -FVector::DotProduct(upvecComp, targetVec);
        float guideRight = FVector::DotProduct(rightvecComp, targetVec);

        if (GuidedPitchFullRange)
        {
            guideUp = -FMath::Atan2(-guideUp, guideFwd);
        }

        if (GuidedYawFullRange)
        {
            guideRight = FMath::Atan2(guideRight, guideFwd);
        }

        PitchTrimPos = FMath::Clamp(PitchTrimPos + (GuidedPID.PitchI * guideUp), -1.0f, 1.0f);
        YawTrimPos = FMath::Clamp(YawTrimPos + (GuidedPID.YawI * guideRight), -1.0f, 1.0f);
        RollTrimPos = FMath::Clamp(RollTrimPos + (GuidedPID.RollI * guideRight), -1.0f, 1.0f);

        float pitchGuide = (guideUp * GuidedPID.PitchP) + PitchTrimPos - (pitchrate * GuidedPID.PitchD * augScale);
        float yawGuide = (guideRight * GuidedPID.YawP) + YawTrimPos - (yawrate * GuidedPID.YawD * augScale);
        float rollGuide = (guideRight * GuidedPID.RollP) + RollTrimPos + (rollrate * GuidedPID.RollD * augScale);

        float autoLevel = FVector::DotProduct(rightvecComp, FVector(0, 0, 1)) * (1.0f - FMath::Clamp(FMath::Abs(pitchGuide), 0.0f, 1.0f));

        Pitch = BlendInput(pitchGuide, Pitch);
        Yaw = BlendInput(yawGuide, Yaw);
        Roll = BlendInput(rollGuide + autoLevel * GuidedAutoLevel, Roll);
    }
    else
    {
        float dampedPitch = 0;
        float dampedYaw = 0;
        float dampedRoll = 0;

        if (StabilityAugmentation)
        {
            dampedPitch = -pitchrate * PitchAug * augScale;
            dampedYaw = -yawrate * YawAug * augScale;
            dampedRoll = rollrate * RollAug * augScale;
        }

        if (AutoTrim)
        {
            if (!InputPauseAutoTrim || FMath::Abs(Pitch) <= PauseAutoTrimThreshold)
            {
                if (TrimPitchTo1G)
                {
                    FVector newAcceleration = acceleration - FVector(0.0f, 0.0f, GetWorld()->GetGravityZ());
                    float AccelZ = FVector::DotProduct(newAcceleration, upvec);
                    PitchTrimPos = FMath::Clamp(PitchTrimPos + ((AccelZ + GetWorld()->GetGravityZ()) * DeltaTime * AutoTrimPitchSensitivity * augScale), -1.0f, 1.0f);
                }
                else
                {
                    PitchTrimPos = FMath::Clamp(PitchTrimPos - (pitchrate * DeltaTime * AutoTrimPitchSensitivity * augScale), -1.0f, 1.0f);
                }
            }

            if (!InputPauseAutoTrim || FMath::Abs(Yaw) <= PauseAutoTrimThreshold)
            {
                if (TrimYawToCenter)
                {
                    YawTrimPos = FMath::Clamp(YawTrimPos - ((slip)*DeltaTime * AutoTrimYawSensitivity * augScale), -1.0f, 1.0f);
                }
                else
                {
                    YawTrimPos = FMath::Clamp(YawTrimPos + (-yawrate * DeltaTime * AutoTrimYawSensitivity * augScale), -1.0f, 1.0f);
                }
            }

            if (!InputPauseAutoTrim || FMath::Abs(Roll) <= PauseAutoTrimThreshold)
            {
                RollTrimPos = FMath::Clamp(RollTrimPos + (rollrate * DeltaTime * AutoTrimRollSensitivity * augScale), -1.0f, 1.0f);
            }
        }

        float yawCenter = YawCentering * -slip;

        Pitch = BlendInput(PitchTrimPos + dampedPitch, Pitch);
        Yaw = BlendInput(YawTrimPos + dampedYaw + yawCenter, Yaw);
        Roll = BlendInput(RollTrimPos + dampedRoll, Roll);
    }

    if (AlphaLimiter)
    {
        Pitch = Limiter(Pitch, alpha - (pitchrate * PitchRateInfluence * augScale), MinAlpha, MaxAlpha, AlphaLimiterSensitivity);
    }

    if (SlipLimiter)
    {
        Yaw = Limiter(Yaw, -slip - (yawrate * YawRateInfluence * augScale), -MaxSlip, MaxSlip, SlipLimiterSensitivity);
    }

    if (AutoThrottle)
    {
        if (!InputPauseAutoThrottle || FMath::Abs(Throttle) <= PauseAutoThrottleThreshold)
        {
            if (AutoThrottleHover)
            {
                float vspeed = rawVelocity.Z;
                float acc = acceleration.Z;
                AutoThrottlePos = FMath::Clamp(AutoThrottlePos - (DeltaTime * (AutoThrottleSensitivity * vspeed + HoverAccelerationInfluence * acc)), -1.0f, 1.0f);
            }
            else
            {
                float diff = speed - previousAirspeed;
                previousAirspeed = speed;
                AutoThrottlePos = FMath::Clamp(AutoThrottlePos - (AutoThrottleSensitivity * diff), -1.0f, 1.0f);
            };
        };
        if (!AutoSpoiler)
        {
            AutoThrottlePos = FMath::Max(AutoThrottlePos, 0.0f);
        }
        else
        {
            Spoiler = BlendInput(0.0f - AutoThrottlePos, -Throttle);
        }
        Throttle = BlendInput(AutoThrottlePos, Throttle);
    }

    if (AutoFlaps)
    {
        float range = FullExtendSpeed - FullRetractSpeed;
        Flaps = (speed - FullRetractSpeed) / range;
    }

    if (ScaleSensitivityWithSpeed)
    {
        Pitch *= GetCurveValue(SpeedSensitivityCurve.Pitch, speed);
        Yaw *= GetCurveValue(SpeedSensitivityCurve.Yaw, speed);
        Roll *= GetCurveValue(SpeedSensitivityCurve.Roll, speed);
    }

    //VTOL
    VTOLPos = FMath::Clamp(Transit(VTOLPos, Vtol, VTOLExtensionTime, VTOLRetractionTime, DeltaTime), 0.0f, 1.0f);
    PitchPos = FMath::Clamp(Transit(PitchPos, Pitch, PitchMoveTime, PitchMoveTime, DeltaTime), -1.0f, 1.0f);
    YawPos = FMath::Clamp(Transit(YawPos, Yaw, YawMoveTime, YawMoveTime, DeltaTime), -1.0f, 1.0f);
    RollPos = FMath::Clamp(Transit(RollPos, Roll, RollMoveTime, RollMoveTime, DeltaTime), -1.0f, 1.0f);
    FlapsPos = FMath::Clamp(Transit(FlapsPos, Flaps, FlapsExtensionTime, FlapsRetractionTime, DeltaTime), 0.0f, 1.0f);
    SpoilerPos = FMath::Clamp(Transit(SpoilerPos, Spoiler, SpoilerExtensionTime, SpoilerRetractionTime, DeltaTime), 0.0f, 1.0f);
    ReverserPos = FMath::Clamp(Transit(ReverserPos, Reverser, ReverserExtensionTime, ReverserRetractionTime, DeltaTime), 0.0f, 1.0f);
    ThrottlePos = FMath::Clamp(Transit(ThrottlePos, Throttle, SpoolUpTime, SpoolDownTime, DeltaTime), 0.0f, 1.0f);
}

/////////////////////////////////////////////////////////
// curve check
/////////////////////////////////////////////////////////

float UAerodynamicsComponent::GetCurveValue(const UCurveFloat *curve, float in, float deflt) const
{
    if (curve == nullptr)
        return deflt;
    return curve->GetFloatValue(in);
};

/////////////////////////////////////////////////////////
// engine
/////////////////////////////////////////////////////////

float UAerodynamicsComponent::EnginePower(float TotalSpeed, float Mach, float AirDensity) const
{
    float Thrust = 0.0f;

    if (EngineCurves)
    {
        Thrust = GetCurveValue(DensityPowerCurve, AirDensity) * GetCurveValue(SpeedPowerCurve, TotalSpeed * FMath::Sqrt(AirDensity / SeaLevelAirDensity)) * GetCurveValue(MachPowerCurve, Mach);
    }
    else
    {
        Thrust = 1.0f;
    };

    //idle thrust
    float idle = FMath::Clamp(EngineIdlePower / EngineFullPower, 0.0f, 1.0f);
    Thrust = FMath::Lerp(idle * Thrust, Thrust, ThrottlePos);
    Thrust = FMath::Lerp(Thrust, -Thrust * ReverserThrustPortion, ReverserPos);

    return Thrust;
}

/////////////////////////////////////////////////////////
// environment
/////////////////////////////////////////////////////////

FVector UAerodynamicsComponent::GetWind_Implementation(FVector Location) const
{
    return Wind;
}

float UAerodynamicsComponent::GetAirDensity_Implementation(FVector Location) const
{
    float AltitudeM;

    FVector DistanceFromOrigin = (Location - WorldCenterLocation + FVector(GetWorld()->OriginLocation));
    if (SphericalAltitude)
    {
        AltitudeM = (DistanceFromOrigin.Size() - SeaLevelRadius);
    }
    else
    {
        AltitudeM = DistanceFromOrigin.Z;
    }

    switch (AtmosphereType)
    {
    case (EAtmosphereType::AT_Curve):
    {
        float airmp = SeaLevelAirDensity / GetCurveValue(AirDensityCurve, 0, SeaLevelAirDensity);
        return GetCurveValue(AirDensityCurve, AltitudeM / WorldScale, SeaLevelAirDensity) * airmp;
    }
    case (EAtmosphereType::AT_Earth):
    {
        return GetAltitudeDensity(AltitudeM / WorldScale / 100.0f);
    }
    default:
    {
        return SeaLevelAirDensity;
    }
    }
}

float UAerodynamicsComponent::GetSpeedOfSound_Implementation(FVector Location) const
{
    if (SpeedOfSoundVariesWithAltitude)
    {

        float AltitudeM;

        FVector DistanceFromOrigin = (Location - WorldCenterLocation + FVector(GetWorld()->OriginLocation));
        if (SphericalAltitude)
        {
            AltitudeM = (DistanceFromOrigin.Size() - SeaLevelRadius);
        }
        else
        {
            AltitudeM = DistanceFromOrigin.Z;
        }

        float soundvmp = SeaLevelSpeedOfSound / GetCurveValue(SpeedOfSoundCurve, 0, SeaLevelSpeedOfSound);

        return SpeedOfSoundCurve->GetFloatValue((AltitudeM) / WorldScale) * WorldScale * soundvmp;
    }
    else
    {
        return SeaLevelSpeedOfSound * WorldScale;
    }
}

float UAerodynamicsComponent::GetAltitudePressure(float AltitudeMeter) const
{
    return FMath::Max(SeaLevelAirPressure * FMath::Pow((1 - (0.0000225577 * AltitudeMeter)), 5.25588), 0.0f);
}

float UAerodynamicsComponent::GetAltitudeTemperature(float AltitudeMeter) const
{
    return SeaLevelAirTemperature - (TemperatureLapseRate * FMath::Min(AltitudeMeter, TropopauseAltitude));
}

float UAerodynamicsComponent::GetAltitudeDensity(float AltitudeMeter) const
{
    float Temperature = GetAltitudeTemperature(AltitudeMeter);
    float Pressure = GetAltitudePressure(AltitudeMeter);
    return Pressure * 100.0f / ((Temperature + 273.15) * SpecificGasConstant);
}

/////////////////////////////////////////////////////////
// input replication
/////////////////////////////////////////////////////////

float UAerodynamicsComponent::SetPitchInput(float Input)
{
    ClientPitchInput = Input;
    if (GetOwner()->Role == ROLE_Authority)
    {
        PitchInput = Input;
    }
    return Input;
}

float UAerodynamicsComponent::SetYawInput(float Input)
{
    ClientYawInput = Input;
    if (GetOwner()->Role == ROLE_Authority)
    {
        YawInput = Input;
    }
    return Input;
}

float UAerodynamicsComponent::SetRollInput(float Input)
{
    ClientRollInput = Input;
    if (GetOwner()->Role == ROLE_Authority)
    {
        RollInput = Input;
    }
    return Input;
}

float UAerodynamicsComponent::SetThrottleInput(float Input)
{
    ClientThrottleInput = Input;
    if (GetOwner()->Role == ROLE_Authority)
    {
        ThrottleInput = Input;
    }
    return Input;
}

float UAerodynamicsComponent::SetFlapsInput(float Input)
{
    ClientFlapsInput = Input;
    if (GetOwner()->Role == ROLE_Authority)
    {
        FlapsInput = Input;
    }
    return Input;
}

float UAerodynamicsComponent::SetSpoilerInput(float Input)
{
    ClientSpoilerInput = Input;
    if (GetOwner()->Role == ROLE_Authority)
    {
        SpoilerInput = Input;
    }
    return Input;
}

float UAerodynamicsComponent::SetVTOLInput(float Input)
{
    ClientVTOLInput = Input;
    if (GetOwner()->Role == ROLE_Authority)
    {
        VTOLInput = Input;
    }
    return Input;
}

float UAerodynamicsComponent::SetReverserInput(float Input)
{
    ClientReverserInput = Input;
    if (GetOwner()->Role == ROLE_Authority)
    {
        ReverserInput = Input;
    }
    return Input;
}

void UAerodynamicsComponent::InputReplicate()
{
    //SCOPE_CYCLE_COUNTER(STAT_InputBroadcast);

    if (InputReplicationFrequency <= 0.0f)
        return;

    if (TimeSinceInputUpdate >= 1.0f / InputReplicationFrequency)
    {

        if ((ClientPitchInput != PitchInput) ||
            (ClientYawInput != YawInput) ||
            (ClientRollInput != RollInput) ||
            (ClientThrottleInput != ThrottleInput))
        {
            InputRep(ClientPitchInput, ClientYawInput, ClientRollInput, ClientThrottleInput);

            TimeSinceInputUpdate = 0.0f;
        }

        if ((ClientFlapsInput != FlapsInput) ||
            (ClientSpoilerInput != SpoilerInput) ||
            (ClientVTOLInput != VTOLInput) ||
            (ClientReverserInput != ReverserInput))
        {
            SecInputRep(ClientFlapsInput, ClientSpoilerInput, ClientVTOLInput, ClientReverserInput);

            TimeSinceInputUpdate = 0.0f;
        }
    }
}

void UAerodynamicsComponent::InputRep_Implementation(float Pitch, float Yaw, float Roll, float Throttle)
{
    PitchInput = Pitch;
    YawInput = Yaw;
    RollInput = Roll;
    ThrottleInput = Throttle;
}

bool UAerodynamicsComponent::InputRep_Validate(float Pitch, float Yaw, float Roll, float Throttle)
{
    return true;
}

void UAerodynamicsComponent::SecInputRep_Implementation(float Flaps, float Spoiler, float VTOL, float Reverser)
{
    FlapsInput = Flaps;
    SpoilerInput = Spoiler;
    VTOLInput = VTOL;
    ReverserInput = Reverser;
}

bool UAerodynamicsComponent::SecInputRep_Validate(float Flaps, float Spoiler, float VTOL, float Reverser)
{
    return true;
}

/////////////////////////////////////////////////////////
// replication
/////////////////////////////////////////////////////////

DECLARE_CYCLE_STAT(TEXT("Aerodynamics Net Move Broadcast"), STAT_MoveBroadcast, STATGROUP_Aerodynamics);
DECLARE_CYCLE_STAT(TEXT("Aerodynamics Net Move Receive"), STAT_MoveReceive, STATGROUP_Aerodynamics);
DECLARE_CYCLE_STAT(TEXT("Aerodynamics Net Move Smoothing"), STAT_MoveSmoothing, STATGROUP_Aerodynamics);
DECLARE_CYCLE_STAT(TEXT("Aerodynamics Net GetReplicatedProps"), STAT_GetProps, STATGROUP_Aerodynamics);
DECLARE_CYCLE_STAT(TEXT("Aerodynamics Net PreReplication"), STAT_PreRep, STATGROUP_Aerodynamics);

void UAerodynamicsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    SCOPE_CYCLE_COUNTER(STAT_GetProps);

    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UAerodynamicsComponent, Wind);

    DOREPLIFETIME(UAerodynamicsComponent, AdditionalDrag);
    DOREPLIFETIME(UAerodynamicsComponent, Buoyancy);

    DOREPLIFETIME(UAerodynamicsComponent, AlphaLimiter);
    DOREPLIFETIME(UAerodynamicsComponent, SlipLimiter);
    DOREPLIFETIME(UAerodynamicsComponent, StabilityAugmentation);
    DOREPLIFETIME(UAerodynamicsComponent, AutoTrim);
    DOREPLIFETIME(UAerodynamicsComponent, AutoThrottle);
    DOREPLIFETIME(UAerodynamicsComponent, AutoFlaps);

    DOREPLIFETIME(UAerodynamicsComponent, Guided);

    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, ThrottleInput, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, PitchInput, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, RollInput, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, YawInput, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, FlapsInput, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, SpoilerInput, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, VTOLInput, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, ReverserInput, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, GuidedVector, COND_Custom);

    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, PitchTrimPos, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, YawTrimPos, COND_Custom);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, RollTrimPos, COND_Custom);

    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, GotServerPosition, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, ServerLocation, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, ServerRotation, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, ServerLinearVelocity, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, ServerAngularVelocity, COND_InitialOnly);

    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, PitchPos, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, YawPos, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, RollPos, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, ThrottlePos, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, VTOLPos, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, FlapsPos, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, SpoilerPos, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(UAerodynamicsComponent, AutoThrottlePos, COND_InitialOnly);
}

void UAerodynamicsComponent::PreReplication(IRepChangedPropertyTracker &ChangedPropertyTracker)
{
    SCOPE_CYCLE_COUNTER(STAT_PreRep);

    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, ThrottleInput, MulticastInputs);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, PitchInput, MulticastInputs);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, RollInput, MulticastInputs);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, YawInput, MulticastInputs);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, FlapsInput, MulticastInputs);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, SpoilerInput, MulticastInputs);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, VTOLInput, MulticastInputs);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, ReverserInput, MulticastInputs);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, GuidedVector, MulticastInputs);

    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, PitchTrimPos, MulticastTrim);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, YawTrimPos, MulticastTrim);
    DOREPLIFETIME_ACTIVE_OVERRIDE(UAerodynamicsComponent, RollTrimPos, MulticastTrim);
}

void UAerodynamicsComponent::MovementBroadcast(FBodyInstance *BodyInst)
{
    SCOPE_CYCLE_COUNTER(STAT_MoveBroadcast);

    if (MovementReplicationFrequency <= 0.0f)
        return;

    if (TimeSincePosUpdate >= 1.0f / MovementReplicationFrequency)
    {
        float overshoot = TimeSincePosUpdate - (1.0f / MovementReplicationFrequency);
        FTransform Transform = BodyInst->GetUnrealWorldTransform();

        FVector LocalLoc = Transform.GetLocation();
        FRotator LocalRot = FRotator(Transform.GetRotation());
        FVector LocalVel = BodyInst->GetUnrealWorldVelocity();
        FVector LocalAngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();

        //rebase
        LocalLoc = UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(), LocalLoc);

        if (HighPrecision)
        {
            PositionRep(LocalLoc + LocalVel * overshoot, LocalRot, LocalVel, LocalAngVel);
        }
        else
        {
            PositionRepLow(LocalLoc + LocalVel * overshoot, LocalRot, LocalVel, LocalAngVel);
        }

        TimeSincePosUpdate = FMath::Fmod(TimeSincePosUpdate, 1.0f / MovementReplicationFrequency);
    }
}

void UAerodynamicsComponent::PositionRep_Implementation(FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity)
{
    //do not receive updates if hosting, or parent not valid
    SCOPE_CYCLE_COUNTER(STAT_MoveReceive);

    UPrimitiveComponent *Parent = Cast<UPrimitiveComponent>(GetAttachParent());

    if ((GetOwner()->Role != ROLE_Authority) && (Parent != nullptr))
    {
        FName ParentSocket = GetAttachSocketName();
        FBodyInstance *BodyInst = Parent->GetBodyInstance(ParentSocket);
        if (!BodyInst)
        {
            return;
        }

        //rebase
        NewLocation = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);

        //latency

        NewLocation += NewVelocity * AdditionalLatency;
        FQuat LagCompDelta = FQuat(NewAngularVelocity, AdditionalLatency * PI / 180.0f);
        NewRotation = (LagCompDelta * NewRotation.Quaternion()).Rotator();

        SoftSync(BodyInst, NewLocation, NewRotation, NewVelocity, NewAngularVelocity);
    }
}

void UAerodynamicsComponent::PositionRepLow_Implementation(FVector_NetQuantize10 NewLocation, FRotator NewRotation, FVector_NetQuantize NewVelocity, FVector_NetQuantize10 NewAngularVelocity)
{
    SCOPE_CYCLE_COUNTER(STAT_MoveReceive);

    UPrimitiveComponent *Parent = Cast<UPrimitiveComponent>(GetAttachParent());

    //do not receive updates if hosting, or parent not valid
    if ((GetOwner()->Role != ROLE_Authority) && (Parent != nullptr))
    {

        FName ParentSocket = GetAttachSocketName();
        FBodyInstance *BodyInst = Parent->GetBodyInstance(ParentSocket);
        if (!BodyInst)
        {
            return;
        }

        //rebase
        NewLocation = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);

        //latency
        NewLocation += NewVelocity * AdditionalLatency;
        FQuat LagCompDelta = FQuat(NewAngularVelocity, AdditionalLatency * PI / 180.0f);
        NewRotation = (LagCompDelta * NewRotation.Quaternion()).Rotator();

        SoftSync(BodyInst, NewLocation, NewRotation, NewVelocity, NewAngularVelocity);
    }
}

void UAerodynamicsComponent::MovementHardSync()
{
    UPrimitiveComponent *Parent = Cast<UPrimitiveComponent>(GetAttachParent());
    if (!Parent)
    {
        return;
    }
    FName ParentSocket = GetAttachSocketName();
    FBodyInstance *BodyInst = Parent->GetBodyInstance(ParentSocket);
    if (!BodyInst)
    {
        return;
    }

    FTransform Transform = BodyInst->GetUnrealWorldTransform();

    FVector LocalLoc = Transform.GetLocation();
    FRotator LocalRot = FRotator(Transform.GetRotation());
    FVector LocalVel = BodyInst->GetUnrealWorldVelocity();
    FVector LocalAngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();

    //rebase
    LocalLoc = UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(), LocalLoc);

    PositionRepHard(LocalLoc, LocalRot, LocalVel, LocalAngVel);
}

void UAerodynamicsComponent::PositionRepHard_Implementation(FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity)
{
    SCOPE_CYCLE_COUNTER(STAT_MoveReceive);

    UPrimitiveComponent *Parent = Cast<UPrimitiveComponent>(GetAttachParent());

    //do not receive updates if hosting, or parent not valid
    if ((GetOwner()->Role != ROLE_Authority) && (Parent != nullptr))
    {

        FName ParentSocket = GetAttachSocketName();
        FBodyInstance *BodyInst = Parent->GetBodyInstance(ParentSocket);

        if (!BodyInst)
        {
            return;
        }

        FTransform Transform = BodyInst->GetUnrealWorldTransform();

        //rebase
        NewLocation = UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), NewLocation);

        //latency
        NewLocation += NewVelocity * AdditionalLatency;
        FQuat LagCompDelta = FQuat(NewAngularVelocity, AdditionalLatency * PI / 180.0f);
        NewRotation = (LagCompDelta * NewRotation.Quaternion()).Rotator();

        PreviousVelocity = NewVelocity;
        BodyInst->SetBodyTransform(FTransform(NewRotation, NewLocation, Transform.GetScale3D()), ETeleportType::TeleportPhysics);
        BodyInst->SetLinearVelocity(NewVelocity, false);
        BodyInst->SetAngularVelocityInRadians(NewAngularVelocity, false);

        LocError = FVector(0, 0, 0);
        RotError = FQuat::Identity;
        VelError = FVector(0, 0, 0);
        AngVelError = FVector(0, 0, 0);
    }
}

void UAerodynamicsComponent::StoreServerPosition()
{
    UPrimitiveComponent *Parent = Cast<UPrimitiveComponent>(GetAttachParent());
    if (!Parent)
    {
        GotServerPosition = false;
        return;
    }
    FName ParentSocket = GetAttachSocketName();
    FBodyInstance *BodyInst = Parent->GetBodyInstance(ParentSocket);

    if (!BodyInst)
    {
        GotServerPosition = false;
        return;
    }
    GotServerPosition = true;

    FTransform Transform = BodyInst->GetUnrealWorldTransform();

    //ServerLocation = LocalParent->GetComponentLocation();
    //rebase
    ServerLocation = UGameplayStatics::RebaseLocalOriginOntoZero(GetWorld(), Transform.GetLocation());
    ServerRotation = FRotator(Transform.GetRotation());
    ServerLinearVelocity = BodyInst->GetUnrealWorldVelocity();
    ServerAngularVelocity = BodyInst->GetUnrealWorldAngularVelocityInRadians();
}

void UAerodynamicsComponent::MoveToServerPosition()
{
    if (!GotServerPosition)
    {
        return;
    }

    UPrimitiveComponent *Parent = Cast<UPrimitiveComponent>(GetAttachParent());
    if (!Parent)
    {
        return;
    }
    FName ParentSocket = GetAttachSocketName();
    FBodyInstance *BodyInst = Parent->GetBodyInstance(ParentSocket);
    if (!BodyInst)
    {
        return;
    }

    FTransform Transform = BodyInst->GetUnrealWorldTransform();
    //rebase
    BodyInst->SetBodyTransform(FTransform(ServerRotation, UGameplayStatics::RebaseZeroOriginOntoLocal(GetWorld(), ServerLocation), Transform.GetScale3D()), ETeleportType::TeleportPhysics);
    BodyInst->SetLinearVelocity(ServerLinearVelocity, false);
    BodyInst->SetAngularVelocityInRadians(ServerAngularVelocity, false);
    PreviousVelocity = ServerLinearVelocity;
}

void UAerodynamicsComponent::SoftSync(FBodyInstance *BodyInst, FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity)
{

    FTransform Transform = BodyInst->GetUnrealWorldTransform();

    FVector LocalLoc = Transform.GetLocation();
    FRotator LocalRot = FRotator(Transform.GetRotation());
    FVector LocalVel = BodyInst->GetUnrealWorldVelocity();
    FVector LocalAngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();

    //jitter filter
    if (JitterFilter)
    {
        FVector FilteredLocation = FMath::ClosestPointOnSegment(NewLocation - (LocalVel * JitterFilterMaxTime), NewLocation + (LocalVel * JitterFilterMaxTime), LocalLoc);
        NewLocation = FMath::Lerp(FilteredLocation, FVector(NewLocation), JitterFilterCentering);
    }

    LocError = NewLocation - LocalLoc;
    RotError = FQuat(NewRotation.Quaternion() * LocalRot.Quaternion().Inverse());
    VelError = NewVelocity - LocalVel;
    AngVelError = NewAngularVelocity - LocalAngVel;

    if ((!LocationSmoothing) || (!RotationSmoothing))
    {
        FVector Loc = Transform.GetLocation();
        FQuat Rot = Transform.GetRotation();

        if (!RotationSmoothing)
        {
            Loc = NewLocation;
        }

        if (!RotationSmoothing)
        {
            Rot = NewRotation.Quaternion();
        }

        BodyInst->SetBodyTransform(FTransform(Rot, Loc, Transform.GetScale3D()), ETeleportType::TeleportPhysics);
    }

    if (!VelocitySmoothing)
    {
        PreviousVelocity += VelError;
        BodyInst->SetLinearVelocity(NewVelocity, false);
    }

    if (!AngularVelocitySmoothing)
    {
        BodyInst->SetAngularVelocityInRadians(NewAngularVelocity, false);
    }

    if (LocError.Size() > TeleportThreshold)
    {
        LocError = FVector(0);
        RotError = FQuat::Identity;
        VelError = FVector(0);
        AngVelError = FVector(0);

        BodyInst->SetBodyTransform(FTransform(NewRotation, NewLocation, Transform.GetScale3D()), ETeleportType::TeleportPhysics);
        BodyInst->SetLinearVelocity(NewVelocity, false);
        BodyInst->SetAngularVelocityInRadians(NewAngularVelocity, false);

        PreviousVelocity = NewVelocity;
    }
}

void UAerodynamicsComponent::ErrorCatchUp(FBodyInstance *BodyInst, float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_MoveSmoothing);

    if (LocationSmoothing || RotationSmoothing)
    {
        FTransform Transform = BodyInst->GetUnrealWorldTransform();
        FVector LocalLoc = Transform.GetLocation();
        FQuat LocalRot = Transform.GetRotation();
        FVector LocalScale = Transform.GetScale3D();

        if (LocationSmoothing)
        {
            float ErrorDelta = FMath::Clamp(DeltaTime / SmoothingTime, 0.0f, 1.0f);
            LocalLoc = LocalLoc + (LocError * ErrorDelta);
            LocError *= (1.0 - ErrorDelta);
        }
        if (RotationSmoothing)
        {
            float ErrorDelta = FMath::Clamp(DeltaTime / SmoothingTime, 0.0f, 1.0f);
            LocalRot = FQuat::Slerp(LocalRot, RotError * LocalRot, ErrorDelta);
            RotError = FQuat::Slerp(RotError, FQuat::Identity, ErrorDelta);
        }
        BodyInst->SetBodyTransform(FTransform(LocalRot, LocalLoc, LocalScale), ETeleportType::TeleportPhysics);
    }

    if (VelocitySmoothing)
    {
        float ErrorDelta = FMath::Clamp(DeltaTime / SmoothingTime, 0.0f, 1.0f);
        FVector LocalVel = BodyInst->GetUnrealWorldVelocity();
        BodyInst->SetLinearVelocity(VelError * ErrorDelta, true);
        PreviousVelocity += VelError * ErrorDelta;
        VelError *= (1.0 - ErrorDelta);
    }
    if (AngularVelocitySmoothing)
    {
        float ErrorDelta = FMath::Clamp(DeltaTime / SmoothingTime, 0.0f, 1.0f);
        FVector LocalAngVel = BodyInst->GetUnrealWorldAngularVelocityInRadians();
        BodyInst->SetAngularVelocityInRadians(AngVelError * ErrorDelta, true);
        AngVelError *= (1.0 - ErrorDelta);
    }
}

/////////////////////////////////////////////////////////
// get data
/////////////////////////////////////////////////////////

void UAerodynamicsComponent::Extract(
    FBodyInstance *BodyInst,
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
    float &rollrate) const
{

    if (UseThisComponentTransform)
    {
        Transform = this->GetComponentTransform();
    }
    else
    {
        Transform = BodyInst->GetUnrealWorldTransform();
    }

    rawVelocity = BodyInst->GetUnrealWorldVelocity();
    Velocity = (rawVelocity - GetWind(Transform.GetLocation()));
    AngularVelocity = BodyInst->GetUnrealWorldAngularVelocityInRadians() / PI * 180.0f;

    AirDensity = GetAirDensity(Transform.GetLocation());
    SpeedOfSound = GetSpeedOfSound(Transform.GetLocation());

    TotalSpeed = Velocity.Size();

    Mach = TotalSpeed / SpeedOfSound;

    GetAxisVectors(Transform, fwdvec, upvec, rightvec);

    pitchrate = FVector::DotProduct(rightvec, AngularVelocity);
    rollrate = FVector::DotProduct(fwdvec, AngularVelocity);
    yawrate = FVector::DotProduct(upvec, AngularVelocity);
};

void UAerodynamicsComponent::GetData(EGetDataFormat Units, bool UseGravity, FVector &Velocity, FVector &TrueVelocity, float &Altitude, float &IndicatedAirSpeed, float &TrueAirSpeed, float &GroundSpeed, float &Mach,
                                     float &Alpha, float &Slip, float &PitchRate, float &YawRate, float &RollRate, float &AccelX, float &AccelY, float &AccelZ) const
{
    UPrimitiveComponent *LocalParent = Cast<UPrimitiveComponent>(GetAttachParent());
    FBodyInstance *BodyInst = LocalParent->GetBodyInstance();

    if (BodyInst == nullptr)
    {
        return;
    }

    //multipliers
    float AltitudeMultiplier;
    float SpeedMultiplier;
    float AccelerationMultiplier;

    switch (Units)
    {
    case EGetDataFormat::GDF_UU:
        AltitudeMultiplier = 1.0f;
        SpeedMultiplier = 1.0f;
        AccelerationMultiplier = 1.0f;
        break;
    case EGetDataFormat::GDF_Metric:
        AltitudeMultiplier = 0.01f;
        SpeedMultiplier = 0.036f;
        AccelerationMultiplier = 1.0 / 980.0f;
        break;
    case EGetDataFormat::GDF_Imperial:
        AltitudeMultiplier = 0.0328084f;
        SpeedMultiplier = 0.0223694f;
        AccelerationMultiplier = 1.0 / 980.0f;
        break;
    case EGetDataFormat::GDF_Nautical:
        AltitudeMultiplier = 0.0328084f;
        SpeedMultiplier = 0.0194384f;
        AccelerationMultiplier = 1.0 / 980.0f;
        break;
    default:
        AltitudeMultiplier = 1.0f;
        SpeedMultiplier = 1.0f;
        AccelerationMultiplier = 1.0 / 980.0f;
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
    IndicatedAirSpeed = FMath::Sqrt((AirDensity / SeaLevelAirDensity) * FMath::Pow(fwdspeed, 2.0f)) * SpeedMultiplier;
    TrueAirSpeed = fwdspeed * SpeedMultiplier;
    GroundSpeed = TrueVelocity.Size() * SpeedMultiplier;

    FVector DistanceFromOrigin = (Transform.GetLocation() - WorldCenterLocation + FVector(GetWorld()->OriginLocation));
    if (SphericalAltitude)
    {
        Altitude = (DistanceFromOrigin.Size() - SeaLevelRadius) * AltitudeMultiplier;
    }
    else
    {
        Altitude = DistanceFromOrigin.Z * AltitudeMultiplier;
    }

    FVector newAcceleration = acceleration;
    if (UseGravity)
    {
        newAcceleration -= FVector(0.0f, 0.0f, GetWorld()->GetGravityZ());
    };

    AccelX = FVector::DotProduct(newAcceleration, fwdvec) * AccelerationMultiplier;
    AccelY = FVector::DotProduct(newAcceleration, rightvec) * AccelerationMultiplier;
    AccelZ = FVector::DotProduct(newAcceleration, upvec) * AccelerationMultiplier;

    Alpha = GetAlpha(Transform, Velocity);
    Slip = GetSlip(Transform, Velocity);
}

void UAerodynamicsComponent::GetAtmosphereData(FVector &OutWind, float &OutDensity, float &OutPressure, float &OutTemperature) const
{
    UPrimitiveComponent *LocalParent = Cast<UPrimitiveComponent>(GetAttachParent());
    FBodyInstance *BodyInst = LocalParent->GetBodyInstance();

    if (BodyInst == nullptr)
    {
        return;
    }
    FTransform Transform;

    if (UseThisComponentTransform)
    {
        Transform = this->GetComponentTransform();
    }
    else
    {
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
    else
    {
        AltitudeM = DistanceFromOrigin.Z / 100.0f;
    }

    OutDensity = GetAltitudeDensity(AltitudeM);
    OutPressure = GetAltitudePressure(AltitudeM);
    OutTemperature = GetAltitudeTemperature(AltitudeM);
}

float UAerodynamicsComponent::GetAlpha(const FTransform &Transform, FVector velocity) const
{
    FVector fwdvec, upvec, rightvec;
    GetAxisVectors(Transform, fwdvec, upvec, rightvec);

    float fwdspeed = FVector::DotProduct(velocity, fwdvec);
    float upspeed = FVector::DotProduct(velocity, upvec);

    return -FMath::Atan2(upspeed * FMath::Lerp(1.0f, GroundEffectAlphaMultiplier, GroundEffect), fwdspeed) * 180.0f / PI;
};

float UAerodynamicsComponent::GetSlip(const FTransform &Transform, FVector velocity) const
{
    FVector fwdvec, upvec, rightvec;
    GetAxisVectors(Transform, fwdvec, upvec, rightvec);

    float fwdspeed = FVector::DotProduct(velocity, fwdvec);
    float rightspeed = FVector::DotProduct(velocity, rightvec);

    return -FMath::Atan2(rightspeed, fwdspeed) * 180.0f / PI;
};

void UAerodynamicsComponent::GetAxisVectors(const FTransform &Transform, FVector &fwdvec, FVector &upvec, FVector &rightvec) const
{
    fwdvec = Transform.GetUnitAxis(EAxis::X);
    upvec = Transform.GetUnitAxis(EAxis::Z);
    rightvec = Transform.GetUnitAxis(EAxis::Y);
}

/////////////////////////////////////////////////////////
// ground trace
/////////////////////////////////////////////////////////

float UAerodynamicsComponent::GetGroundEffect(const FTransform &transform)
{
    FVector upvec = transform.GetUnitAxis(EAxis::Z);
    return FMath::Pow(GroundTrace(transform.GetLocation(), -upvec, GroundTraceLength), GroundEffectFalloffExponent);
};

float UAerodynamicsComponent::GroundTrace(FVector start, FVector direction, float length)
{
    FHitResult result;
    FCollisionObjectQueryParams params;

    FCollisionQueryParams queryParams;
    queryParams.AddIgnoredActor(this->GetOwner());

    for (TEnumAsByte<ECollisionChannel> col : GroundTraceChannels)
    {
        params.AddObjectTypesToQuery(col);
    };

    bool hit = GetWorld()->LineTraceSingleByObjectType(result, start, start + (direction * length), params, queryParams);
    if (hit)
    {
        return 1.0f - result.Time;
    }
    else
    {
        return 0.0f;
    };
};

/////////////////////////////////////////////////////////
// lift
/////////////////////////////////////////////////////////

float UAerodynamicsComponent::CalculateLift(float alpha, float speed, float mach) const
{
    float lift = 0;
    if (LiftMultiplier != 0)
    {
        lift = LiftMultiplier * GetCurveValue(LiftCurve, alpha, 0);
    };

    if (ScaleFlapsWithAlpha)
    {
        lift += (FlapsLift * FlapsPos * GetCurveValue(FlapsAlphaCurve, alpha));
    }
    else
    {
        lift += (FlapsLift * FlapsPos);
    };

    lift *= FMath::Lerp(1.0f, SpoilerLiftMulti, SpoilerPos);
    lift -= SpoilerDownForce * SpoilerPos;

    if (MachCurves)
    {
        lift *= GetCurveValue(MachLiftCurve, mach);
    };

    lift *= speed;

    lift *= FMath::Lerp(1.0f, GroundEffectLiftMultiplier, GroundEffect);

    return lift;
};

float UAerodynamicsComponent::CalculateSideLift(float slip, float speed, float mach) const
{
    float sidelift = 0;
    if (SideLiftMultiplier != 0)
    {
        sidelift = SideLiftMultiplier * GetCurveValue(SideLiftCurve, slip, 0);
    };

    if (MachCurves)
    {
        sidelift *= GetCurveValue(MachLiftCurve, mach);
    };

    sidelift *= speed;

    return sidelift;
};

float UAerodynamicsComponent::CalculateDrag(float alpha, float slip, float speed, float mach) const
{
    float drag = 0;
    if (DragMultiplier != 0)
    {
        drag = DragMultiplier * GetCurveValue(DragCurve, alpha, 0);
    };
    if (SideDragMultiplier != 0)
    {
        drag += SideDragMultiplier * GetCurveValue(SideDragCurve, slip, 0);
    };

    drag += (FlapsDrag * FlapsPos * GetCurveValue(FlapsAlphaCurve, alpha));
    drag += (SpoilerDrag * SpoilerPos);
    drag += AdditionalDrag;

    if (MachCurves)
    {
        drag *= GetCurveValue(MachDragCurve, mach);
    };

    drag *= speed;

    drag *= FMath::Lerp(1.0f, GroundEffectDragMultiplier, GroundEffect);

    return drag;
};

FVector UAerodynamicsComponent::AeroForces(const FTransform &transform, FVector velocity, float soundv) const
{
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

    if (LinearLift)
    {
        streamspeed = FMath::Abs(upspeed) + FMath::Abs(fwdspeed);
        sidestreamspeed = FMath::Abs(rightspeed) + FMath::Abs(fwdspeed);
        totalstreamspeed = FMath::Abs(totalspeed);
    }
    else
    {
        streamspeed = FMath::Pow(upspeed, 2.0f) + FMath::Pow(fwdspeed, 2.0f);
        sidestreamspeed = FMath::Pow(rightspeed, 2.0f) + FMath::Pow(fwdspeed, 2.0f);
        totalstreamspeed = FMath::Pow(totalspeed, 2.0f);
    };

    float AlphaWFlaps = alpha + (FlapsAlpha * FlapsPos) + (SpoilerAlpha * SpoilerPos);

    lift = CalculateLift(AlphaWFlaps, streamspeed, mach);
    sidelift = CalculateSideLift(slip, sidestreamspeed, mach);
    drag = CalculateDrag(AlphaWFlaps, slip, totalstreamspeed, mach);

    return FVector(lift, sidelift, drag);
};

/////////////////////////////////////////////////////////
// main
/////////////////////////////////////////////////////////

UAerodynamicsComponent::UAerodynamicsComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
    // ...

    bReplicates = true;
    OnCalculateCustomPhysics.BindUObject(this, &UAerodynamicsComponent::CustomPhysics);

    SetTickGroup(ETickingGroup::TG_PrePhysics);
}

// Called when the game starts
void UAerodynamicsComponent::BeginPlay()
{
    Super::BeginPlay();

    //replicate initial position
    if (ReplicateMovement)
    {
        if (GetOwner()->Role == ROLE_Authority)
        {
            StoreServerPosition();
        }
        else
        {
            MoveToServerPosition();
        }
    }
}

//physics substep
void UAerodynamicsComponent::CustomPhysics(float DeltaTime, FBodyInstance *BodyInstance)
{
    Simulate(BodyInstance, DeltaTime, true, false);
}

// Called every frame
void UAerodynamicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    //SCOPE_CYCLE_COUNTER(STAT_Tick);

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!Enabled)
        return;

    UPrimitiveComponent *Parent = Cast<UPrimitiveComponent>(GetAttachParent());
    FName ParentSocket = GetAttachSocketName();

    //safe-proofing
    if (!Parent)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("Aerodynamics ERROR - not attached to UPrimitiveComponent"));
        return;
    }
    if (!Parent->IsSimulatingPhysics())
    {
        GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("Aerodynamics ERROR - parent object doesn't simulate physics"));
        return;
    }

    FBodyInstance *bodyInst = Parent->GetBodyInstance(ParentSocket);

    //multiplayer
    if (ReplicateMovement)
    {
        if (GetOwner()->Role == ROLE_Authority)
        {
            StoreServerPosition();

            TimeSincePosUpdate += DeltaTime;

            MovementBroadcast(bodyInst);
        }
        else
        {
            ErrorCatchUp(bodyInst, DeltaTime);
        }
    }

    if (ReplicateInputs)
    {
        if (GetOwner()->Role == ROLE_AutonomousProxy)
        {
            TimeSinceInputUpdate += DeltaTime;
            InputReplicate();
        }
    }

    //substep
    bool Substep;

    const UPhysicsSettings *Settings = GetDefault<UPhysicsSettings>();
    if (Settings)
    {
        Substep = Settings->bSubstepping;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Project settings inaccessible"));
        Substep = false;
    }

    if (Substep)
    {
        bodyInst->AddCustomPhysics(OnCalculateCustomPhysics);
    }
    else
    {
        Simulate(bodyInst, DeltaTime, true, false);
    }

#ifdef WITH_EDITOR
    if (DrawDebug)
    {
        Simulate(bodyInst, DeltaTime, false, true);
    }
#endif
}

/////////////////////////////////////////////////////////
// simulate
/////////////////////////////////////////////////////////

//Actual simulation
void UAerodynamicsComponent::Simulate(FBodyInstance *bodyInst, float DeltaTime, bool apply, bool debug)
{

    //SCOPE_CYCLE_COUNTER(STAT_SimulationStep);

    FTransform Transform;
    FVector RawVelocity, Velocity, AngularVelocity;
    FVector fwdvec, upvec, rightvec;
    float TotalSpeed;
    float AirDensity, SpeedOfSound, Mach;
    float PitchRate, YawRate, RollRate;

    this->Extract(bodyInst, Transform, RawVelocity, Velocity, AngularVelocity, fwdvec, upvec, rightvec, TotalSpeed, AirDensity, SpeedOfSound, Mach, PitchRate, YawRate, RollRate);

    //Recompute GE
    if (apply && DeltaTime > 0.0f && GroundEffectEnabled)
    {
        GroundEffect = GetGroundEffect(Transform);
    }

    float alpha = GetAlpha(Transform, Velocity);
    float slip = GetSlip(Transform, Velocity);

    if (apply && DeltaTime > 0.0f)
    {
        acceleration = (RawVelocity - PreviousVelocity) / previousDelta;
        PreviousVelocity = RawVelocity;
        previousDelta = DeltaTime;
        SimulateControls(DeltaTime, alpha, slip, PitchRate, YawRate, RollRate, FVector::DotProduct(Velocity, fwdvec), Transform, Transform.GetLocation(), RawVelocity, AirDensity / SeaLevelAirDensity);
    }

    //torque
    FVector controltorque = Control(Transform, Velocity, AirDensity);
    FVector damptorque = Damping(PitchRate, RollRate, YawRate, TotalSpeed, AirDensity);
    FVector stabtorque = Stability(Transform, Velocity, AirDensity, SpeedOfSound);

    float AlphaWithFlaps = alpha + (FlapsAlpha * FlapsPos) + (SpoilerAlpha * SpoilerPos);
    FVector liftvec = upvec.RotateAngleAxis(AlphaWithFlaps + PitchRate * DeltaTime, rightvec); //alpha corrected
    FVector sideliftvec = rightvec.RotateAngleAxis(-slip + YawRate * DeltaTime, upvec);
    FVector dragvec = (Velocity * -1.0);

    liftvec.Normalize();
    sideliftvec.Normalize();
    dragvec.Normalize();

    //engine
    float EngineThrust = 0.0f;
    FVector EngineVector = fwdvec;
    FVector WashVelocity = FVector(0, 0, 0);
    if (EngineEnabled)
    {
        EngineThrust = EnginePower(TotalSpeed, Mach, AirDensity);

        if (VTOLEnabled)
        {
            EngineVector = EngineVector.RotateAngleAxis(-VTOLPos * VTOLMaxAngle, rightvec);
        };

        if (PropWash)
        {
            WashVelocity = (EngineVector * (EngineThrust * PropWashVelocity));

            FVector washcontroltorque = Control(Transform, Velocity + WashVelocity, AirDensity);
            FVector washdamptorque = Damping(PitchRate, RollRate, YawRate, (Velocity + WashVelocity).Size(), AirDensity);
            FVector washstabtorque = Stability(Transform, Velocity + (EngineVector * WashVelocity), AirDensity, SpeedOfSound);

            controltorque = FMath::Lerp(controltorque, washcontroltorque, PropWashPortionControl);
            damptorque = FMath::Lerp(damptorque, washdamptorque, PropWashPortionDamping);
            stabtorque = FMath::Lerp(stabtorque, washstabtorque, PropWashPortionStability);
        }
    }

    //Lift
    FVector Forces = AeroForces(Transform, Velocity, SpeedOfSound);
    if (PropWash && EngineEnabled)
    {
        FVector WashForces = AeroForces(Transform, Velocity + WashVelocity, SpeedOfSound);

        Forces.X = FMath::Lerp(Forces.X, WashForces.X, PropWashPortionLift);
        Forces.Y = FMath::Lerp(Forces.Y, WashForces.Y, PropWashPortionSideLift);
        Forces.Z = FMath::Lerp(Forces.Z, WashForces.Z, PropWashPortionDrag);
    };

    //Lift Asymmetry
    if (LiftAsymmetryEnabled && (AsymmetrySampleCount > 1) && (LiftAsymmetryPortion > 0.0f))
    {
        Forces *= (1.0f - LiftAsymmetryPortion); //remove lift portion
        FVector LocWind = GetWind(Transform.GetLocation());

        for (int i = 0; i < AsymmetrySampleCount; i++)
        {
            float Offset = FMath::Lerp(-WingSpan * 0.5, WingSpan * 0.5, float(i) / (AsymmetrySampleCount - 1));

            FVector LocLocation = bodyInst->GetCOMPosition() + (rightvec * Offset * WorldScale);
            FTransform LocTransform = FTransform(Transform.GetRotation(), LocLocation, FVector(1.0f));

            FVector LocVelocity = bodyInst->GetUnrealWorldVelocityAtPoint(LocLocation);
            LocVelocity -= LocWind;

            FVector LocForce = AeroForces(LocTransform, LocVelocity, SpeedOfSound);

            if (PropWash && EngineEnabled)
            {
                FVector WashForces = AeroForces(LocTransform, LocVelocity + WashVelocity, SpeedOfSound);

                LocForce.X = FMath::Lerp(LocForce.X, WashForces.X, PropWashPortionLift);
                LocForce.Y = FMath::Lerp(LocForce.Y, WashForces.Y, PropWashPortionSideLift);
                LocForce.Z = FMath::Lerp(LocForce.Z, WashForces.Z, PropWashPortionDrag);
            }

            if (Offset < 0.0f)
            {
                LocForce *= AsymLiftMultiplierLeft;
            }
            if (Offset > 0.0f)
            {
                LocForce *= AsymLiftMultiplierRight;
            }

            //asymm lift direction
            FVector LocForceRotated = (liftvec * LocForce.X);
            LocForceRotated += (sideliftvec * LocForce.Y);
            LocForceRotated += (dragvec * LocForce.Z);

            if (apply && DeltaTime > 0.0f)
            {
                FVector FinalForceLoc = LocForceRotated * AirDensity;

                if (ScaleWithMass)
                {
                    FinalForceLoc *= bodyInst->GetBodyMass() / 10000.0f;
                }

                FinalForceLoc *= LiftAsymmetryPortion / float(AsymmetrySampleCount);

                FVector ForcePoint = LocLocation; //+( RawVelocity * DeltaTime * 0.5f );
                bodyInst->AddForceAtPosition(FinalForceLoc, ForcePoint, false);
            }

#ifdef WITH_EDITOR
            if (debug)
            {
                FVector DebugLoc = LocLocation;
                if (DebugLagCompensation)
                {
                    DebugLoc += RawVelocity * DeltaTime;
                }

                DrawDebugLine(GetWorld(), DebugLoc, DebugLoc + (LocForceRotated * WorldScale * DebugForceScale), FColor::Green, false, -1.0f, 1, 0);
            }
#endif
        }
    }

    //lift direction
    FVector AeroForcesRotated = (liftvec * Forces.X);
    AeroForcesRotated += (sideliftvec * Forces.Y);
    AeroForcesRotated += (dragvec * Forces.Z);

    //RCS
    FVector rcs = RCSTorque(EngineThrust);

    //torque combine
    FVector torque = stabtorque + damptorque + controltorque + rcs;
    torque = torque.GetClampedToMaxSize(TorqueLimit);
    FVector torqueLoc = Transform.GetRotation() * torque;

    FVector finalForce = ((AeroForcesRotated * AirDensity) + FVector(0.0f, 0.0f, AirDensity * Buoyancy) + (EngineVector * EngineThrust * EngineFullPower));

    if (ScaleWithMass)
    {
        finalForce *= bodyInst->GetBodyMass() / 10000.0f;
    }

    //prop torque
    FVector finalTorque = (torqueLoc +
                           (fwdvec * EngineThrust * PropTorqueRoll) +
                           (upvec * EngineThrust * PropTorqueYaw) +
                           (rightvec * EngineThrust * PropTorquePitch));

    if (ScaleWithMomentOfInertia)
    {
        finalTorque = Cast<UPrimitiveComponent>(GetAttachParent())->ScaleByMomentOfInertia(finalTorque, GetAttachSocketName()) * 0.000001f;
    }

    if (apply && DeltaTime > 0.0f)
    {
        bodyInst->AddForce(finalForce, false, false);
        bodyInst->AddTorqueInRadians(finalTorque, false, false);
    }

#ifdef WITH_EDITOR
    if (debug)
    {
        float accfwd = FVector::DotProduct(acceleration, fwdvec) / WorldScale;
        float accright = FVector::DotProduct(acceleration, rightvec) / WorldScale;
        float accup = FVector::DotProduct(acceleration, upvec) / WorldScale;
        FVector DebugLoc = Transform.GetLocation();
        if (DebugLagCompensation)
        {
            DebugLoc += RawVelocity * DeltaTime;
        }

        if (PrintFMData)
        {
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("groundeffect: ") + FString::SanitizeFloat(GroundEffect));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("accelZ: ") + FString::SanitizeFloat(accup));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("accelY: ") + FString::SanitizeFloat(accright));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("accelX: ") + FString::SanitizeFloat(accfwd));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("trimRoll: ") + FString::SanitizeFloat(RollTrimPos));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("trimYaw: ") + FString::SanitizeFloat(YawTrimPos));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("trimPitch: ") + FString::SanitizeFloat(PitchTrimPos));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("autothrottle: ") + FString::SanitizeFloat(AutoThrottlePos));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("glideRatio (no asymm): ") + FString::SanitizeFloat(Forces.X / Forces.Z));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("drag: ") + FString::SanitizeFloat(Forces.Z));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("lift: ") + FString::SanitizeFloat(Forces.X));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("ALT: ") + FString::SanitizeFloat(Transform.GetLocation().Z / WorldScale));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("IAS: ") + FString::SanitizeFloat(FVector::DotProduct(Velocity, fwdvec) * FMath::Sqrt(AirDensity / SeaLevelAirDensity) / WorldScale));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("mach: ") + FString::SanitizeFloat(Mach));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("AoA: ") + FString::SanitizeFloat(alpha));
            GEngine->AddOnScreenDebugMessage(-1, 0, FColor::White, TEXT("slip: ") + FString::SanitizeFloat(slip));
        };

        //debug trace
        if (DebugLineLength > 0)
        {
            DrawDebugLine(GetWorld(), DebugLoc, DebugLoc + (fwdvec * DebugLineLength), FColor::Red, false, -1.0f, 0, 0);
            DrawDebugLine(GetWorld(), DebugLoc, DebugLoc + (upvec * DebugLineLength), FColor::Blue, false, -1.0f, 0, 0);
            DrawDebugLine(GetWorld(), DebugLoc, DebugLoc + (RawVelocity.GetSafeNormal() * DebugLineLength), FColor::Green, false, -1.0f, 1, 0);
            if (Guided)
            {
                if (GuidedToLocation)
                {
                    DrawDebugLine(GetWorld(), DebugLoc, GuidedVector, FColor::Yellow, false, -1.0f, 1, 0);
                }
                else
                {
                    DrawDebugLine(GetWorld(), DebugLoc, DebugLoc + ((GuidedVector.GetSafeNormal() * DebugLineLength)), FColor::Yellow, false, -1.0f, 1, 0);
                };
            };
        };

        if (DebugTrailTime > 0)
        {
            DrawDebugLine(GetWorld(), DebugLoc, DebugLoc - (RawVelocity * DeltaTime), FColor::White, false, DebugTrailTime, 0, 0);
        };

        //GE debug
        if (GroundEffectEnabled)
        {
            if (GroundEffect > 0.0f)
            {
                DrawDebugLine(GetWorld(), DebugLoc, DebugLoc - (upvec * GroundTraceLength * (1.0f - GroundEffect)), FColor::White, false, -1.0f, 1, 0);
            }
            else
            {
                DrawDebugLine(GetWorld(), DebugLoc, DebugLoc - (upvec * GroundTraceLength), FColor::Cyan, false, -1.0f, 1, 0);
            };
        };
    };
#endif
}

/////////////////////////////////////////////////////////
// state
/////////////////////////////////////////////////////////

TArray<uint8> UAerodynamicsComponent::GetState()
{
    TArray<uint8> Data;
    FMemoryWriter Writer = FMemoryWriter(Data);
    FAerodynamicsArchive Ar = FAerodynamicsArchive(Writer, true);
    Serialize(Ar);
    return Data;
}

void UAerodynamicsComponent::SetState(TArray<uint8> Data)
{
    FMemoryReader Reader = FMemoryReader(Data);
    FAerodynamicsArchive Ar = FAerodynamicsArchive(Reader, true);
    Serialize(Ar);
}

/////////////////////////////////////////////////////////
// torque
/////////////////////////////////////////////////////////

FVector UAerodynamicsComponent::Damping(float pitchrate, float rollrate, float yawrate, float totalspeed, float air) const
{
    FVector damptorque;
    damptorque.X = -rollrate * RollDamping;
    damptorque.Y = -pitchrate * PitchDamping;
    damptorque.Z = -yawrate * YawDamping;

    if (!LinearDamping)
    {
        return damptorque * (pow(totalspeed, 2.0f)) * air;
    }
    else
    {
        return damptorque * totalspeed * air;
    };
}

FVector UAerodynamicsComponent::Control(const FTransform &Transform, FVector velocity, float air) const
{

    float pitchSensMultiplier = 1.0;
    float yawSensMultiplier = 1.0;
    float rollSensMultiplier = 1.0;

    float TotalSpeed = velocity.Size();

    //scale sensitivity
    if (ScaleSensitivityWithAlpha)
    {
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
    controltorque.X = -RollPos * RollSensitivity * rollSensMultiplier;
    controltorque.Y = PitchPos * PitchSensitivity * pitchSensMultiplier;
    controltorque.Z = YawPos * YawSensitivity * yawSensMultiplier;

    if (!LinearControlSensitivity)
    {
        return controltorque * (pow(TotalSpeed, 2.0f)) * air;
    }
    else
    {
        return controltorque * TotalSpeed * air;
    };
}

FVector UAerodynamicsComponent::Stability(const FTransform &transform, FVector velocity, float air, float soundv) const
{

    //scale stability
    float pitchStabMultiplier = 1.0;
    float yawStabMultiplier = 1.0;
    float rollStabMultiplier = 1.0;

    float TotalSpeed = velocity.Size();

    float alpha = GetAlpha(transform, velocity);
    float slip = GetSlip(transform, velocity);

    if (ScaleStabilityWithAlpha)
    {

        pitchStabMultiplier *= GetCurveValue(AlphaStabilityCurve.Pitch, alpha);
        yawStabMultiplier *= GetCurveValue(AlphaStabilityCurve.Yaw, alpha);
        rollStabMultiplier *= GetCurveValue(AlphaStabilityCurve.Roll, alpha);

        pitchStabMultiplier *= GetCurveValue(SlipStabilityCurve.Pitch, slip);
        yawStabMultiplier *= GetCurveValue(SlipStabilityCurve.Yaw, slip);
        rollStabMultiplier *= GetCurveValue(SlipStabilityCurve.Roll, slip);
    }

    //mach stability
    if (MachCurves)
    {
        float mach = TotalSpeed / soundv;

        float mscurveval = GetCurveValue(MachStabilityCurve, mach);
        pitchStabMultiplier *= mscurveval;
        yawStabMultiplier *= mscurveval;
        rollStabMultiplier *= mscurveval;
    }

    pitchStabMultiplier *= FMath::Lerp(1.0f, SpoilerStabilityMulti, SpoilerPos);
    yawStabMultiplier *= FMath::Lerp(1.0f, SpoilerStabilityMulti, SpoilerPos);
    rollStabMultiplier *= FMath::Lerp(1.0f, SpoilerStabilityMulti, SpoilerPos);

    float AlphaWFlaps = alpha + (FlapsAlpha * FlapsPos) + (SpoilerAlpha * SpoilerPos);

    FVector stabtorque;
    stabtorque.X = FMath::Sin((-slip) * PI / 180.0f) * RollStability * rollStabMultiplier;
    stabtorque.Y = FMath::Sin((AlphaWFlaps - FrameTrimPitch) * PI / 180.0f) * PitchStability * pitchStabMultiplier;
    stabtorque.Z = FMath::Sin((-slip - FrameTrimYaw) * PI / 180.0f) * YawStability * yawStabMultiplier;

    if (!LinearStability)
    {
        return stabtorque * FMath::Pow(TotalSpeed, 2.0f) * air;
    }
    else
    {
        return stabtorque * TotalSpeed * air;
    };
}

FVector UAerodynamicsComponent::RCSTorque(float ThrustTemp) const
{
    if (RCSEnabled)
    {

        float ThrustMultiplier;
        if (RCSScaleWithEnginePower)
        {
            ThrustMultiplier = ThrustTemp;
        }
        else
        {
            ThrustMultiplier = 1.0f;
        };

        FVector RCStorque;
        RCStorque.X = -RollPos * RCSRollSensitivity * ThrustMultiplier;
        RCStorque.Y = PitchPos * RCSPitchSensitivity * ThrustMultiplier;
        RCStorque.Z = YawPos * RCSYawSensitivity * ThrustMultiplier;
        return RCStorque;
    }
    else
    {
        return FVector(0, 0, 0);
    }
}
