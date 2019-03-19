#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "RigidBodyController.generated.h"

UCLASS(ClassGroup = (DynamicalSystems), meta = (BlueprintSpawnableComponent))
class DYNAMICALSYSTEMS_API URigidBodyController : public UActorComponent
{
	GENERATED_BODY()

	FVector LocationIntegral = FVector(0);
	FVector LocationLastError = FVector(0);

	float RotationIntegral = 0;
	float RotationLastError = 0;

  public:
	URigidBodyController();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	FVector LocationKp = FVector(5);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	FVector LocationKi = FVector(0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	FVector LocationKd = FVector(0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	float RotationKp = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	float RotationKi = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	float RotationKd = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	class AActor *Target = NULL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	FRotator TargetRotation;

	UPROPERTY(BlueprintReadOnly, Category = "RigidBodyController")
	FVector LocationError = FVector(0);

	UPROPERTY(BlueprintReadOnly, Category = "RigidBodyController")
	FVector LocationControl = FVector(0);

	UPROPERTY(BlueprintReadOnly, Category = "RigidBodyController")
	float RotationError = 0;

	UPROPERTY(BlueprintReadOnly, Category = "RigidBodyController")
	float RotationControl = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RigidBodyController")
	bool Enabled = true;
};
