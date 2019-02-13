// Copyright 2016 Mookie. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/SceneComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "easygearcomponent.generated.h"

DECLARE_STATS_GROUP(TEXT("EasyGear"), STATGROUP_EasyGear, STATCAT_Advanced);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), HideCategories = (Transform, Rendering))
class EASYFM_API UEasyGear : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEasyGear();

	FCalculateCustomPhysics OnCalculateCustomPhysics;

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") bool Enabled = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") bool UseThisComponentTransform = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") TArray<TEnumAsByte<ECollisionChannel>> CollisionChannels;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") bool ScaleWithMass;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") bool ScaleGripLimitWithStrutCompression;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") bool UseMaterialFriction;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") bool DrawDebug;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") bool DebugLagCompensation=true;
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") bool SteerToVector;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "easyGear") float SteerToVectorSensitivity;


	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") bool NoseEnabled;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseLocationLong;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseLocationVert;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseStrutLength;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseWheelRadius;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseSpring;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseShockAbsorber;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseGrip;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseGripLimit;
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") bool NoseSteering;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Nosewheel") float NoseSteeringMaxAngle;

	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") bool MainLeftEnabled;
	UPROPERTY(Replicated, BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") bool MainRightEnabled;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainLocationLong;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainLocationLat;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainLocationVert;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainStrutLength;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainWheelRadius;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainSpring;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainShockAbsorber;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainGrip;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainBrakePower;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") float MainGripLimit;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Main Wheels") bool MainBrakeAntiLock;

	UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs") float SteeringInput=0.0f;
	UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs") float LeftBrakeInput=0.0f;
	UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs") float RightBrakeInput=0.0f;
	UPROPERTY(Replicated, BlueprintReadWrite, SaveGame, Category = "Inputs") FVector GuidedVector=FVector(0,0,0);

	UFUNCTION(BlueprintCallable, Category = "Input") float SetSteeringInput(float Input);
	UFUNCTION(BlueprintCallable, Category = "Input") float SetLeftBrakeInput(float Input);
	UFUNCTION(BlueprintCallable, Category = "Input") float SetRightBrakeInput(float Input);

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") float SteeringPosition=0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") bool NoseOnGround=false;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") bool NoseSkid=false;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") float NoseStrutCompression = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") FVector NoseLocation = FVector(0,0,0);
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") bool MainLeftOnGround = false;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") bool MainRightOnGround = false;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") bool MainLeftSkid = false;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") bool MainRightSkid =false;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") float MainLeftStrutCompression = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") float MainRightStrutCompression = 0.0f;
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") FVector MainLeftLocation = FVector(0, 0, 0);
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Outputs") FVector MainRightLocation = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication") bool ReplicateInputs = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateInputs")) float InputReplicationFrequency = 30.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, SaveGame, Category = "Replication", Meta = (EditCondition = "ReplicateInputs")) bool LocalInputPrediction = true;

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION(Server, WithValidation, Unreliable) void InputRep(float Steering, float LeftBrake, float RightBrake);

	float ClientSteeringInput = 0.0f;
	float ClientLeftBrakeInput = 0.0f;
	float ClientRightBrakeInput = 0.0f;

	float TimeSinceInputUpdate = 0.0f;
	void InputReplicate();

	void CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance);
	void Simulate(FBodyInstance* Parent, float DeltaTime, bool apply, bool debug);
	bool SimulateWheel(FBodyInstance* bodyInst, float DeltaTime, FVector loc, FVector direction, FVector fwdvec, FVector rightvec, 
		float length, float radius, float spring, float shock, float grip, float brakepower, float brakeinput, float maxgrip, float steering, float multiplier, float &compress, bool &skid, FVector &outLocation, bool apply, bool debug);
};