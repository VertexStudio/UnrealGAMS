#include "TransformController.h"

UTransformController::UTransformController()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTransformController::BeginPlay()
{
    Super::BeginPlay();
}

void UTransformController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor *Actor = GetOwner();

    if (Target && Actor)
    {

        FVector LocationSetpoint = Target->GetActorLocation();
        FVector LocationFeedback = Actor->GetActorLocation();

        LocationError = LocationSetpoint - LocationFeedback;
        auto LocationProportional = LocationError;
        LocationIntegral = LocationIntegral + LocationError * DeltaTime;
        auto LocationDerivative = (LocationError - LocationLastError) / DeltaTime;
        LocationLastError = LocationError;
        LocationControl = LocationKp * LocationProportional + LocationKi * LocationIntegral + LocationKd * LocationDerivative;

        FQuat RotationSetpoint = Target->GetActorRotation().Quaternion();
        FQuat RotationFeedback = Actor->GetActorRotation().Quaternion();
        RotationSetpoint.EnforceShortestArcWith(RotationFeedback);
        RotationError = FQuat::Error(RotationSetpoint, RotationFeedback);
        auto RotationProportional = RotationError;
        RotationIntegral = RotationIntegral + RotationError * DeltaTime;
        auto RotationDerivative = -(RotationError - RotationLastError) / DeltaTime;
        RotationLastError = RotationError;
        RotationControl = RotationKp * RotationProportional + RotationKi * RotationIntegral + RotationKd * RotationDerivative;

        FQuat Rotator = RotationSetpoint * RotationFeedback.Inverse();
        FVector RotatorAxis;
        float RotatorAngle;
        Rotator.ToAxisAndAngle(RotatorAxis, RotatorAngle);
        Actor->AddActorWorldRotation(FQuat(RotatorAxis, RotationControl * DeltaTime));
        Actor->AddActorWorldOffset(LocationControl * DeltaTime);
    }
}
