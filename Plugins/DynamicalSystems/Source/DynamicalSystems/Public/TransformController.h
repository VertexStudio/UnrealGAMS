#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "TransformController.generated.h"

UCLASS(ClassGroup = (DynamicalSystems), meta = (BlueprintSpawnableComponent))
class DYNAMICALSYSTEMS_API UTransformController : public UActorComponent
{
	GENERATED_BODY()

	FVector LocationIntegral = FVector(0);
	FVector LocationLastError = FVector(0);

	float RotationIntegral = 0;
	float RotationLastError = 0;

  public:
	UTransformController();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformController")
	FVector LocationKp = FVector(5);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformController")
	FVector LocationKi = FVector(0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformController")
	FVector LocationKd = FVector(0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformController")
	float RotationKp = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformController")
	float RotationKi = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformController")
	float RotationKd = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformController")
	class AActor *Target = NULL;

	UPROPERTY(BlueprintReadOnly, Category = "TransformController")
	FVector LocationError = FVector(0);

	UPROPERTY(BlueprintReadOnly, Category = "TransformController")
	FVector LocationControl = FVector(0);

	UPROPERTY(BlueprintReadOnly, Category = "TransformController")
	float RotationError = 0;

	UPROPERTY(BlueprintReadOnly, Category = "TransformController")
	float RotationControl = 0;
};
