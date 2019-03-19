#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SignalGenerator.generated.h"

UENUM(BlueprintType)
enum class SignalType : uint8
{
    SIGNAL_SINE UMETA(DisplayName = "Sine"),
    SIGNAL_SQUARE UMETA(DisplayName = "Square"),
    SIGNAL_TRIANGLE UMETA(DisplayName = "Triangle"),
    SIGNAL_SAWTOOTH UMETA(DisplayName = "Sawtooth"),
    SIGNAL_PULSE UMETA(DisplayName = "Pulse"),
    SIGNAL_WHITENOISE UMETA(DisplayName = "WhiteNoise"),
    SIGNAL_GAUSSNOISE UMETA(DisplayName = "GaussNoise"),
    SIGNAL_DIGITALNOISE UMETA(DisplayName = "DigitalNoise"),
};

UCLASS(ClassGroup = (DynamicalSystems), meta = (BlueprintSpawnableComponent))
class DYNAMICALSYSTEMS_API USignalGenerator : public UActorComponent
{
    GENERATED_BODY()

    FRandomStream RandomStream;

  public:
    USignalGenerator();

    virtual void BeginPlay() override;

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalGenerator")
    SignalType SignalType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalGenerator")
    float Amplitude = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalGenerator")
    float Frequency = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalGenerator")
    float Phase = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalGenerator")
    float Offset = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalGenerator")
    bool Invert = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SignalGenerator")
    float Time = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SignalGenerator")
    float Value = 0.f;
};
