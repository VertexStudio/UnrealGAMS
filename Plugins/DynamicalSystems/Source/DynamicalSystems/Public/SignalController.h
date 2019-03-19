#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SignalController.generated.h"

UCLASS(ClassGroup = (DynamicalSystems), meta = (BlueprintSpawnableComponent))
class DYNAMICALSYSTEMS_API USignalController : public UActorComponent
{
    GENERATED_BODY()

    float Integral = 0;
    float LastError = 0;

  public:
    USignalController();

    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalController")
    float Kp = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalController")
    float Ki = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalController")
    float Kd = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalController")
    float Setpoint = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SignalController")
    float Error = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SignalController")
    float Control = 0;

    UPROPERTY(BlueprintReadOnly, Category = "SignalController")
    float Value = 0;

    UFUNCTION(BlueprintCallable, Category = "SignalController")
    void Reset();
};
