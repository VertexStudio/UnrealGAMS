#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/SceneComponent.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Curves/CurveFloat.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/World.h"

#include "AerodynamicsComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("AerodynamicsModel"), STATGROUP_Aerodynamics, STATCAT_Advanced);

struct FAerodynamicsArchive : public FObjectAndNameAsStringProxyArchive
{
    FAerodynamicsArchive(FArchive &InInnerArchive, bool bInLoadIfFindFails) : FObjectAndNameAsStringProxyArchive(InInnerArchive, bInLoadIfFindFails) { ArIsSaveGame = true; }
};

USTRUCT(BlueprintType)
struct FGuidedPIDStruct
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Pitch Proportional"))
    float PitchP;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Pitch Integral"))
    float PitchI;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Pitch Derivative"))
    float PitchD;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Yaw Proportional"))
    float YawP;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Yaw Integral"))
    float YawI;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Yaw Derivative"))
    float YawD;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Roll Proportional"))
    float RollP;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Roll Integral"))
    float RollI;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "GuidedPIDStruct", meta = (ToolTip = "Roll Derivative"))
    float RollD;
};

USTRUCT(BlueprintType)
struct FPYRStruct
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "PYRStruct")
    UCurveFloat *Pitch;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "PYRStruct")
    UCurveFloat *Yaw;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "PYRStruct")
    UCurveFloat *Roll;
};

UENUM(BlueprintType)
enum class EGetDataFormat : uint8
{
    GDF_UU UMETA(DisplayName = "UnrealUnits"),
    GDF_Metric UMETA(DisplayName = "Metric"),
    GDF_Imperial UMETA(DisplayName = "Imperial"),
    GDF_Nautical UMETA(DisplayName = "Nautical")
};

UENUM(BlueprintType)
enum class EAtmosphereType : uint8
{
    AT_Constant UMETA(DisplayName = "Constant"),
    AT_Curve UMETA(DisplayName = "Density Curve"),
    AT_Earth UMETA(DisplayName = "Earth/IGL")
};

UCLASS(ClassGroup = (DynamicalSystems), meta = (BlueprintSpawnableComponent))
class DYNAMICALSYSTEMS_API UAerodynamicsComponent : public USceneComponent
{
    GENERATED_BODY()

  public:
    // Sets default values for this component's properties
    UAerodynamicsComponent();

    FCalculateCustomPhysics OnCalculateCustomPhysics;

    UFUNCTION(BlueprintPure, Category = "Outputs")
    void GetData(
        EGetDataFormat Units,
        bool UseGravity,
        FVector &Velocity,
        FVector &TrueVelocity,
        float &Altitude,
        float &IndicatedAirSpeed,
        float &TrueAirSpeed,
        float &GroundSpeed,
        float &Mach,
        float &Alpha,
        float &Slip,
        float &PitchRate,
        float &YawRate,
        float &RollRate,
        float &AccelX,
        float &AccelY,
        float &AccelZ) const;

    UFUNCTION(BlueprintPure, Category = "Outputs")
    void GetAtmosphereData(
        FVector &OutWind,
        float &OutDensity,
        float &OutPressure,
        float &OutTemperature) const;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", meta = (ToolTip = "Disable to turn off all forces and replication"))
    bool Enabled = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", meta = (ToolTip = "Use transformation of this component instead of parent body to determine forward direction etc"))
    bool UseThisComponentTransform = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", meta = (ToolTip = "Scale all linear forces with mass of the parent body, when enabled linear acceleration (lift etc) is the same regardless of the mass"))
    bool ScaleWithMass;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", meta = (ToolTip = "Scale all angular forces with inertia tensor of the parent body, when enabled angular acceleration (stability, damping etc) is the same regardless of mass and dimensons"))
    bool ScaleWithMomentOfInertia;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", meta = (ToolTip = "Display lines indicating axis, direction"))
    bool DrawDebug;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", meta = (ToolTip = "Compensate for 1-frame lag of debug draw"))
    bool DebugLagCompensation = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", Meta = (EditCondition = "DrawDebug", ToolTip = "Show debug data"))
    bool PrintFMData;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", Meta = (EditCondition = "DrawDebug", ClampMin = "0"))
    float DebugLineLength;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", Meta = (EditCondition = "DrawDebug", ClampMin = "0"))
    float DebugForceScale = 0.01;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Aerodynamics", Meta = (EditCondition = "DrawDebug", ClampMin = "0"))
    float DebugTrailTime;

    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World")
    FVector Wind;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "Select atmosphere model"))
    EAtmosphereType AtmosphereType;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "Air density at zero altitude - in kg/m3"))
    float SeaLevelAirDensity = 1.225;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World")
    UCurveFloat *AirDensityCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "Atmosphere pressure at 0,0,0 - in millibars"))
    float SeaLevelAirPressure = 1012.5f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "Atmosphere Temperature at 0,0,0 - in degrees C"))
    float SeaLevelAirTemperature = 20.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "Temperature Decrease With Altitude, degrees per meter"))
    float TemperatureLapseRate = 0.00649f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "Altitude at which temperature stops decreasing, in meters"))
    float TropopauseAltitude = 11000.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "Specific Gas Constant, dry air = 287.058"))
    float SpecificGasConstant = 287.058;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "World Origin Location"))
    FVector WorldCenterLocation = FVector(0, 0, 0);
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "World Origin Location"))
    bool SphericalAltitude = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "World Origin Location"), Meta = (EditCondition = "SphericalAltitude"))
    float SeaLevelRadius = 637100000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World")
    bool SpeedOfSoundVariesWithAltitude = false;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "Speed of sound at zero altitude - in unreal units"))
    float SeaLevelSpeedOfSound = 34300;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", Meta = (EditCondition = "SpeedOfSoundVariesWithAltitude"))
    UCurveFloat *SpeedOfSoundCurve;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "World", meta = (ToolTip = "1.0 = default UE scale"))
    float WorldScale = 1.0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "Multiplier for lift caused by horizontal surfaces"))
    float LiftMultiplier;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "Controls how lift varies with angle of attack, X axis = degrees AOA (from -180 to +180), Y = coeffient of lift"))
    UCurveFloat *LiftCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ClampMin = "0", ToolTip = "Multiplier for drag caused by horizontal surfaces"))
    float DragMultiplier;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "Controls how drag varies with angle of attack, X axis = degrees AOA (from -180 to +180), Y = coeffient of drag"))
    UCurveFloat *DragCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "Multiplier for lift caused by vertical surfaces"))
    float SideLiftMultiplier;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "Controls how lift varies with angle of attack, X axis = degrees AOA (from -180 to +180), Y = coeffient of lift"))
    UCurveFloat *SideLiftCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ClampMin = "0", ToolTip = "Multiplier for lift caused by vertical surfaces"))
    float SideDragMultiplier;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "Controls how drag varies with angle of attack, X axis = degrees AOA (from -180 to +180), Y = coeffient of drag"))
    UCurveFloat *SideDragCurve;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "Additional constant drag"))
    float AdditionalDrag;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "Upwards lift caused by buoyancy (for airships)"))
    float Buoyancy;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift", meta = (ToolTip = "linear scaling with velocity - unrealistic but gives wider range of usable speeds"))
    bool LinearLift;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability")
    float PitchStability;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability")
    float YawStability;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability")
    float RollStability;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability")
    bool ScaleStabilityWithAlpha;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability", Meta = (EditCondition = "ScaleStabilityWithAlpha"))
    FPYRStruct AlphaStabilityCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability", Meta = (EditCondition = "ScaleStabilityWithAlpha"))
    FPYRStruct SlipStabilityCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability", meta = (ClampMin = "0"))
    float PitchDamping;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability", meta = (ClampMin = "0"))
    float YawDamping;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability", meta = (ClampMin = "0"))
    float RollDamping;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability")
    bool LinearStability;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability")
    bool LinearDamping;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Stability")
    float TorqueLimit = FLT_MAX;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Trim")
    float FrameTrimPitch;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Trim")
    float FrameTrimYaw;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Trim")
    float PitchTrimPos;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Trim")
    float YawTrimPos;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Trim")
    float RollTrimPos;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control")
    float PitchSensitivity;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control")
    float YawSensitivity;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control")
    float RollSensitivity;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control", meta = (ClampMin = "0"))
    float PitchMoveTime;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control", meta = (ClampMin = "0"))
    float YawMoveTime;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control", meta = (ClampMin = "0"))
    float RollMoveTime;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control")
    bool LinearControlSensitivity;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control")
    bool ScaleSensitivityWithAlpha;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control", Meta = (EditCondition = "ScaleSensitivityWithAlpha"))
    FPYRStruct AlphaSensitivityCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Control", Meta = (EditCondition = "scaleSensitivityWithAlpha"))
    FPYRStruct SlipSensitivityCurve;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps")
    float FlapsLift;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps", meta = (ClampMin = "0"))
    float FlapsDrag;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps")
    float FlapsAlpha;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps", meta = (ClampMin = "0"))
    float FlapsExtensionTime;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps", meta = (ClampMin = "0"))
    float FlapsRetractionTime;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps")
    bool ScaleFlapsWithAlpha;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps", meta = (EditCondition = " ScaleFlapsWithAlpha"))
    UCurveFloat *FlapsAlphaCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps", meta = (ClampMin = "0"))
    float SpoilerDrag;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps")
    float SpoilerLiftMulti = 1.0;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps")
    float SpoilerDownForce;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps")
    float SpoilerAlpha;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps")
    float SpoilerStabilityMulti = 1.0;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps", meta = (ClampMin = "0"))
    float SpoilerExtensionTime = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Flaps", meta = (ClampMin = "0"))
    float SpoilerRetractionTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Mach")
    bool MachCurves = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Mach", Meta = (EditCondition = "MachCurves"))
    UCurveFloat *MachLiftCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Mach", Meta = (EditCondition = "MachCurves"))
    UCurveFloat *MachDragCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Mach", Meta = (EditCondition = "MachCurves"))
    UCurveFloat *MachStabilityCurve;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Ground Effect")
    bool GroundEffectEnabled = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Ground Effect", Meta = (EditCondition = "GroundEffectEnabled"))
    float GroundTraceLength = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Ground Effect", Meta = (EditCondition = "GroundEffectEnabled"))
    TArray<TEnumAsByte<ECollisionChannel>> GroundTraceChannels;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Ground Effect", Meta = (EditCondition = "GroundEffectEnabled"))
    float GroundEffectFalloffExponent = 1.0;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Ground Effect", Meta = (EditCondition = "GroundEffectEnabled"))
    float GroundEffectLiftMultiplier = 1.0;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Ground Effect", Meta = (EditCondition = "GroundEffectEnabled"))
    float GroundEffectDragMultiplier = 1.0;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Ground Effect", Meta = (EditCondition = "GroundEffectEnabled"))
    float GroundEffectAlphaMultiplier = 1.0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift Asymmetry")
    bool LiftAsymmetryEnabled = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift Asymmetry", Meta = (EditCondition = "LiftAsymmetryEnabled"))
    float LiftAsymmetryPortion = 1.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift Asymmetry", Meta = (EditCondition = "LiftAsymmetryEnabled"))
    float WingSpan = 1000.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift Asymmetry", Meta = (EditCondition = "LiftAsymmetryEnabled", ClampMin = "2"))
    int AsymmetrySampleCount = 6;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift Asymmetry", Meta = (EditCondition = "LiftAsymmetryEnabled"))
    float AsymLiftMultiplierLeft = 1.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Lift Asymmetry", Meta = (EditCondition = "LiftAsymmetryEnabled"))
    float AsymLiftMultiplierRight = 1.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine")
    bool EngineEnabled = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine")
    float EngineIdlePower = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine")
    float EngineFullPower = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine")
    bool EngineCurves = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine", Meta = (EditCondition = "EngineCurves"))
    UCurveFloat *DensityPowerCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine", Meta = (EditCondition = "EngineCurves"))
    UCurveFloat *SpeedPowerCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine", Meta = (EditCondition = "EngineCurves"))
    UCurveFloat *MachPowerCurve;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine", meta = (ClampMin = "0"))
    float SpoolUpTime = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Engine", meta = (ClampMin = "0"))
    float SpoolDownTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash")
    float PropTorquePitch = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash")
    float PropTorqueYaw = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash")
    float PropTorqueRoll = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash")
    bool PropWash = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash", Meta = (EditCondition = "PropWash"))
    float PropWashVelocity = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash", Meta = (EditCondition = "PropWash"))
    float PropWashPortionLift = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash", Meta = (EditCondition = "PropWash"))
    float PropWashPortionSideLift = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash", Meta = (EditCondition = "PropWash"))
    float PropWashPortionDrag = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash", Meta = (EditCondition = "PropWash"))
    float PropWashPortionStability = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash", Meta = (EditCondition = "PropWash"))
    float PropWashPortionDamping = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Propwash", Meta = (EditCondition = "PropWash"))
    float PropWashPortionControl = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Reverser")
    float ReverserThrustPortion = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Reverser", meta = (ClampMin = "0"))
    float ReverserExtensionTime = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Reverser", meta = (ClampMin = "0"))
    float ReverserRetractionTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "VTOL")
    bool VTOLEnabled = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "VTOL", Meta = (EditCondition = "VTOLEnabled"))
    float VTOLMaxAngle = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "VTOL", meta = (EditCondition = "VTOLEnabled", ClampMin = "0"))
    float VTOLExtensionTime = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "VTOL", meta = (EditCondition = "VTOLEnabled", ClampMin = "0"))
    float VTOLRetractionTime = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "RCS")
    bool RCSEnabled = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "RCS", Meta = (EditCondition = "RCSEnabled"))
    bool RCSScaleWithEnginePower = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "RCS", Meta = (EditCondition = "RCSEnabled"))
    float RCSPitchSensitivity = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "RCS", meta = (EditCondition = "RCSEnabled"))
    float RCSYawSensitivity = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "RCS", meta = (EditCondition = "RCSEnabled"))
    float RCSRollSensitivity = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists")
    bool AlphaLimiter = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AlphaLimiter"))
    float MinAlpha = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AlphaLimiter"))
    float MaxAlpha = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AlphaLimiter"))
    float PitchRateInfluence = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AlphaLimiter"))
    float AlphaLimiterSensitivity = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists")
    bool SlipLimiter = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "SlipLimiter"))
    float MaxSlip = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "SlipLimiter"))
    float YawRateInfluence = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "SlipLimiter"))
    float SlipLimiterSensitivity = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists")
    bool StabilityAugmentation = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "StabilityAugmentation", ClampMin = "0"))
    float PitchAug = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "StabilityAugmentation", ClampMin = "0"))
    float YawAug = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "StabilityAugmentation", ClampMin = "0"))
    float RollAug = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists")
    bool AutoTrim = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoTrim"))
    bool InputPauseAutoTrim = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoTrim", ClampMin = "0"))
    float PauseAutoTrimThreshold = 0.01f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoTrim"))
    bool TrimPitchTo1G = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoTrim"))
    bool TrimYawToCenter = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoTrim"))
    float AutoTrimPitchSensitivity = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoTrim"))
    float AutoTrimYawSensitivity = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoTrim"))
    float AutoTrimRollSensitivity = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists")
    float YawCentering = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists")
    bool AutoThrottle = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoThrottle"))
    float AutoThrottleSensitivity = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoThrottle"))
    bool InputPauseAutoThrottle = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoThrottle", ClampMin = "0"))
    float PauseAutoThrottleThreshold = 0.01f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoThrottle"))
    bool AutoSpoiler = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoThrottle"))
    bool AutoThrottleHover = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoThrottle"))
    float HoverAccelerationInfluence = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists")
    bool AutoFlaps = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoFlaps", ClampMin = "0"))
    float FullExtendSpeed = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "AutoFlaps", ClampMin = "0"))
    float FullRetractSpeed = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists")
    bool ScaleSensitivityWithSpeed = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Assists", Meta = (EditCondition = "ScaleSensitivityWithSpeed"))
    FPYRStruct SpeedSensitivityCurve;

    UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Guidance")
    bool Guided = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Guidance", Meta = (EditCondition = "Guided"))
    bool GuidedCompensateAlpha = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Guidance", Meta = (EditCondition = "Guided"))
    bool GuidedToLocation = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Guidance", Meta = (EditCondition = "Guided"))
    FGuidedPIDStruct GuidedPID;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Guidance", Meta = (EditCondition = "Guided"))
    float GuidedAutoLevel = 0.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Guidance", Meta = (EditCondition = "Guided"))
    bool GuidedPitchFullRange = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Guidance", Meta = (EditCondition = "Guided"))
    bool GuidedYawFullRange = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Damping Speed Limit")
    bool DampingScaleWithSpeed = false;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Damping Speed Limit", Meta = (EditCondition = "DampingScaleWithSpeed"))
    float DampingShutoffSpeed = 0.0f;

    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    float PitchInput = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    float YawInput = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    float RollInput = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    float ThrottleInput = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    float FlapsInput = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    float SpoilerInput = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    float VTOLInput = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    float ReverserInput = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs")
    FVector GuidedVector = FVector(0, 0, 0);

    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float PitchPos = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float YawPos = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float RollPos = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float ThrottlePos = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float VTOLPos = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float FlapsPos = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float SpoilerPos = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float ReverserPos = 0.0f;
    UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Outputs")
    float AutoThrottlePos = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication")
    bool ReplicateMovement = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    bool HighPrecision = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    float MovementReplicationFrequency = 5.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    bool LocationSmoothing = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    bool RotationSmoothing = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    bool VelocitySmoothing = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    bool AngularVelocitySmoothing = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    float SmoothingTime = 0.5f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    float TeleportThreshold = 1000.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    float AdditionalLatency = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateMovement"))
    bool JitterFilter = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "JitterFilter"))
    float JitterFilterMaxTime = 0.1f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "JitterFilter"))
    float JitterFilterCentering = 0.1f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication")
    bool MulticastInputs = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication")
    bool MulticastTrim = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication")
    bool ReplicateInputs = true;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateInputs"))
    float InputReplicationFrequency = 30.0f;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateInputs"))
    bool LocalInputPrediction = true;

    // Called when the game starts
    virtual void BeginPlay() override;

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    void ErrorCatchUp(FBodyInstance *BodyInst, float DeltaTime);

    UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Replication_")
    void MovementHardSync();

    UFUNCTION(BlueprintNativeEvent, Category = "World")
    FVector GetWind(FVector Location) const;
    UFUNCTION(BlueprintNativeEvent, Category = "World")
    float GetAirDensity(FVector Location) const;
    UFUNCTION(BlueprintNativeEvent, Category = "World")
    float GetSpeedOfSound(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "State")
    TArray<uint8> GetState();
    UFUNCTION(BlueprintCallable, Category = "State")
    void SetState(TArray<uint8> Data);

    UFUNCTION(BlueprintCallable, Category = "Input")
    float SetPitchInput(float Input);
    UFUNCTION(BlueprintCallable, Category = "Input")
    float SetYawInput(float Input);
    UFUNCTION(BlueprintCallable, Category = "Input")
    float SetRollInput(float Input);
    UFUNCTION(BlueprintCallable, Category = "Input")
    float SetThrottleInput(float Input);
    UFUNCTION(BlueprintCallable, Category = "Input")
    float SetFlapsInput(float Input);
    UFUNCTION(BlueprintCallable, Category = "Input")
    float SetSpoilerInput(float Input);
    UFUNCTION(BlueprintCallable, Category = "Input")
    float SetVTOLInput(float Input);
    UFUNCTION(BlueprintCallable, Category = "Input")
    float SetReverserInput(float Input);

  private:
    float ClientPitchInput = 0.0f;
    float ClientYawInput = 0.0f;
    float ClientRollInput = 0.0f;
    float ClientThrottleInput = 0.0f;
    float ClientFlapsInput = 0.0f;
    float ClientSpoilerInput = 0.0f;
    float ClientVTOLInput = 0.0f;
    float ClientReverserInput = 0.0f;

    UFUNCTION(Server, WithValidation, Unreliable)
    void InputRep(float Pitch, float Yaw, float Roll, float Throttle);
    UFUNCTION(Server, WithValidation, Unreliable)
    void SecInputRep(float Flaps, float Spoiler, float VTOL, float Reverser);

    void StoreServerPosition();
    void MoveToServerPosition();
    UPROPERTY(Replicated)
    bool GotServerPosition = false;
    UPROPERTY(Replicated)
    FVector ServerLocation;
    UPROPERTY(Replicated)
    FRotator ServerRotation;
    UPROPERTY(Replicated)
    FVector ServerLinearVelocity;
    UPROPERTY(Replicated)
    FVector ServerAngularVelocity;

    void MovementBroadcast(FBodyInstance *BodyInst);
    void InputReplicate();
    UFUNCTION(NetMulticast, Unreliable)
    void PositionRep(FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity);
    UFUNCTION(NetMulticast, Unreliable)
    void PositionRepLow(FVector_NetQuantize10 NewLocation, FRotator NewRotation, FVector_NetQuantize NewVelocity, FVector_NetQuantize10 NewAngularVelocity);
    UFUNCTION(NetMulticast, Reliable)
    void PositionRepHard(FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity);
    void SoftSync(FBodyInstance *BodyInst, FVector NewLocation, FRotator NewRotation, FVector NewVelocity, FVector NewAngularVelocity);

    float TimeSincePosUpdate = 0;
    float TimeSinceInputUpdate = 0;

    FVector LocError = FVector(0, 0, 0);
    FQuat RotError = FQuat::Identity;
    FVector VelError = FVector(0, 0, 0);
    FVector AngVelError = FVector(0, 0, 0);

    float previousAirspeed = 0;
    float previousDelta = FLT_MAX;
    float GroundEffect = 0;
    FVector PreviousVelocity = FVector(0, 0, 0);
    FVector acceleration = FVector(0, 0, 0);

    void Extract(
        FBodyInstance *bodyInst,
        FTransform &transform,
        FVector &rawVelocity,
        FVector &velocity,
        FVector &angular,
        FVector &fwdvec,
        FVector &upvec,
        FVector &rightvec,
        float &totalspeed,
        float &air,
        float &soundv,
        float &mach,
        float &pitchrate,
        float &yawrate,
        float &rollrate) const;
    float GetAlpha(const FTransform &transform, FVector velocity) const;
    float GetSlip(const FTransform &transform, FVector velocity) const;
    void GetAxisVectors(const FTransform &transform, FVector &fwdvec, FVector &upvec, FVector &rightvec) const;

    void CustomPhysics(float DeltaTime, FBodyInstance *BodyInstance);
    void Simulate(FBodyInstance *Parent, float DeltaTime, bool apply, bool debug);
    void SimulateControls(float DeltaTime, float alpha, float slip, float pitchrate, float yawrate, float rollrate, float speed, const FTransform &Transform, FVector location, FVector rawVelocity, float air);
    float BlendInput(float low, float high) const;
    float Limiter(float input, float limitedVar, float min, float max, float sensitivity) const;
    float Transit(float input, float target, float upspeed, float downspeed, float DeltaTime) const;
    float GetCurveValue(const UCurveFloat *curve, float in, float deflt = 1.0f) const;

    FVector AeroForces(const FTransform &transform, FVector velocity, float soundv) const;
    FVector RCSTorque(float ThrustTemp) const;
    FVector Damping(float pitchrate, float rollrate, float yawrate, float totalspeed, float air) const;
    FVector Control(const FTransform &transform, FVector velocity, float air) const;
    FVector Stability(const FTransform &transform, FVector velocity, float air, float soundv) const;

    float GetAltitudePressure(float AltitudeMeter) const;
    float GetAltitudeTemperature(float AltitudeMeter) const;
    float GetAltitudeDensity(float AltitudeMeter) const;

    float CalculateLift(float alpha, float speed, float mach) const;
    float CalculateSideLift(float slip, float speed, float mach) const;
    float CalculateDrag(float alpha, float slip, float speed, float mach) const;
    float EnginePower(float TotalSpeed, float Mach, float AirDensity) const;
    float GetGroundEffect(const FTransform &transform);
    float GroundTrace(FVector start, FVector direction, float length);

    void PreReplication(IRepChangedPropertyTracker &ChangedPropertyTracker) override;
};
