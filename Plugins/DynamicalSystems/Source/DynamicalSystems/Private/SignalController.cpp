#include "SignalController.h"

USignalController::USignalController()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USignalController::BeginPlay()
{
    Super::BeginPlay();
}

void USignalController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    Error = Setpoint - Value;
    auto Proportional = Error;
    Integral = Integral + Error * DeltaTime;
    auto Derivative = (Error - LastError) / DeltaTime;
    LastError = Error;
    Control = Kp * Proportional + Ki * Integral + Kd * Derivative;
    Value = Value + Control * DeltaTime;
}

void USignalController::Reset()
{
    Error = 0;
    LastError = 0;
    Integral = 0;
}
