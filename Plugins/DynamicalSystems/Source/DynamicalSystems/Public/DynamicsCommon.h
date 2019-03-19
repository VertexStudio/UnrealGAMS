#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DynamicsCommon.generated.h"

/**
 * 
 */
UCLASS()
class DYNAMICALSYSTEMS_API UDynamicsCommon : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

  public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Float")
    static float MeanOfFloatArray(const TArray<float> &Samples);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Float")
    static float VarianceOfFloatArray(const TArray<float> &Samples);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Float")
    static float MedianOfFloatArray(const TArray<float> &Samples);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Float")
    static float StandardDeviationOfFloatArray(const TArray<float> &Samples);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Rotator")
    static FQuat RotatorToQuat(FRotator Rotator);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Math|Vector")
    static FVector CubicBezier(float Time, FVector P0, FVector P1, FVector P2, FVector P3);
};
